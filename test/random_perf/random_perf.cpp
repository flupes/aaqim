#include <Arduino.h>

const size_t kMaxRandomNumbers = 4096;

int16_t randoms[kMaxRandomNumbers];

void setup()
{

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  unsigned long start = millis();
  ESP.random((uint8_t*)(randoms), kMaxRandomNumbers*sizeof(int16_t));
  unsigned long stop = millis();
  printf("Time elapsed to compute %d random bytes : %ldms\n", kMaxRandomNumbers*sizeof(int16_t), stop-start);

  for (size_t i=0; i<50; i++) {
    printf("%d ", randoms[i]);
  }

  start = millis();
  const size_t nbPixels = 4 * 100 * 144 / 3;
  const size_t xSeed = ESP.random() % kMaxRandomNumbers;
  const size_t ySeed = ESP.random() % kMaxRandomNumbers;
  printf("\n seedX = %d / seedY = %d\n", xSeed, ySeed);
  int32_t sumX = 0;
  int32_t sumY = 0;
  for (size_t r = 0; r < nbPixels; r++) {
    int16_t x = randoms[(r+xSeed)%kMaxRandomNumbers];
    int16_t y = randoms[(r+ySeed)%kMaxRandomNumbers];
    sumX += (int32_t)(x);
    sumY += (int32_t)(y);
  }
  stop = millis();
  printf("Time elapsed to compute %d coordinates : %ldms\n", nbPixels, stop-start);
  printf("Resulting sums = { %d, %d }\n", sumX, sumY);

}

void loop()
{
}
