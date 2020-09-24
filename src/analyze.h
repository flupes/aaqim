#ifndef AAQIM_ANALYZE_H
#define AAQIM_ANALYZE_H

#if defined(ARDUINO)

#include <stdlib.h>

#include "air_sample.h"
#include "sensors.h"
#include "stats.h"

// Uncomment to use real time data rather than sensor computed 10min averages
// #define USE_PM_REAL_TIME

// PA sensors seem to be updated every two minutes.
// We consider the sensor valid even if we miss three beats
const int16_t kMaxReadingAgeMinutes = 7;

const float kMaxPercentDiscrepancy = 0.08f;
const float kMaxConcentrationDiscrepancy = 8.0f;
const float kConcentrationConsistencyThreshold = 100.0f;

size_t ComputeStats(const AirSensors& sensors, AirSample& sample,
                    int32_t& primaryIndex) {
  primaryIndex = -1;
  size_t count = 0;
#if defined(USE_PM_REAL_TIME)
  float cfvalues[2 * kMaxSensors];
#else
  float cfvalues[kMaxSensors];
#endif

  for (size_t i = 0; i < sensors.Count(); i++) {
    const SensorData& data = sensors.Data(i);

    // check age
    if (data.age_A > kMaxReadingAgeMinutes || data.age_B > kMaxReadingAgeMinutes) {
      continue;
    }

    // check sensors consistency
    bool rejected = false;
    float diff = fabs(data.pm_2_5_A - data.pm_2_5_B);
    float sum = data.pm_2_5_A + data.pm_2_5_B;
    if ( sum < 2.0 * kConcentrationConsistencyThreshold ) {
      if ( diff > kMaxConcentrationDiscrepancy ) {
        rejected = true;
      }
    } else {
      if ( (diff / sum) > 2.0 * kMaxPercentDiscrepancy ) {
        rejected = true;
      }
    }
    if ( rejected ) continue;

    // keep which sensor is considered the primary one
    if (primaryIndex < 0) {
      primaryIndex = i;
    }

#if defined (USE_PM_REAL_TIME)
    cfvalues[count * 2] = data.pm_2_5_A;
    cfvalues[count * 2 + 1] = data.pm_2_5_B;
    count++;
#else
    cfvalues[count++] = data.averages[static_cast<int>(PmAvgIndexes::TenMinutes)];
#endif 
  }

  if (primaryIndex < 0) {
    Serial.println("Could not identify a primary sensor!");
    sample.Set(0, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0);
  } else {
    Serial.print("Compute stats with n sensors = ");
    Serial.println(count);
    float avg, mae, nmae;
#if 0
    // If we want to use the real time data
    avg = mean_error(count * 2, cfvalues, mae, nmae);
#else
    avg = mean_error(count, cfvalues, mae, nmae);
#endif
    Serial.print("avg = ");
    Serial.print(avg);
    Serial.print(" | MAE = ");
    Serial.print(mae);
    Serial.print(" / NMAE = ");
    Serial.println(nmae);

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
    // Just forget about pm_1_0 and pm_10_0 for now
    sample.Set(data.timestamp, 0.0, avg, 0.0, data.pressure, data.temperature,
               data.humidity, count, mae);
  }

  return count;
}

#endif

#endif
