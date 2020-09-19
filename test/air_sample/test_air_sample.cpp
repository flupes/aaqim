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
  for (uint16_t t = -100; t<156; t++) {
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

void test_data_structure(void) {
  uint32_t sz = sizeof(AirSampleData);
  TEST_ASSERT_EQUAL(kCompactedSampleSize, sz);
}

#if defined(ARDUINO)
#include <Arduino.h>
void loop() {}
void setup() {
#else
int main(int argc, char **argv) {
#endif
  UNITY_BEGIN();
  RUN_TEST(test_data_structure);
  RUN_TEST(test_pressure);
  RUN_TEST(test_temperature);
  RUN_TEST(test_concentration);
  RUN_TEST(test_timestamp);
  UNITY_END();
}
