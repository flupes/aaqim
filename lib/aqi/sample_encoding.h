#ifndef AAQIM_SAMPLE_ENCODING_H
#define AAQIM_SAMPLE_ENCODING_H

#include <stdint.h>

const int16_t kTemperatureOffsetF = -100;
const uint32_t kPressureOffsetPa = 60000U;
const uint32_t kMaxRecordablePressurePa = 125500U;

uint16_t pressure_mbar_to_short(float pressure_mbar);
float short_to_mbar_pressure(uint16_t coded_pressure);
uint8_t temperature_f_to_byte(int16_t temperature_f);
int16_t byte_to_temperature_f(uint8_t coded_temperature);
uint16_t cf_to_short(float concentration);
float short_to_cf(uint16_t coded_concentration);
void unix_seconds_to_timestamp_22bits(uint32_t seconds, uint8_t ts24[]);
void timestamp_22bits_to_unix_seconds(const uint8_t ts24[], uint32_t &seconds);
void stats_to_byte(float mae, uint8_t count, uint8_t &code);
void byte_to_stats(uint8_t code, float &mae, uint8_t &count);

#endif
