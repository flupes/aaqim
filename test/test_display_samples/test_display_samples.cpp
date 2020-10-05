#include "display_samples.h"
#include "aaqim_utils.h"
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
  DisplaySamples<8> displaySamples(300);
  size_t count = displaySamples.Fill(gFlashSamples, kNowSeconds);
  TEST_ASSERT_EQUAL(0, count);
}

void StoreSample(uint32_t age, float pm25) {
  static uint32_t counter = 1;
  dbg_printf("store sample # %d with age %d --> ts = %d\n", counter++, age, kNowSeconds-age);

  AirSample sample(kNowSeconds - age, 0.0f, pm25, 10.0*counter, 1000.0f, 77, 33, 3,
                   0.5f);
  AirSampleData data;
  sample.ToData(data);
  gFlashSamples.StoreSample(data);
}

void StoreSerie() {
  // The target samples are designed to results in the current
  // display buffer (number indicates how many samples is this time slice)
  //
  // | 0 | 3 | 1 | 2 | 0 | 0 | 2 | 0 |
  //
  // - missing very recent and very old samples
  // - gap in the middle of the data
  // - uneven distribution of the samples
  StoreSample(2050, 110.0f);
  // sample unordered: should not matter IF within a timeslice!
  StoreSample(1900, 100.0f);
  StoreSample(2000, 90.0f);

  StoreSample(1600, 200.0f);

  StoreSample(1400, 250.0f);
  StoreSample(1300, 350.0f);

  StoreSample(500, 350.0f);
  StoreSample(400, 450.0f);
}

void TestFillDisplaySample() {
  // First we need to populate the flash...
  StoreSerie();

  DisplaySamples<8> displaySamples(300);
  size_t count = displaySamples.Fill(gFlashSamples, kNowSeconds);
  TEST_ASSERT_EQUAL(4, count);
}

int main() {

  UNITY_BEGIN();
  RUN_TEST(TestFillFromEmptyFlash);
  RUN_TEST(TestFillDisplaySample);

  UNITY_END();
}
