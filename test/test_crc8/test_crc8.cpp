
#include "crc8_functions.h"
#include "unity.h"

void TestCrc8Maxim() {
  // Checksum computed with: https://crccalc.com/
  uint8_t data_64[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
  uint8_t crc64 = crc8_maxim(data_64, 8);
  TEST_ASSERT_EQUAL_UINT8(0x7B, crc64);

  uint8_t data_40[5] = {0x12, 0x34, 0x56, 0x78, 0x9A};
  uint8_t crc40 = crc8_maxim(data_40, 5);
  TEST_ASSERT_EQUAL_UINT8(0xBC, crc40);

  uint8_t data_128[16] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
                          0x0f, 0xed, 0xcb, 0xa9, 0x87, 0x65, 0x43, 0x21};
  uint8_t crc128 = crc8_maxim(data_128, 16);
  TEST_ASSERT_EQUAL_UINT8(0x81, crc128);
}

#if defined(ARDUINO)
#include <Arduino.h>
void loop() {}
void setup() {
#else
int main(int argc, char **argv) {
#endif
  UNITY_BEGIN();
  RUN_TEST(TestCrc8Maxim);

  UNITY_END();
}
