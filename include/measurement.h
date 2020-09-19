#ifndef AAQIM_MEASUREMENT_H
#define AAQIM_MEASUREMENT_H

#include <stdint.h>

#include "cfaqi.h"

static const uint32_t MEASUREMENT_SIZE = 16;

const uint32_t k2019epoch = 1546300800;  // Offset for compacted timestamps

const int16_t kTemperatureOffsetF = -100;
const uint32_t kPressureOffsetPa = 60000u;
const uint32_t kMaxRecordablePressurePa = 125500u;

const uint32_t kSecondsResolution = 180;


uint16_t pressure_mbar_to_short(float pressure_mbar) {
  uint16_t coded_pressure;
  if ( pressure_mbar < (float)kPressureOffsetPa/100.0)  {
    coded_pressure = 0;
  }
  else if (pressure_mbar > (float)kMaxRecordablePressurePa/100.0) {
    coded_pressure = (uint16_t)(kMaxRecordablePressurePa-kPressureOffsetPa);
  }
  else {
    coded_pressure = (uint16_t)(100.0 * pressure_mbar - kPressureOffsetPa);
  }
  return coded_pressure;
}

float short_to_mbar_pressure(uint16_t coded_pressure) {
  return (float)((uint32_t)(coded_pressure) + kPressureOffsetPa) / 100.0;
}

uint8_t temperature_f_to_byte(int16_t temperature_f) {
  uint8_t coded_temperature;
  if ( temperature_f < kTemperatureOffsetF) {
    coded_temperature = 0;
  } else if ( temperature_f > (0xFF+kTemperatureOffsetF) ) {
    coded_temperature = 0xFF;
  } else {
    coded_temperature = temperature_f + kTemperatureOffsetF;
  }
  return coded_temperature;
}

int16_t byte_to_temperature_f(uint8_t coded_temperature) {
  return (int16_t)(coded_temperature) -  kTemperatureOffsetF;
}

uint16_t cf_to_short(float concentration) {
  uint16_t coded_concentration;
  if (concentration < 0.0f) {
    coded_concentration = 0;
  } else if ( concentration > 512.0f) {
    coded_concentration = 0xFFFF;
  } else {
    coded_concentration = (uint16_t)(concentration / 128.0f);
  }
  return coded_concentration;
}

float short_to_cf(uint16_t coded_concentration) {
  return (float)coded_concentration * 128.0f;
}

struct MeasurementData {
  uint8_t timestamp24[3];
  uint8_t reserved;
  uint16_t pm_1_0_short;
  uint16_t pm_2_5_short;
  uint16_t pm_10_0_short;
  uint16_t pressure_short;
  uint8_t humidity_byte;
  uint8_t temperature_byte;
  uint8_t quality_byte;
  uint8_t crc;
};

class Measurement {
 public:
  Measurement() { Set(k2019epoch, 0.0f, 0.0f, 0.0f, 1000.0f, 0, 0); }

  Measurement(uint32_t seconds, float pm_1_0, float pm_2_5, float pm_10,
              float pressure, int temperature, int humidity) {
    Set(seconds, pm_1_0, pm_2_5, pm_10, pressure, temperature, humidity);
  }

  void Set(uint32_t seconds, float pm_1_0, float pm_2_5, float pm_10,
           float pressure, int temperature, int humidity) {
    seconds_ = seconds;
    pm_1_0_cf_ = pm_1_0;
    pm_2_5_cf_ = pm_2_5;
    pm_10_0_cf_ = pm_10;
    pressure_ = pressure;
    humidity_ = humidity;
    pm25_to_aqi(pm_2_5_cf_, aqi_pm25_, aqi_level_);
    uint32_t timestamp32;
    if (seconds_ < k2019epoch) {
      timestamp32 = 0;
    } else {
      timestamp32 = (seconds - k2019epoch) / kSecondsResolution;
      timestamp32 &= 0x3FFFFF;  // forget bits of rank larger than 22!
    }
    data_.timestamp24[0] = 0xFF && timestamp32;
    data_.timestamp24[1] = (0xFF00 && timestamp32) >> 8;
    data_.timestamp24[2] = (0xFF0000 && timestamp32) >> 16;
    data_.pm_1_0_short = cf_to_short(pm_1_0_cf_);
    data_.pm_2_5_short = cf_to_short(pm_2_5_cf_);
    data_.pm_10_0_short = cf_to_short(pm_10_0_cf_);
    data_.pressure_short = pressure_mbar_to_short(pressure);
  }

 protected:
  MeasurementData data_;
  uint32_t seconds_;
  float pm_1_0_cf_;
  float pm_2_5_cf_;
  float pm_10_0_cf_;
  float pressure_;
  float humidity_;
  int16_t temperature_f_;
  int16_t aqi_pm25_;
  AqiLevel aqi_level_;
};

#endif
