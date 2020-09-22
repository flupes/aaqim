#ifndef AAQIM_ANALYZE_H
#define AAQIM_ANALYZE_H

#if defined(ARDUINO)

#include <stdlib.h>

#include "air_sample.h"
#include "sensors.h"
#include "stats.h"

// PA sensors seem to be updated every two minutes.
// We consider the sensor valid even if we miss three beats
const int16_t kMaxReadingAgeMinutes = 7;

// PA report 100% confidence when the two readings of one station
// are within 6%. So we should still accept these, but will
// discard the sensor if higher (not sure at what point PA confidence decreases)
const float kMaxPercentDiscrepancy = 0.06;

size_t ComputeStats(const AirSensors& sensors, AirSample& sample,
                    int32_t& primaryIndex) {
  primaryIndex = -1;
  size_t count = 0;
  float cfvalues[2 * kMaxSensors];

  for (size_t i = 0; i < sensors.Count(); i++) {
    const SensorData& data = sensors.Data(i);

    // check age
    if (data.age_A > kMaxReadingAgeMinutes || data.age_B > kMaxReadingAgeMinutes) {
      continue;
    }

    // check sensors consistency in percent (it seems that for low readings,
    // a 1.2 absolute diffence is still acceptable?)
    float diff = fabs(data.pm_2_5_A - data.pm_2_5_B);
    if ( (diff / (data.pm_2_5_A + data.pm_2_5_B) >
        kMaxPercentDiscrepancy) && diff > 1.2) {
      continue;
    }

    // keep which sensor is considered the primary one
    if (primaryIndex < 0) {
      primaryIndex = i;
    }

    cfvalues[count * 2] = data.pm_2_5_A;
    cfvalues[count * 2 + 1] = data.pm_2_5_B;
    count++;
  }

  if (primaryIndex < 0) {
    Serial.println("Could not identify a primary sensor!");
    sample.Set(0, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0);
  } else {
    Serial.print("Compute stats with n sensors = ");
    Serial.println(count);
    float avgRealTime, nmaeRealTime;
    mean_error(count * 2, cfvalues, avgRealTime, nmaeRealTime);
    Serial.print("real time avg = ");
    Serial.println(avgRealTime);
    Serial.print("NMAE = ");
    Serial.println(nmaeRealTime);

#if 0
    float avg10values[kMaxSensors];
    // Get out strong outliers
    count = 0;
    for (size_t i = 0; i < sensors.Count(); i++) {
      const SensorData& data = sensors.Data(i);
      float avg10min = data.averages[static_cast<int>(PmAvgIndexes::TenMinutes)];
      if ( fabs(avg10min-avgRealTime) < 5.0 * nmae*avg10min) {
        count++;
      }
#endif
    const SensorData& data = sensors.Data(primaryIndex);
    sample.Set(data.timestamp, 0.0, avgRealTime, 0.0, data.pressure, data.temperature,
               data.humidity, count, nmaeRealTime);
  }

  return count;
}

#endif

#endif
