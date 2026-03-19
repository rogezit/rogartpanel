/**
 * TEST PANEL P10 32x16 · 1/8 scan · ESP32
 * Con configuración explícita para paneles chinos P10
 */

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// ── DIMENSIONES ───────────────────────────────────────────
#define PANEL_WIDTH   32
#define PANEL_HEIGHT  16
#define PANELS_NUM     1

// ── PINES ─────────────────────────────────────────────────
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

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("Iniciando panel P10 32x16 1/8 scan..."));

  HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANELS_NUM);

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

  // Configuración específica para P10 chinos 1/8 scan
  mxconfig.driver     = HUB75_I2S_CFG::SHIFTREG;
  mxconfig.clkphase   = false;
  mxconfig.i2sspeed   = HUB75_I2S_CFG::HZ_10M;  // bajar velocidad de reloj
  mxconfig.latch_blanking = 4;

  display = new MatrixPanel_I2S_DMA(mxconfig);

  if (display->begin() == false) {
    Serial.println(F("ERROR: No se pudo inicializar el panel."));
    Serial.println(F("Verifica conexiones y alimentacion."));
    while(true) { delay(1000); }
  }

  Serial.println(F("Panel inicializado OK."));
  display->setBrightness8(128);
  display->clearScreen();
  delay(100);

  // ── PRUEBA 1: Panel completo blanco ───────────────────
  // Si ves algo blanco = conexiones RGB bien
  Serial.println(F("Blanco completo..."));
  display->fillScreen(display->color565(255, 255, 255));
  delay(2000);

  // ── PRUEBA 2: Solo rojo ───────────────────────────────
  Serial.println(F("Solo rojo..."));
  display->fillScreen(display->color565(255, 0, 0));
  delay(2000);

  // ── PRUEBA 3: Solo verde ──────────────────────────────
  Serial.println(F("Solo verde..."));
  display->fillScreen(display->color565(0, 255, 0));
  delay(2000);

  // ── PRUEBA 4: Solo azul ───────────────────────────────
  Serial.println(F("Solo azul..."));
  display->fillScreen(display->color565(0, 0, 255));
  delay(2000);

  display->clearScreen();
  Serial.println(F("Prueba de colores completa."));
}

void loop() {
  // Marcador estático para verificar texto
  display->clearScreen();

  // Fondo negro, números grandes
  display->setTextSize(2);

  // Equipo A rojo
  display->setTextColor(display->color565(255, 0, 0));
  display->setCursor(2, 1);
  display->print("3");

  // Guión blanco
  display->setTextColor(display->color565(255, 255, 255));
  display->setTextSize(1);
  display->setCursor(14, 5);
  display->print("-");

  // Equipo B azul
  display->setTextSize(2);
  display->setTextColor(display->color565(0, 100, 255));
  display->setCursor(20, 1);
  display->print("1");

  delay(5000);

  // Alternar brillo para confirmar que el panel responde
  display->setBrightness8(200);
  delay(1000);
  display->setBrightness8(128);
  delay(1000);
}
