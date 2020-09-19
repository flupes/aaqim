// Get the board definitions...
#include <Adafruit_GFX.h>
#include <Arduino.h>

#include "B612Mono-Bold-18pt7b.h"
#include "B612Mono-Bold-48pt7b.h"
#include "Fonts/ClearSans-Bold-18pt7b.h"
#include "Fonts/ClearSans-Bold-48pt7b.h"
#include "Fonts/ClearSans-Medium-10pt7b.h"
#include "Fonts/ClearSans-Medium-12pt7b.h"
#include "Fonts/ClearSans-Medium-16pt7b.h"
#include "Fonts/ClearSans-Medium-18pt7b.h"
#include "Fonts/ClearSans-Medium-32pt7b.h"
#include "Hack-Bold-18pt7b.h"
#include "Hack-Bold-48pt7b.h"
#include "epd2in7b.h"

// Horrible bug in PlatformIO Forces to include these...
#include <Adafruit_I2CDevice.h>
#include <Wire.h>

#define COLORED 1
#define UNCOLORED 0

Epd epd;
GFXcanvas1 *canvas[2];

char short_str[] = "012345689.:+";
char long_str[] = "-0123456789.:+ABCO";

extern "C" char *sbrk(int i);

enum class ColorBuffer : uint8_t { Black = 0, Red = 1 };

static int16_t line = 0;

uint16_t printText(const GFXfont *font, const char *str, ColorBuffer color) {
  int16_t x, y;
  uint16_t w, h;

  uint8_t bufferId = static_cast<uint8_t>(color);
  canvas[bufferId]->setFont(font);
  canvas[bufferId]->getTextBounds(str, 0, 100, &x, &y, &w, &h);
  Serial.print("Font Height = ");
  Serial.print(h);
  Serial.print(" / String length = ");
  Serial.println(w);
  line += h + 2;
  canvas[bufferId]->setCursor(4, line);
  canvas[bufferId]->print(str);
  line += 2;
  canvas[bufferId]->drawFastHLine(0, line, EPD_HEIGHT, COLORED);
  return line;
}

void setup() {
  Serial.begin(115200);

  Serial.println("Init e-Paper...");

  if (epd.Init() != 0) {
    Serial.println("e-Paper init failed");
    return;
  }
  epd.ClearFrame();

  Serial.print("Free RAM before Canvas allocation: ");
  Serial.println(ESP.getFreeHeap());

  canvas[0] = new GFXcanvas1(EPD_WIDTH, EPD_HEIGHT);
  // canvas[0]->setRotation(1);
  canvas[1] = new GFXcanvas1(EPD_WIDTH, EPD_HEIGHT);
  // canvas[1]->setRotation(1);

  Serial.print("Free RAM after Canvas allocation: ");
  Serial.println(ESP.getFreeHeap());

  for (uint8_t i = 0; i < 2; i++) {
    canvas[i]->fillScreen(UNCOLORED);

    canvas[i]->setTextColor(COLORED);
    canvas[i]->setTextSize(1);
    canvas[i]->setTextWrap(false);

    printText(&ClearSans_Medium10pt7b, long_str, static_cast<ColorBuffer>(i));
    printText(&ClearSans_Medium12pt7b, long_str, static_cast<ColorBuffer>(i));
    printText(&ClearSans_Medium16pt7b, short_str, static_cast<ColorBuffer>(i));
    printText(&ClearSans_Medium18pt7b, short_str, static_cast<ColorBuffer>(i));
    printText(&ClearSans_Medium32pt7b, short_str, static_cast<ColorBuffer>(i));
  }

  // canvas[1]->drawRect(0, 0, EPD_WIDTH, EPD_HEIGHT, COLORED);

  epd.TransmitPartialBlack(canvas[0]->getBuffer(), 0, 0, EPD_WIDTH, EPD_HEIGHT);
  epd.TransmitPartialRed(canvas[1]->getBuffer(), 0, 0, EPD_WIDTH, EPD_HEIGHT);
  epd.DisplayFrame();

  delay(12 * 1000);

  line = 0;
  for (uint8_t i = 0; i < 2; i++) {
    canvas[i]->fillScreen(UNCOLORED);
  }
  printText(&ClearSans_Bold18pt7b, short_str, static_cast<ColorBuffer>(0));
  printText(&ClearSans_Bold48pt7b, short_str, static_cast<ColorBuffer>(0));
  printText(&Hack_Bold18pt7b, short_str, static_cast<ColorBuffer>(0));
  printText(&Hack_Bold48pt7b, short_str, static_cast<ColorBuffer>(0));
  printText(&B612Mono_Bold18pt7b, short_str, static_cast<ColorBuffer>(0));
  printText(&B612Mono_Bold48pt7b, short_str, static_cast<ColorBuffer>(0));
  epd.TransmitPartial(canvas[0]->getBuffer(), canvas[1]->getBuffer(), 0, 0,
                      EPD_WIDTH, EPD_HEIGHT);
  epd.DisplayFrame();

  delay(12 * 1000);
  epd.TransmitPartial(canvas[1]->getBuffer(), canvas[0]->getBuffer(), 0, 0,
                      EPD_WIDTH, EPD_HEIGHT);
  epd.DisplayFrame();

  Serial.println("Put display to sleep");
  epd.Sleep();
}

void loop() {}
