#ifndef AAQIM_SAMPLE_ENCODING_H
#define AAQIM_SAMPLE_ENCODING_H

#include <stdint.h>

const uint32_t k2019epoch = 1546300800;  // Offset for compacted timestamps
const uint32_t kSecondsResolution = 60;  // Resoluton of the encoded timestamps

const int16_t kTemperatureOffsetF = -100;
const uint32_t kPressureOffsetPa = 60000U;
const uint32_t kMaxRecordablePressurePa = 125500U;

enum class FlagsBitsPos : uint8_t { IsValid = 0, MaxPos = 7 };

void set_bit(uint8_t &byte, FlagsBitsPos pos) {
  byte |= (0x01 << static_cast<uint8_t>(pos));
}

void clear_bit(uint8_t &byte, FlagsBitsPos pos) {
  byte &= ~(0x01 << static_cast<uint8_t>(pos));
}

bool get_bit(uint8_t byte, FlagsBitsPos pos) {
  byte &= (0x01 << static_cast<uint8_t>(pos));
  return (byte >> static_cast<uint8_t>(pos));
}

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
    coded_temperature = temperature_f - kTemperatureOffsetF;
  }
  return coded_temperature;
}

int16_t byte_to_temperature_f(uint8_t coded_temperature) {
  return (int16_t)(coded_temperature) + kTemperatureOffsetF;
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
  timestamp32 =
      ((uint32_t)(ts24[0]) << 16) + ((uint16_t)(ts24[1]) << 8) + ts24[2];
  seconds = timestamp32 * kSecondsResolution + k2019epoch;
}

/* Code both sensor count and mae on a single byte.
  Warning: count should be in the range [1 .. 8] (zero not valid)!
  */
void stats_to_byte(float mae, uint8_t count, uint8_t &code) {
  if (count > 8) {
    count = 8;
  }
  // We assume that a sample should not be created if zero sensors were
  // available! if count = 0, then the encoding will be wrong :-(
  code = (0x0F & (count - 1)) << 5;
  // Let's use a resolution of 2 AQI units, which would let encode a MAE up
  // to 65 on 5 bits.
  if (mae > 63.0) {
    mae = 63.0;
  }
  code |= 0x1F & static_cast<uint8_t>(roundf((mae - 0.01) / 2.0));
}

void byte_to_stats(uint8_t code, float &mae, uint8_t &count) {
  count = 1 + (code >> 5);
  mae = (float)(0x1F & code) * 2.0;
}

#endif
