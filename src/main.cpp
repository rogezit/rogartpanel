/**
 * MARCADOR PÁDEL · Panel P10 32x16 · ESP32
 *
 * Layout FINAL (32px ancho):
 *
 *  x=  0    6    12   18  20..31
 *      S1   S2   S3   |   PTS centrado
 *      6px  6px  6px  1px  12px
 *
 * CONEXIONES IDC:
 *   Pin 1  R1  → GPIO 25    Pin 9  A   → GPIO 23
 *   Pin 2  G1  → GPIO 26    Pin 10 B   → GPIO 19
 *   Pin 3  B1  → GPIO 2     Pin 11 C   → GPIO 5
 *   Pin 4  GND → GND        Pin 12 D   → GPIO 17
 *   Pin 5  R2  → GPIO 14    Pin 13 CLK → GPIO 16
 *   Pin 6  G2  → GPIO 12    Pin 14 LAT → GPIO 4
 *   Pin 7  B2  → GPIO 27    Pin 15 OE  → desconectado
 *   Pin 8  GND → desconect. Pin 16 GND → GND
 */

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// ── PANEL ─────────────────────────────────────────────────
#define PANEL_WIDTH 32
#define PANEL_HEIGHT 16
#define PANELS_NUM 1

#define PIN_R1 25
#define PIN_G1 26
#define PIN_B1 2
#define PIN_R2 14
#define PIN_G2 12
#define PIN_B2 27
#define PIN_A 23
#define PIN_B 19
#define PIN_C 5
#define PIN_D 17
#define PIN_CLK 16
#define PIN_LAT 4
#define PIN_OE 15

MatrixPanel_I2S_DMA *display = nullptr;

// ── COLORES ───────────────────────────────────────────────
uint16_t C_RED, C_BLUE, C_WHITE, C_YELLOW, C_GREEN, C_GRAY, C_DIM;

// ── ESTADO DEL PARTIDO ────────────────────────────────────
int pts[2] = {0, 0};
int games[2] = {0, 0};
int sets[2] = {0, 0};
int setHistory[3][2] = {{0, 0}, {0, 0}, {0, 0}};
int currentSet = 0;
bool deuce = false;
bool matchOver = false;
int winner = -1;

#define POINT_DELAY_MS 1500

const char *PTS_LABELS[] = {"0", "15", "30", "40", "AD"};

// ── POSICIONES LAYOUT FINAL ───────────────────────────────
#define X_S0 0   // Set 1  → x=0
#define X_S1 6   // Set 2  → x=6
#define X_S2 12  // Set 3  → x=12
#define X_SEP 18 // Separador vertical → x=18
#define X_PTS 20 // Inicio zona puntos → x=20
#define PTS_W 12 // Ancho zona puntos (20..31 = 12px)

// ── INIT DISPLAY ──────────────────────────────────────────
void initDisplay()
{
  HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANELS_NUM);
  mxconfig.gpio.r1 = PIN_R1;
  mxconfig.gpio.g1 = PIN_G1;
  mxconfig.gpio.b1 = PIN_B1;
  mxconfig.gpio.r2 = PIN_R2;
  mxconfig.gpio.g2 = PIN_G2;
  mxconfig.gpio.b2 = PIN_B2;
  mxconfig.gpio.a = PIN_A;
  mxconfig.gpio.b = PIN_B;
  mxconfig.gpio.c = PIN_C;
  mxconfig.gpio.d = PIN_D;
  mxconfig.gpio.clk = PIN_CLK;
  mxconfig.gpio.lat = PIN_LAT;
  mxconfig.gpio.oe = PIN_OE;
  mxconfig.driver = HUB75_I2S_CFG::FM6126A;
  mxconfig.clkphase = false;
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;
  mxconfig.latch_blanking = 4;
  mxconfig.min_refresh_rate = 60;
  mxconfig.double_buff = true;

  display = new MatrixPanel_I2S_DMA(mxconfig);
  if (!display->begin())
  {
    Serial.println("ERROR panel");
    while (true)
      delay(1000);
  }
  display->setBrightness8(200);
  display->clearScreen();
  display->flipDMABuffer();
  delay(100);

  C_RED = display->color565(255, 0, 0);
  C_BLUE = display->color565(0, 80, 255);
  C_WHITE = display->color565(255, 255, 255);
  C_YELLOW = display->color565(255, 220, 0);
  C_GREEN = display->color565(0, 220, 0);
  C_GRAY = display->color565(100, 100, 100);
  C_DIM = display->color565(35, 35, 35);
}

// ── DIBUJAR FILA DE EQUIPO ────────────────────────────────
void drawTeamRow(int team, int y)
{
  uint16_t mainColor = (team == 0) ? C_RED : C_BLUE;
  uint16_t dimColor = (team == 0)
                          ? display->color565(120, 0, 0)
                          : display->color565(0, 0, 120);

  display->setTextWrap(false);
  display->setTextSize(1);

  const int setX[3] = {X_S0, X_S1, X_S2};

  // ── SETS ─────────────────────────────────────────────
  for (int s = 0; s < 3; s++)
  {
    if (s < currentSet)
    {
      int myVal = setHistory[s][team];
      int oppVal = setHistory[s][1 - team];
      bool iWon = myVal > oppVal;
      display->setTextColor(iWon ? mainColor : C_GRAY);
      display->setCursor(setX[s], y);
      display->print(myVal);
    }
    else if (s == currentSet)
    {
      display->setTextColor(dimColor);
      display->setCursor(setX[s], y);
      display->print(games[team]);
    }
    else
    {
      display->setTextColor(C_DIM);
      display->setCursor(setX[s], y);
      display->print("-");
    }
  }

  // ── SEPARADOR VERTICAL ───────────────────────────────
  for (int py = y; py < y + 7; py++)
  {
    display->drawPixel(X_SEP, py, C_GRAY);
  }

  // ── PUNTOS centrado en zona X_PTS..31 ────────────────
  const char *pstr = PTS_LABELS[pts[team]];
  int lenPx = strlen(pstr) * 6;
  int xPts = X_PTS + (PTS_W - lenPx) / 2;
  if (xPts < X_PTS)
    xPts = X_PTS;

  uint16_t pColor = (deuce && pts[team] == 4) ? C_YELLOW : deuce ? C_GRAY
                                                                 : C_WHITE;

  display->setTextColor(pColor);
  display->setCursor(xPts, y);
  display->print(pstr);
}

// ── MARCADOR COMPLETO ─────────────────────────────────────
void _drawScoreContent()
{
  drawTeamRow(0, 1);

  for (int x = 0; x < PANEL_WIDTH; x++)
  {
    display->drawPixel(x, 8, C_DIM);
  }

  drawTeamRow(1, 9);
}

void drawScore()
{
  for (int i = 0; i < 2; i++)
  {
    display->clearScreen();
    _drawScoreContent();
    display->flipDMABuffer();
    delay(20);
  }
}

// ── LÓGICA PUNTUACIÓN ─────────────────────────────────────
void winGame(int team);
void winSet(int team);

void awardPoint(int team)
{
  int opp = 1 - team;
  if (!deuce)
  {
    pts[team]++;
    if (pts[team] == 3 && pts[opp] == 3)
    {
      deuce = true;
      Serial.println("DEUCE");
    }
    else if (pts[team] == 4)
    {
      winGame(team);
    }
  }
  else
  {
    if (pts[team] == 4)
    {
      winGame(team);
    }
    else if (pts[opp] == 4)
    {
      pts[0] = 3;
      pts[1] = 3;
      Serial.println("DEUCE again");
    }
    else
    {
      pts[team] = 4;
      Serial.printf("VENTAJA %c\n", team == 0 ? 'A' : 'B');
    }
  }
}

void winGame(int team)
{
  int opp = 1 - team;
  games[team]++;
  pts[0] = 0;
  pts[1] = 0;
  deuce = false;
  Serial.printf("GAME %c → %d-%d\n", team == 0 ? 'A' : 'B', games[0], games[1]);
  if (games[team] >= 6 && (games[team] - games[opp]) >= 2)
    winSet(team);
  else if (games[team] == 7)
    winSet(team);
}

void winSet(int team)
{
  setHistory[currentSet][0] = games[0];
  setHistory[currentSet][1] = games[1];
  currentSet++;
  sets[team]++;
  games[0] = 0;
  games[1] = 0;
  Serial.printf("SET %c → %d-%d\n", team == 0 ? 'A' : 'B', sets[0], sets[1]);
  if (sets[team] >= 2)
  {
    matchOver = true;
    winner = team;
    Serial.printf("MATCH OVER — Gana %c!\n", team == 0 ? 'A' : 'B');
  }
}

// ── ANIMACIONES ───────────────────────────────────────────
void flashColor(uint16_t color, int times)
{
  for (int i = 0; i < times; i++)
  {
    for (int b = 0; b < 2; b++)
    {
      display->fillScreen(color);
      display->flipDMABuffer();
      delay(20);
    }
    delay(200);
    for (int b = 0; b < 2; b++)
    {
      display->clearScreen();
      display->flipDMABuffer();
      delay(20);
    }
    delay(200);
  }
}

void animateMatchOver()
{
  uint16_t wColor = (winner == 0) ? C_RED : C_BLUE;
  flashColor(wColor, 4);

  for (int i = 0; i < 2; i++)
  {
    display->clearScreen();
    display->setTextSize(1);
    display->setTextColor(wColor);
    display->setCursor(1, 1);
    display->print(winner == 0 ? "GANA A!" : "GANA B!");
    display->setTextColor(C_WHITE);
    display->setCursor(4, 9);
    display->print(sets[0]);
    display->print(" - ");
    display->print(sets[1]);
    display->flipDMABuffer();
    delay(20);
  }
  delay(5000);
}

// ── SETUP ─────────────────────────────────────────────────
void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("Iniciando marcador padel...");
  initDisplay();
  flashColor(C_GREEN, 1);
  delay(300);
  drawScore();
  Serial.println("Listo.");
}

// ── SECUENCIA SIMULACIÓN ──────────────────────────────────
const int SEQUENCE[] = {
    // Set 1: A gana 6-3
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
    0,
    0,
    1,
    0,
    0,
    1,
    0,
    1,
    0,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    // Set 2: B gana 6-4
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    // Set 3: A gana 6-4
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

int seqIndex = 0;
uint32_t lastMs = 0;

// ── LOOP ──────────────────────────────────────────────────
void loop()
{
  if (matchOver)
  {
    animateMatchOver();
    pts[0] = 0;
    pts[1] = 0;
    games[0] = 0;
    games[1] = 0;
    sets[0] = 0;
    sets[1] = 0;
    for (int s = 0; s < 3; s++)
    {
      setHistory[s][0] = 0;
      setHistory[s][1] = 0;
    }
    currentSet = 0;
    deuce = false;
    matchOver = false;
    winner = -1;
    seqIndex = 0;
    delay(2000);
    return;
  }

  uint32_t now = millis();
  if (now - lastMs >= POINT_DELAY_MS)
  {
    lastMs = now;
    if (seqIndex < (int)(sizeof(SEQUENCE) / sizeof(SEQUENCE[0])))
    {
      int scorer = SEQUENCE[seqIndex++];
      Serial.printf("Punto → %c\n", scorer == 0 ? 'A' : 'B');
      awardPoint(scorer);
    }
    drawScore();
  }
}
