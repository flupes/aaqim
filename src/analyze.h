#ifndef AAQIM_ANALYZE_H
#define AAQIM_ANALYZE_H

#if defined(ARDUINO)

#include <stdlib.h>

#include "air_sample.h"
#include "sensors.h"
#include "stats.h"

static const uint32_t kMaxReadingAgeSeconds = 12 * 60;
static const float kMaxPercentDiscrepancy = 0.05;

size_t ComputeStats(const AirSensors& sensors, AirSample& sample) {
  short primary = -1;
  size_t count = 0;
  float cfvalues[2 * kMaxSensors];

  for (size_t i = 0; i < sensors.Count(); i++) {
    const SensorData& data = sensors.Data(i);

    // check age
    if (data.age_A > 5 || data.age_B > 5 || abs(data.age_A - data.age_B) > 15) {
      break;
    }

    // check sensors consistency
    if (fabs(data.pm_2_5_A - data.pm_2_5_B) / (data.pm_2_5_A + data.pm_2_5_B) >
        kMaxPercentDiscrepancy) {
      break;
    }

    // keep which sensor is considered the primary one
    if (primary < 0) {
      primary = i;
    }

    cfvalues[count * 2] = data.pm_2_5_A;
    cfvalues[count * 2 + 1] = data.pm_2_5_B;
    count++;
  }

  if (primary < 0) {
    Serial.println("Could not identify a primary sensor!");
    sample.Set(0, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0);
  } else {
    Serial.print("Compute stats with n sensors = ");
    Serial.println(count);
    float avg, nmae;
    mean_error(count * 2, cfvalues, avg, nmae);
    Serial.print("NMAE = ");
    Serial.println(nmae);
    const SensorData& data = sensors.Data(primary);
    sample.Set(data.timestamp, 0.0, avg, 0.0, data.pressure, data.temperature,
               data.humidity, count, nmae);
  }

  return count;
}

#endif

#endif
