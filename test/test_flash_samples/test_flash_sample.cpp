#include "flash_samples.h"
#include "unity.h"

#if defined(ARDUINO)
EspFlash gFlash;
#else
#include "sim_flash.h"
SimFlash gFlash;
#endif


const uint32_t kFlashOffset = 0x000A0000;
const uint32_t kNumberOfSectorToUse = 3;
const uint32_t kBytesToAllocate = kNumberOfSectorToUse * SPI_FLASH_SEC_SIZE;
const size_t kSampleSize = sizeof(uint64_t);
const size_t kMaxSampleLength = kBytesToAllocate / kSampleSize;
const size_t kSamplesPerSector = SPI_FLASH_SEC_SIZE / kSampleSize;

void TestUnaligned() {
  FlashSamples<uint64_t> unaligned(gFlash, kMaxSampleLength - 256, kFlashOffset - 512);

  TEST_ASSERT_EQUAL(FS_PHYS_ADDR + kFlashOffset, unaligned.FlashStorageStart());
  TEST_ASSERT_EQUAL(kBytesToAllocate, unaligned.FlashStorageLength());
  TEST_ASSERT_EQUAL(FS_PHYS_ADDR + kFlashOffset + kBytesToAllocate,
                    unaligned.FlashStorageEnd());
  TEST_ASSERT_FALSE(unaligned.IsScanned());
}

void TestAligned() {
  FlashSamples<uint64_t> aligned(gFlash, kMaxSampleLength, kFlashOffset);
  TEST_ASSERT_EQUAL(FS_PHYS_ADDR + kFlashOffset, aligned.FlashStorageStart());
  TEST_ASSERT_EQUAL(kBytesToAllocate, aligned.FlashStorageLength());
  TEST_ASSERT_EQUAL(FS_PHYS_ADDR + kFlashOffset + kBytesToAllocate,
                    aligned.FlashStorageEnd());
  TEST_ASSERT_EQUAL(kNumberOfSectorToUse, aligned.SectorsInUse());
  TEST_ASSERT_EQUAL(UINT32_MAX, aligned.FirstSampleAddr());
  TEST_ASSERT_EQUAL(UINT32_MAX, aligned.LastSampleAddr());
}

void TestErase() {
  FlashSamples<uint64_t> samples(gFlash, kMaxSampleLength, kFlashOffset);
  samples.Begin(true);
  TEST_ASSERT_TRUE(samples.IsScanned());
  TEST_ASSERT_TRUE(samples.IsEmpty());
  TEST_ASSERT_EQUAL(UINT32_MAX, samples.FirstSampleAddr());
  TEST_ASSERT_EQUAL(UINT32_MAX, samples.LastSampleAddr());
}

void TestWriteOnEmptyFirstSector() {
  FlashSamples<uint64_t> samples(gFlash, kMaxSampleLength, kFlashOffset);
  TEST_ASSERT_EQUAL(UINT32_MAX, samples.NumberOfSamples());
  samples.Begin();
  TEST_ASSERT_TRUE(samples.IsEmpty());
  TEST_ASSERT_EQUAL(0, samples.NumberOfSamples());
  bool r;
  uint64_t data = 0;
  for (size_t i = 0; i < kSamplesPerSector; i++) {
    r = samples.StoreSample(data);
    data++;
    TEST_ASSERT_TRUE(r);
  }
  TEST_ASSERT_EQUAL(samples.FlashStorageStart(), samples.FirstSampleAddr());
  TEST_ASSERT_EQUAL(samples.FlashStorageStart() +
                        (kSamplesPerSector - 1) * samples.SampleSize(),
                    samples.LastSampleAddr());
  TEST_ASSERT_EQUAL(kSamplesPerSector, samples.NumberOfSamples());

  data = UINT64_MAX;
  r = samples.ReadSample(0, data);
  TEST_ASSERT_TRUE(r);
  TEST_ASSERT_EQUAL(kSamplesPerSector - 1, data);
  r = samples.ReadSample(kSamplesPerSector - 1, data);
  TEST_ASSERT_TRUE(r);
  TEST_ASSERT_EQUAL(0, data);
  r = samples.ReadSample(kSamplesPerSector, data);
  TEST_ASSERT_FALSE(r);
}

void TestWriteOnSecondSector() {
  FlashSamples<uint64_t> samples(gFlash, kMaxSampleLength, kFlashOffset);
  samples.Begin();
  TEST_ASSERT_FALSE(samples.IsEmpty());
  TEST_ASSERT_EQUAL(kSamplesPerSector, samples.NumberOfSamples());

  bool r;
  uint64_t data = UINT64_MAX;
  r = samples.ReadSample(0, data);
  TEST_ASSERT_TRUE(r);
  TEST_ASSERT_EQUAL(kSamplesPerSector - 1, data);

  for (uint64_t i = 0; i < kSamplesPerSector; i++) {
    r = samples.StoreSample(++data);
    TEST_ASSERT_TRUE(r);
  }
  TEST_ASSERT_EQUAL(2 * kSamplesPerSector, samples.NumberOfSamples());
  TEST_ASSERT_EQUAL(samples.FlashStorageStart(), samples.FirstSampleAddr());
  TEST_ASSERT_EQUAL(samples.FlashStorageStart() +
                        (2 * kSamplesPerSector - 1) * samples.SampleSize(),
                    samples.LastSampleAddr());

  r = samples.ReadSample(0, data);
  TEST_ASSERT_TRUE(r);
  TEST_ASSERT_EQUAL(2 * kSamplesPerSector - 1, data);
  r = samples.ReadSample(2 * kSamplesPerSector - 1, data);
  TEST_ASSERT_TRUE(r);
  TEST_ASSERT_EQUAL(0, data);
  r = samples.ReadSample(2 * kSamplesPerSector, data);
  TEST_ASSERT_FALSE(r);
}

void TestWriteOnThirdSector() {
  FlashSamples<uint64_t> samples(gFlash, kMaxSampleLength, kFlashOffset);
  samples.Begin();
  TEST_ASSERT_FALSE(samples.IsEmpty());
  TEST_ASSERT_EQUAL(2 * kSamplesPerSector, samples.NumberOfSamples());

  bool r;
  uint64_t data = UINT64_MAX;
  r = samples.ReadSample(0, data);
  TEST_ASSERT_TRUE(r);
  TEST_ASSERT_EQUAL(2 * kSamplesPerSector - 1, data);

  // Store samples until 1 before the end of the reserved flash
  for (uint64_t i = 0; i < kSamplesPerSector - 1; i++) {
    r = samples.StoreSample(++data);
    TEST_ASSERT_TRUE(r);
  }
  TEST_ASSERT_EQUAL(3 * kSamplesPerSector - 1, samples.NumberOfSamples());
  TEST_ASSERT_EQUAL(samples.FlashStorageStart(), samples.FirstSampleAddr());
  r = samples.ReadSample(samples.NumberOfSamples() - 1, data);
  TEST_ASSERT_TRUE(r);
  TEST_ASSERT_EQUAL(0, data);

  // One last store to fill the reserved flash
  r = samples.ReadSample(0, data);
  r = samples.StoreSample(++data);
  TEST_ASSERT_TRUE(r);
  // First sector should have been wiped out (in preparation of new write and
  // to mark the boundary with old data) --> we lost a full sector of sample
  TEST_ASSERT_EQUAL(2 * kSamplesPerSector, samples.NumberOfSamples());
  TEST_ASSERT_EQUAL(samples.FlashStorageStart() + SPI_FLASH_SEC_SIZE,
                    samples.FirstSampleAddr());
  TEST_ASSERT_EQUAL(samples.FlashStorageEnd() - kSampleSize,
                    samples.LastSampleAddr());

  r = samples.ReadSample(0, data);
  TEST_ASSERT_TRUE(r);
  TEST_ASSERT_EQUAL(3 * kSamplesPerSector - 1, data);
  r = samples.ReadSample(samples.NumberOfSamples() - 1, data);
  TEST_ASSERT_TRUE(r);
  TEST_ASSERT_EQUAL(kSamplesPerSector, data);
  r = samples.ReadSample(2 * kSamplesPerSector, data);
  TEST_ASSERT_FALSE(r);
}

void TestWriteAgainOnFirstAndSecond() {
  FlashSamples<uint64_t> samples(gFlash, kMaxSampleLength, kFlashOffset);
  samples.Begin();
  TEST_ASSERT_FALSE(samples.IsEmpty());

  TEST_ASSERT_EQUAL(samples.FlashStorageStart() + SPI_FLASH_SEC_SIZE,
                    samples.FirstSampleAddr());
  TEST_ASSERT_EQUAL(samples.FlashStorageEnd() - kSampleSize,
                    samples.LastSampleAddr());
  TEST_ASSERT_EQUAL(2 * kSamplesPerSector, samples.NumberOfSamples());

  bool r;
  uint64_t data = UINT64_MAX;
  r = samples.ReadSample(0, data);
  TEST_ASSERT_TRUE(r);
  TEST_ASSERT_EQUAL(3 * kSamplesPerSector - 1, data);

  for (uint64_t i = 0; i < kSamplesPerSector / 2; i++) {
    r = samples.StoreSample(++data);
    TEST_ASSERT_TRUE(r);
  }
  TEST_ASSERT_EQUAL(2 * kSamplesPerSector + kSamplesPerSector / 2,
                    samples.NumberOfSamples());
  TEST_ASSERT_EQUAL(samples.FlashStorageStart() + SPI_FLASH_SEC_SIZE,
                    samples.FirstSampleAddr());
  TEST_ASSERT_EQUAL(
      samples.FlashStorageStart() + SPI_FLASH_SEC_SIZE / 2 - kSampleSize,
      samples.LastSampleAddr());

  for (uint64_t i = 0; i < kSamplesPerSector; i++) {
    r = samples.StoreSample(++data);
    TEST_ASSERT_TRUE(r);
  }
  TEST_ASSERT_EQUAL(2 * kSamplesPerSector + kSamplesPerSector / 2,
                    samples.NumberOfSamples());
  TEST_ASSERT_EQUAL(samples.FlashStorageStart() + 2 * SPI_FLASH_SEC_SIZE,
                    samples.FirstSampleAddr());
  TEST_ASSERT_EQUAL(samples.FlashStorageStart() + SPI_FLASH_SEC_SIZE +
                        SPI_FLASH_SEC_SIZE / 2 - kSampleSize,
                    samples.LastSampleAddr());

  r = samples.ReadSample(0, data);
  TEST_ASSERT_TRUE(r);
  TEST_ASSERT_EQUAL(4 * kSamplesPerSector + kSamplesPerSector / 2 - 1, data);
}

#if defined(ARDUINO)
void loop() {}

void setup() {
  Serial.begin(115200);
#else
  int main() {
#endif

  UNITY_BEGIN();
  TEST_ASSERT_EQUAL(8, kSampleSize);
  TEST_ASSERT_EQUAL(4096 * 3, kBytesToAllocate);
  TEST_ASSERT_EQUAL(1536, kMaxSampleLength);

  RUN_TEST(TestUnaligned);
  RUN_TEST(TestAligned);

  // Note: this following test needs to be peformed in order since
  // the result depends of the previous flash state.
  RUN_TEST(TestErase);
  RUN_TEST(TestWriteOnEmptyFirstSector);
  RUN_TEST(TestWriteOnSecondSector);
  RUN_TEST(TestWriteOnThirdSector);
  RUN_TEST(TestWriteAgainOnFirstAndSecond);
  UNITY_END();
}
