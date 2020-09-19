#include "cfaqi.h"
#include "unity.h"

void test_function_pm25_to_aqi_out_of_range(void) {
  int16_t value;
  AqiLevel level;
  bool valid;

  valid = pm25_to_aqi(-10.0, value, level);
  TEST_ASSERT_FALSE(valid);
  TEST_ASSERT_EQUAL(value, -1);
  TEST_ASSERT_EQUAL(level, AqiLevel::OutOfRange);
  valid = pm25_to_aqi(600.0, value, level);
  TEST_ASSERT_FALSE(valid);
  TEST_ASSERT_EQUAL(value, 500);
  TEST_ASSERT_EQUAL(level, AqiLevel::OutOfRange);
}

void test_function_pm25_to_aqi_conversions(void) {
  const float concentrations[] = {0.0,   5.0,   10.0,  12.0,  12.1,  15.0,
                                  35.4,  35.5,  40.0,  55.4,  55.5,  100.0,
                                  150.4, 150.5, 200.0, 250.4, 250.5, 300.0,
                                  350.4, 350.5, 400.0, 500.4};
  // conversion from: https://www.airnow.gov/aqi/aqi-calculator-concentration/
  const int aqivalues[] = {0, 21,  42,  50,  51,  57,  100, 101,
                           112, 150, 151, 174, 200, 201, 250, 300,
                           301, 350, 400, 401, 434, 500};
  const int aqilevels[] = {0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3,
                           3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6};

  int i = 0;
  for (float c : concentrations) {
    int16_t value;
    AqiLevel level;
    bool valid = pm25_to_aqi(c, value, level);
    TEST_ASSERT_TRUE(valid);
    TEST_ASSERT_EQUAL(aqivalues[i], value);
    TEST_ASSERT_EQUAL(aqilevels[i], static_cast<int>(level));
    i++;
  }
}

#if defined(ARDUINO)
#include <Arduino.h>
void setup() {
#else
int main(int argc, char **argv) {
#endif
  UNITY_BEGIN();
  RUN_TEST(test_function_pm25_to_aqi_out_of_range);
  RUN_TEST(test_function_pm25_to_aqi_conversions);
  UNITY_END();
}

#if defined(ARDUINO)
void loop() {}
#endif
