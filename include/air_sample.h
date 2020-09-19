#ifndef AAQIM_AIR_SAMPLE_H
#define AAQIM_AIR_SAMPLE_H

#include <stdint.h>

#include "cfaqi.h"

static const uint32_t kCompactedSampleSize = 16;

const uint32_t k2019epoch = 1546300800;  // Offset for compacted timestamps

const int16_t kTemperatureOffsetF = -100;
const uint32_t kPressureOffsetPa = 60000U;
const uint32_t kMaxRecordablePressurePa = 125500U;

const uint32_t kSecondsResolution = 60;

uint16_t pressure_mbar_to_short(float pressure_mbar) {
  uint16_t coded_pressure;
  if (pressure_mbar < (float)kPressureOffsetPa / 100.0) {
    coded_pressure = 0;
  } else if (pressure_mbar > (float)kMaxRecordablePressurePa / 100.0) {
    coded_pressure = (uint16_t)(kMaxRecordablePressurePa - kPressureOffsetPa);
  } else {
    coded_pressure = (uint16_t)(100.0 * pressure_mbar - kPressureOffsetPa);
  }
  return coded_pressure;
}

float short_to_mbar_pressure(uint16_t coded_pressure) {
  return (float)((uint32_t)(coded_pressure) + kPressureOffsetPa) / 100.0;
}

uint8_t temperature_f_to_byte(int16_t temperature_f) {
  uint8_t coded_temperature;
  if (temperature_f < kTemperatureOffsetF) {
    coded_temperature = 0;
  } else if (temperature_f > (0xFF + kTemperatureOffsetF)) {
    coded_temperature = 0xFF;
  } else {
    coded_temperature = temperature_f + kTemperatureOffsetF;
  }
  return coded_temperature;
}

int16_t byte_to_temperature_f(uint8_t coded_temperature) {
  return (int16_t)(coded_temperature)-kTemperatureOffsetF;
}

uint16_t cf_to_short(float concentration) {
  uint16_t coded_concentration;
  if (concentration < 0.0f) {
    coded_concentration = 0;
  } else if (concentration > 512.0f) {
    coded_concentration = 0xFFFF;
  } else {
    coded_concentration = (uint16_t)(concentration * 128.0f);
  }
  return coded_concentration;
}

float short_to_cf(uint16_t coded_concentration) {
  return (float)coded_concentration / 128.0f;
}

void unix_seconds_to_timestamp_22bits(uint32_t seconds, uint8_t ts24[]) {
  uint32_t timestamp32;
  if (seconds < k2019epoch) {
    timestamp32 = 0;
  } else {
    timestamp32 = (seconds - k2019epoch) / kSecondsResolution;
    timestamp32 &= 0x3FFFFF;  // forget bits of rank larger than 22!
  }
  ts24[0] = timestamp32 >> 16;
  ts24[1] = (0xFFFF & timestamp32) >> 8;
  ts24[2] = 0xFF & timestamp32;
}

void timestamp_22bits_to_unix_seconds(const uint8_t ts24[], uint32_t &seconds) {
  uint32_t timestamp32;
  timestamp32 = ((uint32_t)(ts24[0]) << 16) + ((uint16_t)(ts24[1]) << 8) + ts24[2];
  seconds = timestamp32 * kSecondsResolution + k2019epoch;
}

void stats_to_byte(float nmae, uint8_t count, uint8_t &code) {
  code = ( 0x0F & count ) << 4;
  // let's assume that nmea was computed right and is positive! 
  code |= 0x0F & (uint8_t)(nmae * 16.0);
}

void byte_to_stats(uint8_t code, float &nmae, uint8_t &count) {
  count = code >> 4;
  nmae = (float)(0x0F & code) / 16.0;
}

struct AirSampleData {
  uint8_t timestamp24[3];
  uint8_t reserved;
  uint16_t pm_1_0_short;
  uint16_t pm_2_5_short;
  uint16_t pm_10_0_short;
  uint16_t pressure_short;
  uint8_t temperature_byte;
  uint8_t humidity_byte;
  uint8_t stats_byte;
  uint8_t crc;
};

class AirSample {
 public:
  AirSample() { Set(k2019epoch, 0.0f, 0.0f, 0.0f, 1000.0f, 0, 0, 0, 0.0f); }

  AirSample(uint32_t seconds, float pm_1_0, float pm_2_5, float pm_10,
              float pressure, int temperature, int humidity, int count, float nmae) {
    Set(seconds, pm_1_0, pm_2_5, pm_10, pressure, temperature, humidity, count, nmae);
  }

  AirSample(const AirSampleData &data) {
    timestamp_22bits_to_unix_seconds(data.timestamp24, seconds_);
    pm_1_0_cf_ = short_to_cf(data.pm_1_0_short);
    pm_2_5_cf_ = short_to_cf(data.pm_2_5_short);
    pm_10_0_cf_ = short_to_cf(data.pm_10_0_short);
    pressure_ = short_to_mbar_pressure(data.pressure_short);
    temperature_f_ = byte_to_temperature_f(data.temperature_byte);
    humidity_ = data.humidity_byte;
    byte_to_stats(data.stats_byte, pm_2_5_nmae_, samples_count_);
    pm25_to_aqi(pm_2_5_cf_, aqi_pm25_, aqi_level_);
  }

  void Set(uint32_t seconds, float pm_1_0, float pm_2_5, float pm_10,
           float pressure, int temperature, int humidity, int count, float nmae) {
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
    pm_2_5_nmae_ = nmae;
    pm25_to_aqi(pm_2_5_cf_, aqi_pm25_, aqi_level_);
  }

  void SetData(AirSampleData &data) {
    unix_seconds_to_timestamp_22bits(seconds_, data.timestamp24);
    data.pm_1_0_short = cf_to_short(pm_1_0_cf_);
    data.pm_2_5_short = cf_to_short(pm_2_5_cf_);
    data.pm_10_0_short = cf_to_short(pm_10_0_cf_);
    data.pressure_short = pressure_mbar_to_short(pressure_);
    data.temperature_byte = temperature_f_to_byte(temperature_f_);
    data.humidity_byte = humidity_;
    stats_to_byte(pm_2_5_nmae_, samples_count_, data.stats_byte);
    data.crc = 0;  // TOFIX !
  }

 protected:
  uint32_t seconds_;
  float pm_1_0_cf_;
  float pm_2_5_cf_;
  float pm_10_0_cf_;
  float pm_2_5_nmae_;
  float pressure_;
  int16_t temperature_f_;
  int16_t aqi_pm25_;
  uint8_t humidity_;
  uint8_t samples_count_;
  AqiLevel aqi_level_;
};

#endif
