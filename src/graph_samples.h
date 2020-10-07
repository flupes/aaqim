#ifndef AAQIM_GRAPH_SAMPLES_H
#define AAQIM_GRAPH_SAMPLES_H

#include "Adafruit_GFX.h"
#include "Fonts/ClearSans-Medium-12pt7b.h"
#include "Fonts/ClearSans-Medium-8pt7b.h"
#include "Fonts/Picopixel.h"
#include "display_samples.h"
#include "epd2in7b.h"

constexpr int16_t kGraphWidth = 144;
constexpr int16_t kGraphHeight = 100;
constexpr int16_t kGraphXstart = 2;
constexpr int16_t kGraphYstart = EPD_HEIGHT - 2;

class GraphSamples : public DisplaySamples<kGraphWidth, int16_t> {
 public:
  GraphSamples(uint32_t period_in_seconds)
      : DisplaySamples(period_in_seconds) {}

  void Draw(GFXcanvas1 *black, GFXcanvas1 *red, uint8_t background_color = 0,
            uint8_t foreground_color = 1);
};

void GraphSamples::Draw(GFXcanvas1 *black, GFXcanvas1 *red, uint8_t bgColor,
                        uint8_t fgColor) {
  // draw the left/right vertical line of the graph boundary.
  // (all the others boundaries will be draw together with the tick marks)
  black->drawFastVLine(kGraphXstart, kGraphYstart, -kGraphHeight, fgColor);
  black->drawFastVLine(kGraphXstart + kGraphWidth, kGraphYstart, -kGraphHeight,
                       fgColor);

  // Round upper and lower limits to 100 AQI
  const int16_t vStep = 100;
  const int16_t lowerLimit = min_ - min_ % vStep;
  const int16_t upperLimit = ((max_ - 1) / vStep + 1) * vStep;
  printf("upper limit = %d\n", upperLimit);

  // char buffer[8];
  black->setTextColor(fgColor);
  black->setTextSize(1);
  black->setTextWrap(false);
  black->setFont(&ClearSans_Medium12pt7b);

  // legend
  uint32 hourPeriod = length_ * period_ / 3600;
  printf("period in hours = %d\n", hourPeriod);
  char msg[3];
  if (hourPeriod < 100) {
    int16_t base = EPD_HEIGHT - 52;
    black->setCursor(EPD_WIDTH - 11, base + 8);
    black->print("h");
    sprintf(msg, "%d", hourPeriod);
    for (size_t c = 0; c < 2; c++) {
      char l = msg[c];
      black->setCursor(EPD_WIDTH - 22, base);
      black->print(l);
      base += 16;
    }
  }

  // upper limit text
  int16_t uBase = EPD_HEIGHT - kGraphHeight + 16;
  black->setCursor(EPD_WIDTH - 27, uBase);
  black->print(upperLimit / 100);
  // lower limit text
  int16_t lBase = EPD_HEIGHT - 2;
  bool lowIsZero = false;
  if (lowerLimit < 100) {
    black->setCursor(EPD_WIDTH - 20, lBase);
    black->print("0");
    lowIsZero = true;
  } else {
    black->setCursor(EPD_WIDTH - 27, lBase);
    black->print(lowerLimit / 100);
  }
  // black->setFont(&Picopixel);
  black->setFont(&ClearSans_Medium8pt7b);
  if (!lowIsZero) {
    black->setCursor(EPD_WIDTH - 16, lBase - 5);
    black->print("00");
  }
  black->setCursor(EPD_WIDTH - 16, uBase - 5);
  black->print("00");

  // Vertical ticks
  int16_t y = kGraphYstart;
  // int16_t xLabel = kGraphXstart + kGraphWidth;
  for (int16_t v = lowerLimit; v <= upperLimit; v += vStep) {
    black->drawFastHLine(kGraphXstart, y, kGraphWidth + 4, fgColor);
    // sprintf(buffer, "%d", v);
    // black->setCursor(xLabel + 6, y + 4);
    // black->print(buffer);
    y -= vStep;
  }

  // Actually draw the Time Serie :-)
  int16_t y1 = 0;
  for (size_t i = 0; i < length_; i++) {
    int16_t data = Value(i);
    int16_t y2 = kGraphYstart -
                 kGraphHeight * (data - lowerLimit) / (upperLimit - lowerLimit);
    if (y1 != 0 && data != INT16_MIN) {
      if (y1 < y2) {
        black->drawFastVLine(i + kGraphXstart, y1 - 1, 2 + (y2 - y1), fgColor);
      } else {
        black->drawFastVLine(i + kGraphXstart, y2 - 1, 2 + (y1 - y2), fgColor);
      }
    }
    if (data == INT16_MIN) {
      y1 = 0;
    } else {
      y1 = y2;
    }
  }
}

#endif
