#ifndef AAQIM_AIR_SAMPLE_H
#define AAQIM_AIR_SAMPLE_H

#include "cfaqi.h"
#include "crc8_functions.h"
#include "sample_encoding.h"

const uint32_t kCompactedSampleSize = 16;
const uint32_t kNaturalSampleSize = 32;

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
    timestamp_22bits_to_unix_seconds(data.timestamp24, seconds_);
    pm_1_0_cf_ = short_to_cf(data.pm_1_0_short);
    pm_2_5_cf_ = short_to_cf(data.pm_2_5_short);
    pm_10_0_cf_ = short_to_cf(data.pm_10_0_short);
    pressure_ = short_to_mbar_pressure(data.pressure_short);
    temperature_f_ = byte_to_temperature_f(data.temperature_byte);
    humidity_ = data.humidity_byte;
    byte_to_stats(data.stats_byte, pm_2_5_mae_, samples_count_);
    pm25_to_aqi(pm_2_5_cf_, aqi_pm25_, aqi_level_);
    uint8_t crc = crc8_maxim((uint8_t *)(&data), 15);
    if (crc == data.crc) {
      set_bit(flags_, FlagsBitsPos::IsValid);
    } else {
      clear_bit(flags_, FlagsBitsPos::IsValid);
    }
  }

  void Set(uint32_t seconds, float pm_1_0, float pm_2_5, float pm_10,
           float pressure, int temperature, int humidity, int count,
           float nmae) {
    seconds_ = seconds;
    pm_1_0_cf_ = pm_1_0;
    pm_2_5_cf_ = pm_2_5;
    pm_10_0_cf_ = pm_10;
    pressure_ = pressure;
    temperature_f_ = temperature;
    if (humidity > 100) {
      humidity_ = 100;
    } else if (humidity < 0) {
      humidity = 0;
    } else {
      humidity_ = humidity;
    }
    samples_count_ = count;
    pm_2_5_mae_ = nmae;
    pm25_to_aqi(pm_2_5_cf_, aqi_pm25_, aqi_level_);
  }

  void SetData(AirSampleData &data) {
    unix_seconds_to_timestamp_22bits(seconds_, data.timestamp24);
    data.reserved = 0x00;
    data.pm_1_0_short = cf_to_short(pm_1_0_cf_);
    data.pm_2_5_short = cf_to_short(pm_2_5_cf_);
    data.pm_10_0_short = cf_to_short(pm_10_0_cf_);
    data.pressure_short = pressure_mbar_to_short(pressure_);
    data.temperature_byte = temperature_f_to_byte(temperature_f_);
    data.humidity_byte = humidity_;
    stats_to_byte(pm_2_5_mae_, samples_count_, data.stats_byte);
    data.crc = crc8_maxim((uint8_t *)(&data), 15);
  }

  uint32_t Seconds() { return seconds_; }
  float Pm_1_0() { return pm_1_0_cf_; }
  float Pm_2_5() { return pm_2_5_cf_; }
  float Pm_10_0() { return pm_10_0_cf_; }
  float Pm_2_5_Nmae() { return pm_2_5_mae_; }
  float PressureMbar() { return pressure_; }
  int16_t TemperatureF() { return temperature_f_; }
  float TemperatureC() { return ((float)temperature_f_ - 32.0f) * 5.0f / 9.0f; }
  int16_t AqiPm_2_5() { return aqi_pm25_; }
  uint8_t HumidityPercent() { return humidity_; }
  uint8_t SamplesCount() { return samples_count_; }
  float MaeValue() { return pm_2_5_mae_; }
  AqiLevel Level() { return aqi_level_; }
  bool IsValid() { return get_bit(flags_, FlagsBitsPos::IsValid); }

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
