#include "measurement.h"

#include "unity.h"

void test_data_structure(void) {
  uint32_t sz = sizeof(MeasurementData);
  TEST_ASSERT_EQUAL(MEASUREMENT_SIZE, sz);
}

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

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_data_structure);
  RUN_TEST(test_pressure);
  RUN_TEST(test_temperature);
  UNITY_END();
  return 0;
}
