#include "stats.h"

#include "unity.h"

void test_avg_float_1(void) {
    const float data[] = {1.0, 2.0, 3.0, 3.0, 3.0, 4.0, 5.0 };
    float mae;
    float nmae;
    float avg = mean_error(7, data, mae, nmae);
    TEST_ASSERT_EQUAL_FLOAT(3.0, avg);
    TEST_ASSERT_EQUAL_FLOAT(6.0/7.0, mae);
    TEST_ASSERT_EQUAL_FLOAT(0.2857143, nmae);
}

void test_avg_float_100(void) {
    const float data[] = {100.0, 200.0, 300.0, 300.0, 300.0, 400.0, 500.0 };
    float mae;
    float nmae;
    float avg = mean_error(7, data, mae, nmae);
    TEST_ASSERT_EQUAL_FLOAT(300.0, avg);
    TEST_ASSERT_EQUAL_FLOAT(600.0/7.0, mae);
    TEST_ASSERT_EQUAL_FLOAT(0.2857143, nmae);
}

void test_avg_int_10(void) {
    const int data[] = {10, 20, 30, 30, 30, 40, 50 };
    int mae;
    float nmae;
    int avg = mean_error(7, data, mae, nmae);
    TEST_ASSERT_EQUAL_INT(30, avg);
    TEST_ASSERT_EQUAL_INT(60/7, mae);
    TEST_ASSERT_EQUAL_FLOAT(0.2857143, nmae);
}

#if defined(ARDUINO)
#include <Arduino.h>
void setup() {
#else
int main(int argc, char **argv) {
#endif
  UNITY_BEGIN();
  RUN_TEST(test_avg_float_1);
  RUN_TEST(test_avg_float_100);
  RUN_TEST(test_avg_int_10);
  UNITY_END();
}

#if defined(ARDUINO)
void loop() {}
#endif
