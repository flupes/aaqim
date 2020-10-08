#ifndef AAQIM_AIR_SAMPLE_H
#define AAQIM_AIR_SAMPLE_H

#include "cfaqi.h"

const uint32_t kCompactedSampleSize = 16;
const uint32_t kNaturalSampleSize = 32;

const uint32_t k2019epoch = 1546300800;  // Offset for compacted timestamps
const uint32_t kSecondsResolution = 60;  // Resoluton of the encoded timestamps

enum class FlagsBitsPos : uint8_t { IsValid = 0, MaxPos = 7 };

struct AirSampleData {
  uint8_t timestamp24[3];
  uint8_t reserved;
  uint16_t pm_1_0_short;
  uint16_t pm_2_5_short;
  uint16_t pm_10_0_short;
  uint16_t pressure_short;
  uint8_t temperature_byte;
  uint8_t humidity_byte;
  uint8_t stats_byte;  // 3 bits for sample count + 5 bits for mae
  uint8_t crc;
};

class AirSample {
 public:
  AirSample() { Set(k2019epoch, 0.0f, 0.0f, 0.0f, 1000.0f, 0, 0, 0, 0.0f); }

  AirSample(uint32_t seconds, float pm_1_0, float pm_2_5, float pm_10,
            float pressure, int temperature, int humidity, int count,
            float nmae) {
    Set(seconds, pm_1_0, pm_2_5, pm_10, pressure, temperature, humidity, count,
        nmae);
  }

  AirSample(const AirSampleData &data) {
    FromData(data);
  }

  void Set(uint32_t seconds, float pm_1_0, float pm_2_5, float pm_10,
           float pressure, int temperature, int humidity, int count,
           float nmae);

  void FromData(const AirSampleData &data);

  void ToData(AirSampleData &data);

  uint32_t Seconds() const { return seconds_; }
  float Pm_1_0() const{ return pm_1_0_cf_; }
  float Pm_2_5() const { return pm_2_5_cf_; }
  float Pm_10_0() const { return pm_10_0_cf_; }
  float Pm_2_5_Nmae() const { return pm_2_5_mae_; }
  float PressureMbar() const { return pressure_; }
  int16_t TemperatureF() const { return temperature_f_; }
  float TemperatureC() const { return ((float)temperature_f_ - 32.0f) * 5.0f / 9.0f; }
  int16_t AqiPm_2_5() const { return aqi_pm25_; }
  uint8_t HumidityPercent() const { return humidity_; }
  uint8_t SamplesCount() const { return samples_count_; }
  float MaeValue() const { return pm_2_5_mae_; }
  AqiLevel Level() const { return aqi_level_; }
  bool IsValid() const;

 protected:
  uint32_t seconds_;
  float pm_1_0_cf_;
  float pm_2_5_cf_;
  float pm_10_0_cf_;
  float pm_2_5_mae_;
  float pressure_;
  int16_t temperature_f_;
  int16_t aqi_pm25_;
  uint8_t humidity_;
  uint8_t samples_count_;
  AqiLevel aqi_level_;
  uint8_t flags_;
};

#endif
