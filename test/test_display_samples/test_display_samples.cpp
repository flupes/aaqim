#include "aaqim_utils.h"
#include "display_samples.h"
#include "unity.h"

#if defined(ARDUINO)
EspFlash gFlash;
#else
#include "sim_flash.h"
SimFlash gFlash;
#endif

const uint32_t kFlashOffset = 0x000A0000;
const uint32_t kNowSeconds = k2019epoch + 365 * 24 * 3600;

FlashAirDataSamples gFlashSamples(gFlash, 64, kFlashOffset);

void TestFillFromEmptyFlash() {
  gFlashSamples.Begin(true);
  DisplaySamples<8, int16_t> displaySamples(300);
  size_t count = displaySamples.Fill(gFlashSamples, kNowSeconds);
  TEST_ASSERT_EQUAL(0, count);
}

void StoreSample(uint32_t age, float pm25) {
  static uint32_t counter = 1;
  dbg_printf("store sample # %d with age %d --> ts = %d\n", counter++, age,
             kNowSeconds - age);

  AirSample sample(kNowSeconds - age, 0.0f, pm25, 10.0 * counter, 1000.0f, 77,
                   33, 3, 0.5f);
  AirSampleData data;
  sample.ToData(data);
  gFlashSamples.StoreSample(data);
}

void StoreSerie() {
  // The target samples are designed to results in the current
  // display buffer (samples number indicates how many samples
  // should be sorted is this time slice)
  // | bucket  |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
  // | age   -2400   -2100   -1800   -1500   -1200   -900    -600    -300 now=0
  // | samples |   0   |   3   |   1   |   2   |   0   |   0   |   2   |   0   |
  // | pm_2_5  |       | 35.4  | 150.4 | 250.4 |       |       | 350.4 |       |
  // | aqi     |       |  100  |  200  |  300  |       |       |  400  |       |
  //
  //
  // We show:
  // - missing very recent and very old samples
  // - gap in the middle of the data
  // - uneven distribution of the samples

  StoreSample(1920, 35.4f);  // flash index = 7
  StoreSample(2040, 30.4f);  // flash index = 6
  // sample unordered: should not matter IF within a timeslice!
  StoreSample(1860, 40.4f);  // flash index = 5

  StoreSample(1620, 150.4);  // flash index = 4

  StoreSample(1440, 250.4f + 40.0f);  // flash index = 3
  // Sample at the edge: it should fall in bucket 4 (not 3)
  StoreSample(1260, 250.4f - 40.0f);  // flash index = 2

  StoreSample(540, 350.4f + 20.0f);  // flash index = 1
  // sample at the edge: it should fall in bucket 1
  StoreSample(300, 350.4f - 20.0f);  // flash index = 0
}

void TestFillDisplaySample() {
  // First we need to populate the flash...
  StoreSerie();

  DisplaySamples<8, int16_t> displaySamples(300);
  TEST_ASSERT_EQUAL(8, displaySamples.Length());

  size_t count = displaySamples.Fill(gFlashSamples, kNowSeconds);
  TEST_ASSERT_EQUAL(4, count);

  TEST_ASSERT_EQUAL(INT16_MIN, displaySamples.AqiPm_2_5(0));
  TEST_ASSERT_EQUAL(100, displaySamples.AqiPm_2_5(1));
  TEST_ASSERT_EQUAL(200, displaySamples.AqiPm_2_5(2));
  TEST_ASSERT_EQUAL(300, displaySamples.AqiPm_2_5(3));
  TEST_ASSERT_EQUAL(INT16_MIN, displaySamples.AqiPm_2_5(4));
  TEST_ASSERT_EQUAL(INT16_MIN, displaySamples.AqiPm_2_5(5));
  TEST_ASSERT_EQUAL(400, displaySamples.AqiPm_2_5(6));
  TEST_ASSERT_EQUAL(INT16_MIN, displaySamples.AqiPm_2_5(7));

  // Out of range index
  TEST_ASSERT_EQUAL(INT16_MAX, displaySamples.AqiPm_2_5(8));

#if defined(AAQIM_DEBUG)
  // For unknown reason, this does *not* print when running on the ESP8266!
  for (size_t s = 0; s < displaySamples.Length(); s++) {
    int16_t aqi = displaySamples.AqiPm_2_5(s);
    if (aqi == INT16_MIN) {
      printf("| N/A ");
    } else {
      printf("| %d ", aqi);
    }
  }
  printf("|\n");
#endif
}

#if defined(ARDUINO)
void loop() {}
void setup() {
#else
int main() {
#endif
  UNITY_BEGIN();
  RUN_TEST(TestFillFromEmptyFlash);
  RUN_TEST(TestFillDisplaySample);

  UNITY_END();
}
