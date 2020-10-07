#ifndef AAQIM_DISPLAY_SAMPLES_H
#define AAQIM_DISPLAY_SAMPLES_H

#include <limits>

#include "aaqim_utils.h"
#include "air_sample.h"
#include "flash_samples.h"

typedef FlashSamples<AirSampleData> FlashAirDataSamples;

/**
 * Create a linear buffer which is a "view" of sample stored on flash.
 *
 * Each element of the buffer is a bucket containing the average of
 * all the flash samples withing a timeslice defined by a *period*.
 *
 * @param BUFFER_LENGTH defines the length of the buffer
 * @param DATA_TYPE defines the type of data stored in the buffer
 */
template <size_t BUFFER_LENGTH, typename DATA_TYPE>
class DisplaySamples {
 public:
  /**
   * Constructor: just defines the period to be used.
   * @param periodInSeconds
   */
  DisplaySamples(uint32_t periodInSeconds)
      : length_(BUFFER_LENGTH), period_(periodInSeconds) {}

  /**
   * Fills the buffer from samples stored on flash.
   * 
   * @param MAPPING_FUNC Function to map the average for each time slice to
   *                     the bucket value. For example, for the air samples 
   *                     we compute the average on the pm25 concentration 
   *                     (linear scale), but desire to fill the buffer with
   *                     the AQI index (using pm25_to_aqi_value).
   *
   * @param src accessor to the AirData samples stored on flash
   *            (should become a template if we ever need retrieving other
   * types)
   * @param now Timestamp in seconds defining the most recent element in the
   * time time serie to retrieve. This value does not have to match an exact
   *            sample timestamp, and it can be either in the past of the future
   * of the last element store on flash. The buffer will be filled correctly by
   * either skipping the more recent flash samples, or filling the buffer with
   * N/A values. The oldest element to retrieve on flash will be defined by:
   *   timestamp >= now - BUFFER_LENGTH * period
   */
  template <typename MAPPING_FUNC>
  size_t Fill(FlashAirDataSamples &src, uint32_t now, MAPPING_FUNC mapf);

  /** Return the sample at the requested position in the buffer.
   * @param position of the sample requested
   *          - 0 = first element of the buffer = older sample of the serie
   *          - Length()-1 = last element of the buffer = most recent sample
   * @return AQI for the sample at this position
   *          - If index is out of range, return INT16_MAX
   *          - If no sample in the bucket, return INT16_MIN
   */
  int16_t Value(size_t position) {
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
  DATA_TYPE min_;
  DATA_TYPE max_;
  DATA_TYPE buffer_[BUFFER_LENGTH];

};

template <size_t BUFFER_LENGTH, typename DATA_TYPE>
template <typename MAPPING_FUNC>
size_t DisplaySamples<BUFFER_LENGTH, DATA_TYPE>::Fill(FlashAirDataSamples &src,
                                                      uint32_t now, MAPPING_FUNC mapf) {
  // Initialize min/max (cannot use INT16_MIN because it is already used
  // to mark "no data"
  min_ = std::numeric_limits<DATA_TYPE>::max() - 1;
  max_ = std::numeric_limits<DATA_TYPE>::min() + 1;

  // Initialize the full buffer with INT16_MIN = "no data available in this
  // bucket"
  for (size_t i = 0; i < length_; i++) {
    buffer_[i] = std::numeric_limits<DATA_TYPE>::min();
  }

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
               bufferMinTimestamp, bufferMaxTimestamp, now - bufferMaxTimestamp,
               now - bufferMinTimestamp);

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
            mapf(accumulator / (float)(bucketCount));
          count++;
          accumulator = 0;
          bucketCount = 0;
        } else {
          // No samples belong to this time slice
          dbg_printf(
              "-- no data for this bucket, just move on (already initialized "
              "N/A): # %d\n",
              bufferReversedIndex);
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
    buffer_[length_ - bufferReversedIndex - 1] =
        mapf(accumulator / (float)(bucketCount));
    bufferReversedIndex++;
    count++;
  }
  for (size_t r = bufferReversedIndex; r < length_; r++) {
    dbg_printf("## mark %d with no data\n", r);
    buffer_[length_ - r - 1] = INT16_MIN;
  }
  return count;
}

#endif
