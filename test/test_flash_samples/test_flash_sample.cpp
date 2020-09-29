#include <Arduino.h>

#include "flash_samples.h"

// #define CLEAR_TEST_FLASH 1

void setup() {
  Serial.begin(115200);

  const uint32_t kTestFlashOffset = 112 * SPI_FLASH_SEC_SIZE;

  FlashSamples samplesA(16, kTestFlashOffset / 16);
  FlashSamples samplesB(4, 100, kTestFlashOffset);

#if 0  
  Serial.println("Samples A before Begin");
  samplesA.Info();
  Serial.println("Samples A after Begin");
  samplesA.Begin();
  samplesA.Info();
  Serial.println("");
#endif

  Serial.println("Samples B");
#if defined(CLEAR_TEST_FLASH)
  samplesB.Begin(true);
#else
  samplesB.Begin();
#endif
  samplesB.Info();

  bool result = false;
  uint32_t number = 0;
  if ( samplesB.NumberOfSamples() > 0 ) {
    result = samplesB.ReadSample(0, &number);
    if ( !result ) {
      Serial.println("Failed to read the last element!");
    }
  }
  Serial.println("Write 400 samples...");
  for (size_t i=0; i<400; i++) {
    number++;
    result = samplesB.StoreSample(&number);
    if ( !result ) {
      Serial.print("Failed to write element #");
      Serial.println(i);
    }
  }
  samplesB.Info();
  for (size_t s = 0; s < 4; s++) {
    samplesB.ReadSample(s, &number);
    Serial.print(s);
    Serial.print(" -> ");
    Serial.println(number);
  }

}

void loop() {}