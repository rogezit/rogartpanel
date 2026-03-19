/**
 * ╔══════════════════════════════════════════════════════════╗
 * ║   TEST PANEL HUB75 · P10 32×16 · 1/8 scan              ║
 * ║   Librería: ESP32-HUB75-MatrixPanel-DMA                 ║
 * ╚══════════════════════════════════════════════════════════╝
 *
 *  CONEXIONES IDC 16 pines → ESP32 (lado derecho):
 *  ─────────────────────────────────────────────────
 *  Pin 1  R1   → GPIO 25
 *  Pin 2  G1   → GPIO 26
 *  Pin 3  B1   → GPIO 27
 *  Pin 4  GND  → GND
 *  Pin 5  R2   → GPIO 14
 *  Pin 6  G2   → GPIO 12
 *  Pin 7  B2   → GPIO 13
 *  Pin 8  GND  → GND
 *  Pin 9  A    → GPIO 23
 *  Pin 10 B    → GPIO 19
 *  Pin 11 C    → GPIO  5
 *  Pin 12 D    → GPIO 17   ← necesario para 1/8 scan
 *  Pin 13 CLK  → GPIO 16
 *  Pin 14 LAT  → GPIO  4
 *  Pin 15 OE   → GPIO 15
 *  Pin 16 GND  → GND
 *  ─────────────────────────────────────────────────
 *  POWER rojo  → 5V fuente externa
 *  POWER negro → GND fuente externa
 *  ─────────────────────────────────────────────────
 *  ⚠ GND del ESP32 y GND de la fuente deben estar
 *    unidos (cable corto entre ambos GND)
 */

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// ── DIMENSIONES DEL PANEL ─────────────────────────────────
#define PANEL_WIDTH   32
#define PANEL_HEIGHT  16
#define PANELS_NUM     1   // solo un panel por ahora

// ── PINES HUB75 ───────────────────────────────────────────
#define PIN_R1   25
#define PIN_G1   26
#define PIN_B1   27
#define PIN_R2   14
#define PIN_G2   12
#define PIN_B2   13
#define PIN_A    23
#define PIN_B    19
#define PIN_C     5
#define PIN_D    17
#define PIN_CLK  16
#define PIN_LAT   4
#define PIN_OE   15

MatrixPanel_I2S_DMA *display = nullptr;

// Colores útiles (formato 16-bit 565)
uint16_t RED, GREEN, BLUE, WHITE, YELLOW, CYAN, MAGENTA, BLACK;

// ──────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println(F("\n[ PANEL HUB75 ] Iniciando..."));

  // Configuración del panel
  HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANELS_NUM);

  // Asignar pines
  mxconfig.gpio.r1  = PIN_R1;
  mxconfig.gpio.g1  = PIN_G1;
  mxconfig.gpio.b1  = PIN_B1;
  mxconfig.gpio.r2  = PIN_R2;
  mxconfig.gpio.g2  = PIN_G2;
  mxconfig.gpio.b2  = PIN_B2;
  mxconfig.gpio.a   = PIN_A;
  mxconfig.gpio.b   = PIN_B;
  mxconfig.gpio.c   = PIN_C;
  mxconfig.gpio.d   = PIN_D;
  mxconfig.gpio.clk = PIN_CLK;
  mxconfig.gpio.lat = PIN_LAT;
  mxconfig.gpio.oe  = PIN_OE;

  // Panel P10 1/8 scan
  mxconfig.driver = HUB75_I2S_CFG::SHIFTREG;
  mxconfig.clkphase = false;

  // Crear display
  display = new MatrixPanel_I2S_DMA(mxconfig);
  display->begin();
  display->setBrightness8(80);   // 0-255, empieza suave
  display->clearScreen();

  // Definir colores
  RED     = display->color565(255,   0,   0);
  GREEN   = display->color565(  0, 255,   0);
  BLUE    = display->color565(  0,   0, 255);
  WHITE   = display->color565(255, 255, 255);
  YELLOW  = display->color565(255, 255,   0);
  CYAN    = display->color565(  0, 255, 255);
  MAGENTA = display->color565(255,   0, 255);
  BLACK   = display->color565(  0,   0,   0);

  Serial.println(F("[ PANEL HUB75 ] Listo."));
}

// ──────────────────────────────────────────────────────────
void loop() {

  // ── PRUEBA 1: Llenar con colores sólidos ──────────────
  Serial.println(F("Rojo..."));
  display->fillScreen(RED);
  delay(800);

  Serial.println(F("Verde..."));
  display->fillScreen(GREEN);
  delay(800);

  Serial.println(F("Azul..."));
  display->fillScreen(BLUE);
  delay(800);

  display->fillScreen(BLACK);
  delay(200);

  // ── PRUEBA 2: Líneas de colores ───────────────────────
  Serial.println(F("Lineas de colores..."));
  display->drawFastHLine(0, 0,  PANEL_WIDTH, RED);
  display->drawFastHLine(0, 2,  PANEL_WIDTH, GREEN);
  display->drawFastHLine(0, 4,  PANEL_WIDTH, BLUE);
  display->drawFastHLine(0, 6,  PANEL_WIDTH, YELLOW);
  display->drawFastHLine(0, 8,  PANEL_WIDTH, CYAN);
  display->drawFastHLine(0, 10, PANEL_WIDTH, MAGENTA);
  display->drawFastHLine(0, 12, PANEL_WIDTH, WHITE);
  display->drawFastHLine(0, 14, PANEL_WIDTH, RED);
  delay(1500);

  display->fillScreen(BLACK);
  delay(200);

  // ── PRUEBA 3: Marcador simulado ───────────────────────
  Serial.println(F("Marcador simulado..."));
  display->fillScreen(BLACK);

  // Equipo A (rojo) - lado izquierdo
  display->setTextSize(2);
  display->setTextColor(RED);
  display->setCursor(1, 1);
  display->print("3");

  // Separador
  display->setTextColor(WHITE);
  display->setTextSize(1);
  display->setCursor(14, 5);
  display->print("-");

  // Equipo B (azul) - lado derecho
  display->setTextSize(2);
  display->setTextColor(BLUE);
  display->setCursor(19, 1);
  display->print("1");

  // Sets pequeños abajo
  display->setTextSize(1);
  display->setTextColor(display->color565(180,180,180));
  display->setCursor(1, 10);
  display->print("A");
  display->setCursor(8, 10);
  display->print("2-1");
  display->setCursor(20, 10);
  display->print("B");

  delay(3000);

  display->fillScreen(BLACK);
  delay(300);

  // ── PRUEBA 4: Píxeles individuales ───────────────────
  Serial.println(F("Pixeles..."));
  for (int y = 0; y < PANEL_HEIGHT; y++) {
    for (int x = 0; x < PANEL_WIDTH; x++) {
      uint16_t color = display->color565(
        random(0, 255),
        random(0, 255),
        random(0, 255)
      );
      display->drawPixel(x, y, color);
      delay(3);
    }
  }
  delay(1000);

  display->fillScreen(BLACK);
  delay(300);
}
