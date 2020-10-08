#ifndef AAQIM_GRAPH_SAMPLES_H
#define AAQIM_GRAPH_SAMPLES_H

#include "display_samples.h"

class GFXcanvas1;

constexpr int16_t kGraphWidth = 144;
constexpr int16_t kGraphHeight = 100;

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
  int16_t ValueToVerticalPixel(int16_t value);
};

#endif
