#ifndef AAQIM_DISPLAY_SAMPLES_H
#define AAQIM_DISPLAY_SAMPLES_H

#include "air_sample.h"
#include "flash_samples.h"

typedef FlashSamples<AirSampleData> FlashAirDataSamples;

template <size_t BUFFER_LENGTH>
class DisplaySamples {
 public:
  DisplaySamples(uint32_t periodInSeconds)
      : length_(BUFFER_LENGTH), period_(periodInSeconds) {}

  size_t Fill(FlashAirDataSamples &src, uint32_t now) {
    // Initialize min/max (cannot use INT16_MIN because it is already used
    // to mark "no data"
    min_ = INT16_MAX - 1;
    max_ = INT16_MIN + 1;

    uint32_t count = 0;
    if (!src.IsScanned() || src.IsEmpty()) {
      return count;
    }

    const size_t numberOfAvailableSamples =
        src.NumberOfSamples();  // this method is not efficient, so cache the
                                // value
    size_t samplesIndex = 0;
    size_t previousSampleIndex = UINT32_MAX;
    size_t bufferReversedIndex = 0;
    size_t bucketCount = 0;
    int32_t accumulator = 0;

    printf("==== now = %d\n", now);
    AirSampleData data;
    AirSample sample;
    while (bufferReversedIndex < length_ &&
           samplesIndex < numberOfAvailableSamples) {
      if (previousSampleIndex != samplesIndex) {
        src.ReadSample(samplesIndex, data);
        sample.FromData(data);
        previousSampleIndex = samplesIndex;
      }
      uint32_t sampleTimestamp = sample.Seconds();
      uint32_t bufferMaxTimestamp = now - bufferReversedIndex * period_;
      uint32_t bufferMinTimestamp = bufferMaxTimestamp - period_;

      printf("samplesIndex =        %d\n", samplesIndex);
      printf("sampleTimestamp =     %d (age = %d)\n", sampleTimestamp,
             now - sampleTimestamp);
      printf("bufferReversedIndex = %d\n", bufferReversedIndex);
      printf("buffer min/max ts =  [%d, %d] | age = (%d, %d)\n",
             bufferMinTimestamp, bufferMaxTimestamp, now - bufferMaxTimestamp,
             now - bufferMinTimestamp);
      printf("bucketCount =         %d | count = %d\n", bucketCount, count);

      if (bufferMinTimestamp < sampleTimestamp &&
          sampleTimestamp <= bufferMaxTimestamp) {
        printf("-- accumulate\n");
        accumulator += sample.AqiPm_2_5();
        bucketCount++;
        samplesIndex++;
      } else {
        if (sampleTimestamp <= bufferMinTimestamp) {
          // move to previous buffer slot
          if (bucketCount > 0) {
            printf("-- add sample\n");
            buffer_[length_ - bufferReversedIndex - 1] =
                accumulator / bucketCount;
            count++;
            accumulator = 0;
            bucketCount = 0;
          } else {
            // No samples belong to this time slice
            printf("-- mark no data\n");
            buffer_[length_ - bufferReversedIndex - 1] = INT16_MIN;
          }
          bufferReversedIndex++;
        }
        if (sampleTimestamp > bufferMaxTimestamp) {
          // skip sample
          printf("-- skip sample\n");
          samplesIndex++;
        }
      }
    }
    for (size_t r = bufferReversedIndex; r < length_; r++) {
      printf("## mark %d with no data\n", r);
      buffer_[length_ - r - 1] = INT16_MIN;
    }
    return count;
  }

 protected:
  size_t length_;
  uint32_t period_;
  int16_t buffer_[BUFFER_LENGTH];
  int16_t min_;
  int16_t max_;
};

#endif

#if 0
      if (sampleTimestamp < bufferMinTimestamp) {
        // move to previous buffer slot
        if (bucketCount > 0) {
          printf("-- add sample\n");
          buffer_[length_ - bufferReversedIndex - 1] =
              accumulator / bucketCount;
          count++;
          accumulator = 0;
          bucketCount = 0;
        } else {
          // No samples belong to this time slice
          printf("-- mark no data\n");
          buffer_[length_ - bufferReversedIndex - 1] = INT16_MIN;
        }
        bufferReversedIndex++;
      } else {
        if (sampleTimestamp > bufferMaxTimestamp) {
          // skip sample
          printf("-- skip sample\n");
          samplesIndex++;
        } else {
          printf("-- accumulate\n");
          accumulator += sample.AqiPm_2_5();
          bucketCount++;
          samplesIndex++;
        }
      }
    }
#endif