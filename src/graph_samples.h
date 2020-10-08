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

 protected:
  const int16_t vStep_ = 100;
  int16_t lowerLimit_;
  int16_t upperLimit_;
  int16_t ValueToVerticalPixel(int16_t value) {
      return (kGraphYstart - kGraphHeight * (value - lowerLimit_) / (upperLimit_ - lowerLimit_));
  }
};

void GraphSamples::Draw(GFXcanvas1 *black, GFXcanvas1 *red, uint8_t bgColor,
                        uint8_t fgColor) {
  // Round upper and lower limits to 100 AQI
  lowerLimit_ = min_ - min_ % vStep_;
  upperLimit_ = ((max_ - 1) / vStep_ + 1) * vStep_;

  // Grid
  for (int16_t h = 0; h < 5; h++) {
    int16_t x = kGraphXstart + ((float)kGraphWidth / 4.0) * h;
    black->drawFastVLine(x, kGraphYstart, -kGraphHeight, fgColor);
  }
  for (int16_t v = lowerLimit_; v <= upperLimit_; v += vStep_ / 2) {
    int16_t y = ValueToVerticalPixel(v);
    black->drawFastHLine(kGraphXstart, y, kGraphWidth, fgColor);
  }

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
  black->print(upperLimit_ / 100);
  // lower limit text
  int16_t lBase = EPD_HEIGHT - 2;
  bool lowIsZero = false;
  if (lowerLimit_ < 100) {
    black->setCursor(EPD_WIDTH - 20, lBase);
    black->print("0");
    lowIsZero = true;
  } else {
    black->setCursor(EPD_WIDTH - 27, lBase);
    black->print(lowerLimit_ / 100);
  }
  // black->setFont(&Picopixel);
  black->setFont(&ClearSans_Medium8pt7b);
  if (!lowIsZero) {
    black->setCursor(EPD_WIDTH - 16, lBase - 5);
    black->print("00");
  }
  black->setCursor(EPD_WIDTH - 16, uBase - 5);
  black->print("00");

  // Actually draw the Time Serie :-)
  int16_t y1 = 0;
  for (size_t i = 0; i < length_; i++) {
    int16_t data = Value(i);

    int16_t y2 = ValueToVerticalPixel(data);
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
