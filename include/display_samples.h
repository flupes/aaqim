#ifndef AAQIM_DISPLAY_SAMPLES_H
#define AAQIM_DISPLAY_SAMPLES_H

#include "aaqim_utils.h"
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

    // Initialize the full buffer with INT16_MIN = "no data available in this
    // bucket"
    for (size_t i = 0; i < length_; i++) {
      buffer_[i] = INT16_MIN;
    }

    uint32_t count = 0;
    if (!src.IsScanned() || src.IsEmpty()) {
      min_ = 0;
      max_ = 0;
      return count;
    }

    const size_t numberOfAvailableSamples =
        src.NumberOfSamples();  // this method is not efficient, so cache the
                                // value
    size_t samplesIndex = 0;
    size_t previousSampleIndex = UINT32_MAX;
    size_t bufferReversedIndex = 0;
    size_t bucketCount = 0;
    float samplePm_2_5 = 0.0f;
    float accumulator = 0.0f;

    dbg_printf("==== now = %d\n", now);
    AirSampleData data;
    AirSample sample;
    while (bufferReversedIndex < length_ &&
           samplesIndex < numberOfAvailableSamples) {
      if (previousSampleIndex != samplesIndex) {
        src.ReadSample(samplesIndex, data);
        sample.FromData(data);
        previousSampleIndex = samplesIndex;
        // The AQI is a non-linear scale. So to perform a correct average
        // we use the initial concentration. This forces to reconvert
        // the final results to AQI.
        samplePm_2_5 = sample.Pm_2_5();
        dbg_printf("Read sample # %d : pm_2_5 = %.1f\n", samplesIndex,
                   samplePm_2_5);
        if (samplePm_2_5 < min_) {
          min_ = samplePm_2_5;
        }
        if (samplePm_2_5 > max_) {
          max_ = samplePm_2_5;
        }
      }
      uint32_t sampleTimestamp = sample.Seconds();
      uint32_t bufferMaxTimestamp = now - bufferReversedIndex * period_;
      uint32_t bufferMinTimestamp = bufferMaxTimestamp - period_;

      dbg_printf("samplesIndex = %d / bufferReversedIndex = %d\n", samplesIndex,
                 bufferReversedIndex);
      dbg_printf("sample age = %d (ts=%d)\n", now - sampleTimestamp,
                 sampleTimestamp);
      dbg_printf("buffer min/max ts =  [%d, %d] | age = [%d, %d]\n",
                 bufferMinTimestamp, bufferMaxTimestamp,
                 now - bufferMaxTimestamp, now - bufferMinTimestamp);

      if (bufferMinTimestamp < sampleTimestamp &&
          sampleTimestamp <= bufferMaxTimestamp) {
        accumulator += samplePm_2_5;
        dbg_printf("-- accumulate with %.1f (sample age = %d) : sum = %.1f\n",
                   samplePm_2_5, now - sampleTimestamp, accumulator);
        bucketCount++;
        samplesIndex++;
      } else {
        if (sampleTimestamp <= bufferMinTimestamp) {
          // move to previous buffer slot
          if (bucketCount > 0) {
            buffer_[length_ - bufferReversedIndex - 1] =
                pm_to_aqi(accumulator / (float)(bucketCount));
            // dbg_printf(
            //     "-- add sample into bucket # %d (avg count=%d) : avg = %.1f "
            //     "--> aqi = %d\n",
            //     bufferReversedIndex, bucketCount, avg, value);
            count++;
            accumulator = 0;
            bucketCount = 0;
          } else {
            // No samples belong to this time slice
            dbg_printf("-- mark no data for bucket # %d\n",
                       bufferReversedIndex);
            // Seems that this call is now redundant... Keep here to explain the
            // algorith?
            buffer_[length_ - bufferReversedIndex - 1] = INT16_MIN;
          }
          bufferReversedIndex++;
        }
        if (sampleTimestamp > bufferMaxTimestamp) {
          // skip sample
          dbg_printf("-- skip sample with flash index = %d\n", samplesIndex);
          samplesIndex++;
        }
      }
    }
    // flush last accumulated samples
    if (bucketCount > 0) {
      // int16_t aqi = pm_to_aqi( accumulator / (float)(bucketCount) );
      // dbg_printf(
      //     "-- last bucket: index = %d (avg count=%d) : avg = %.1f "
      //     "--> aqi = %d\n",
      //     bufferReversedIndex, bucketCount, avg, value);
      buffer_[length_ - bufferReversedIndex - 1] =
          pm_to_aqi(accumulator / (float)(bucketCount));
      bufferReversedIndex++;
      count++;
    }
    for (size_t r = bufferReversedIndex; r < length_; r++) {
      dbg_printf("## mark %d with no data\n", r);
      buffer_[length_ - r - 1] = INT16_MIN;
    }
    return count;
  }

  /** Return the sample at the requested position in the buffer.
   * @param position of the sample requested
   *          - 0 = first element of the buffer = older sample of the serie
   *          - Length()-1 = last element of the buffer = most recent sample
   * @return AQI for the sample at this position
   *          - If index is out of range, return INT16_MAX
   *          - If no sample in the bucket, return INT16_MIN
   */
  int16_t AqiPm_2_5(size_t position) {
    if (position >= length_) {
      return INT16_MAX;
    } else {
      return buffer_[position];
    }
  }

  size_t Length() const { return length_; }

  int16_t SerieMin() const { return min_; }

  int16_t SerieMax() const { return max_; }

 protected:
  size_t length_;
  uint32_t period_;
  int16_t min_;
  int16_t max_;
  int16_t buffer_[BUFFER_LENGTH];

  int16_t pm_to_aqi(float cf) {
    AqiLevel level;
    int16_t value;
    pm25_to_aqi(cf, value, level);
    return value;
  }

};

#endif
