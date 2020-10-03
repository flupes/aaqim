#include "air_sample.h"

#include "unity.h"

void test_pressure(void) {
  TEST_ASSERT_EQUAL_UINT16(0, pressure_mbar_to_short(550.0));
  TEST_ASSERT_EQUAL_UINT16(65500, pressure_mbar_to_short(1300.0));
  for (float p=600.0; p<1250.0; p+=50.0) {
    uint16_t s = pressure_mbar_to_short(p);
    TEST_ASSERT_EQUAL_FLOAT(p, short_to_mbar_pressure(s));
  }
}

void test_temperature(void) {
  TEST_ASSERT_EQUAL_UINT8(0, temperature_f_to_byte(-101));
  TEST_ASSERT_EQUAL_UINT8(255, temperature_f_to_byte(156));
  for (int16_t t = -100; t<156; t++) {
    uint8_t b = temperature_f_to_byte(t);
    TEST_ASSERT_EQUAL_INT16(t, byte_to_temperature_f(b));
  }
}

void test_concentration(void) {
  TEST_ASSERT_EQUAL_UINT16(0, cf_to_short(-1.0f));
  TEST_ASSERT_EQUAL_UINT16(0xFFFF, cf_to_short(513.0));
  for (float c = 0.0; c<512.f; c+=0.25) {
    uint16_t s = cf_to_short(c);
    float r = short_to_cf(s);
    TEST_ASSERT_TRUE(fabs(c-r) < 0.01f);
  }
}

void test_timestamp(void) {
  uint8_t ts24[3];
  uint32_t now = k2019epoch + 1000 * 24 * 3600;
  unix_seconds_to_timestamp_22bits(now, ts24);
  uint32_t seconds;
  timestamp_22bits_to_unix_seconds(ts24, seconds);
  TEST_ASSERT_EQUAL_UINT32(now, seconds);
}

void test_stats(void) {
  uint8_t code;
  uint8_t count;
  float mae;

  stats_to_byte(0.0f, 1, code);
  byte_to_stats(code, mae, count);
  TEST_ASSERT_EQUAL_UINT8(1, count);
  TEST_ASSERT_EQUAL_FLOAT(0.0, mae);

  stats_to_byte(63.0f, 10, code);
  byte_to_stats(code, mae, count);
  TEST_ASSERT_EQUAL_UINT8(8, count);
  TEST_ASSERT_EQUAL_FLOAT(62.0, mae);

  stats_to_byte(9.5f, 2, code);
  byte_to_stats(code, mae, count);
  TEST_ASSERT_EQUAL_UINT8(2, count);
  TEST_ASSERT_EQUAL_FLOAT(10.0f, mae);

  stats_to_byte(31.0f, 6, code);
  byte_to_stats(code, mae, count);
  TEST_ASSERT_EQUAL_UINT8(6, count);
  TEST_ASSERT_EQUAL_FLOAT(30.0f, mae);

  stats_to_byte(62.0f, 8, code);
  byte_to_stats(code, mae, count);
  TEST_ASSERT_EQUAL_UINT8(8, count);
  TEST_ASSERT_EQUAL_FLOAT(62.0f, mae);

}

void test_data_structure(void) {
  TEST_ASSERT_EQUAL(kCompactedSampleSize, sizeof(AirSampleData));
  TEST_ASSERT_EQUAL(kNaturalSampleSize, sizeof(AirSample));

  uint32_t seconds = k2019epoch + 365 * 24 * 3600;
  AirSample input(seconds, 10.0f, 50.0f, 100.0f, 1000.0f, 77, 40, 5, 0.1f);

  AirSampleData data;
  input.SetData(data);
  AirSample output(data);
  TEST_ASSERT_EQUAL(seconds, output.Seconds());
  TEST_ASSERT_EQUAL(10.0f, output.Pm_1_0());
  TEST_ASSERT_EQUAL(50.0f, output.Pm_2_5());
  TEST_ASSERT_EQUAL(100.0f, output.Pm_10_0());
  TEST_ASSERT_EQUAL(1000.0f, output.PressureMbar());
  TEST_ASSERT_EQUAL(77, output.TemperatureF());
  TEST_ASSERT_EQUAL(25.0f, output.TemperatureC());
  TEST_ASSERT_EQUAL(40, output.HumidityPercent());
  TEST_ASSERT_EQUAL(5, output.SamplesCount());
  TEST_ASSERT_EQUAL(0.1f, output.Pm_2_5_Nmae());
}

void test_crc()
{
  uint32_t seconds = k2019epoch + 365 * 24 * 3600;
  AirSample input(seconds, 10.0f, 50.0f, 100.0f, 1000.0f, 77, 40, 5, 0.1f);

  AirSampleData data;
  input.SetData(data);

  AirSample outputOk(data);
  TEST_ASSERT_TRUE(outputOk.IsValid());

  data.reserved = 0x01;
  AirSample outputCorrupted(data);
  TEST_ASSERT_FALSE(outputCorrupted.IsValid());
}

#if defined(ARDUINO)
#include <Arduino.h>
void loop() {}
void setup() {
#else
int main(int argc, char **argv) {
#endif
  UNITY_BEGIN();
  RUN_TEST(test_pressure);
  RUN_TEST(test_temperature);
  RUN_TEST(test_concentration);
  RUN_TEST(test_timestamp);
  RUN_TEST(test_stats);
  RUN_TEST(test_data_structure);
  RUN_TEST(test_crc);
  UNITY_END();
}
