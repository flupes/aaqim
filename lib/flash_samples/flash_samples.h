#ifndef AAQIM_FLASH_SAMPLES_H
#define AAQIM_FLASH_SAMPLES_H

#include <Arduino.h>
#include <flash_hal.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Save or read samples on flash.
 *
 * The class implements a ring buffer on flash memory.
 *
 * The class takes care of erasing the proper sector if it was already in use.
 *
 * Fully dependant on the specific ESP8266 flash implementation (not a general
 * utility). Store samples on the flash memory normally reserved for the File
 * System, and allow easy access to the indexed samples.
 *
 * The class is not resilient to multiple instanciantion, so use with care!
 *
 */
template <typename T>
class FlashSamples {
 public:
  /** Declare the flash accessor.
   *
   * @param sampleLength Desired total number of samples to store on flash
   * The sampleLength will be clamped to the maximum flash capacity.
   * @param startOffset Optional offset from the nominal flash storage (bytes)
   * By default, startOffset is zero, and then the samples will be start to be
   * stored at the begining of the ESP8266 filesystem flash area.
   */

  FlashSamples(size_t samplesLength, uint32_t startOffset = 0);

  /** Retrieve the first/last sample addresses on the existing storage.
   *
   * This is not part of the constructor, to allow the user to only start
   * accessing the flash when desired.
   * @param erase Default false. If set to true, the flash used by this
   * class is first erase. This is a debug scenario only.
   */
  void Begin(bool erase = false);

  /** Returns the number of sample currently stored on flash.
   *
   * @return current number of samples, or UINT32_MAX if flash is not scanned
   * yet.
   */
  size_t NumberOfSamples();

  /** Write the given sample data to flash
   * @param pointer to the data to store permanently
   * @return true if the sample was successfully writen, false otherwise
   */
  bool StoreSample(const T& data);

  /** Retrieve the sample with the given index
   * @param index Index of the desired sample. 0 is the most recent sample (end
   *              of the serie) and NumberOfSample would be the oldest sample
   *              in the serie.
   * @param data Where to store the data read. The caller is responsible to have
   *             properly allocated this memory space
   * @return true if a sample at the given index could be read, false otherwise
   *         (like index out of range)
   */
  bool ReadSample(size_t index, T& data);

  /** Returns the number of sectors used by the FlashSample storage
   */
  size_t SectorsInUse() { return flashStorageLength_ / flashSectorSize_; }

  /** Returns the actual FlashSample capacity in number of samples.
   *
   * It may differ from the requested size: it will be at least two sectors of
   * samples, and since one sector needs to be cleared for rewrite, it will be
   * one sector less than the size requested in samples.
   */
  size_t NominalCapacity() {
    return (SectorsInUse() - 1) * flashSectorSize_ / sampleSize_;
  }

  uint32_t SampleSize() const { return sampleSize_; }

  uint32_t FlashStorageStart() const { return flashStorageStart_; }

  uint32_t FlashStorageLength() const { return flashStorageLength_; }

  uint32_t FlashStorageEnd() const {
    return flashStorageStart_ + flashStorageLength_;
  }

  uint32_t FlashSectorSize() const { return flashSectorSize_; }

  uint32_t FirstSampleAddr() const { return firstSampleAddr_; }

  uint32_t LastSampleAddr() const { return lastSampleAddr_; }

  bool IsScanned() const { return scanned_; }

  bool IsEmpty() const { return empty_; }

  void Info();

 protected:
  uint32_t WrapAddress(uint32_t addr) {
    if (addr >= FlashStorageEnd()) {
      addr -= flashStorageLength_;
    }
    return addr;
  }

  const uint32_t
      sampleSize_; /** Size of samples to store (fixed size for all storage) */
  const uint32_t flashSectorSize_; /** Flash sector size */
  uint32_t
      flashStorageStart_; /** Start of the flash to use (expressed using the
                      full flash range, typically start is around 3M) */
  uint32_t flashStorageLength_; /** Size of the flash storage to be used */
  uint32_t firstSampleAddr_;    /** Address of the firt (=older) sample (not
                                   necessary   zero since this is a ring buffer */
  uint32_t lastSampleAddr_;     /** Address of the last sample recoded */
  bool scanned_; /** Was Begin() called once to scane the flash? */
  bool empty_;   /** Is the flash area empty */
};

template <typename T>
FlashSamples<T>::FlashSamples(size_t samplesLength, uint32_t startOffset)
    : sampleSize_(sizeof(T)),
      flashSectorSize_(SPI_FLASH_SEC_SIZE),
      scanned_(false),
      empty_(false) {
  // startOffset needs to be aligned with a sector!
  if (startOffset % flashSectorSize_ != 0) {
    startOffset = flashSectorSize_ * (1 + startOffset / flashSectorSize_);
  }
  uint32_t length = sampleSize_ * samplesLength;
  uint32_t available = FS_PHYS_SIZE - startOffset;
  if (available < 2 * flashSectorSize_) {
    Serial.println("FlashSamples request is not valid:");
    Serial.println("  less than 2 secors available from the specified offset!");
    Serial.println("Stop now");
    while (1)
      ;
  }
  // we need at the very least 2 sectors to avoid loosing data
  // when erasing a used sector
  if (length < 2 * flashSectorSize_) {
    length = 2 * flashSectorSize_;
  }
  if (length > available) {
    length = available;
  }
  // normalize the length to an entire number of sectors
  if (length % flashSectorSize_ != 0) {
    length = flashSectorSize_ * (1 + length / flashSectorSize_);
  }
  flashStorageLength_ = length;
  flashStorageStart_ = FS_PHYS_ADDR + startOffset;
  firstSampleAddr_ = UINT32_MAX;
  lastSampleAddr_ = UINT32_MAX;
}

template <typename T>
void FlashSamples<T>::Begin(bool erase) {
  // For some debug scenarios, we may want to clear the flash first!
  if (erase) {
    uint32_t sector = flashStorageStart_ / flashSectorSize_;
    for (uint32_t s = 0; s < SectorsInUse(); s++) {
      ESP.flashEraseSector(sector);
      sector++;
    }
  }

  uint32_t current = 0xFFFFFFFF;
  uint32_t previous = 0xFFFFFFFF;
  uint32_t addr = flashStorageStart_;
  for (uint32_t i = 0; i < flashStorageLength_; i += sampleSize_) {
    // Only check the first four byte of each sample
    // (do not write sample starting with 32 bits set to one!)
    ESP.flashRead(addr, &current, 4);
    if (firstSampleAddr_ == UINT32_MAX) {
      if (previous == 0xFFFFFFFF && current != 0xFFFFFFFF) {
        firstSampleAddr_ = addr;
      }
    }
    if (lastSampleAddr_ == UINT32_MAX) {
      if (previous != 0xFFFFFFFF && current == 0xFFFFFFFF) {
        lastSampleAddr_ = addr - sampleSize_;
      }
    }
    previous = current;
    addr += sampleSize_;
  }
  if (firstSampleAddr_ == UINT32_MAX) {
    empty_ = true;
  }
  else {
    // there are samples, but we did not find the last one yet
    // (when at the very end of the flash space)
    if ( lastSampleAddr_ == UINT32_MAX) {
      lastSampleAddr_ = FlashStorageEnd() - sampleSize_;
    }
  }
  scanned_ = true;
}

template <typename T>
size_t FlashSamples<T>::NumberOfSamples() {
  uint32_t count = UINT32_MAX;
  if (scanned_) {
    // Only if flash has been scanned properly
    if (firstSampleAddr_ == lastSampleAddr_) {
      count = 0;
    } else {
      uint32_t span;
      if (firstSampleAddr_ < lastSampleAddr_) {
        span = lastSampleAddr_ - firstSampleAddr_;
      } else {
        span = flashStorageLength_ + lastSampleAddr_ - firstSampleAddr_;
      }
      count = span / sampleSize_ + 1;
    }
  }
  return count;
}

template <typename T>
bool FlashSamples<T>::ReadSample(size_t index, T& data) {
  if (index > NumberOfSamples() - 1) {
    return false;
  }
  uint32_t addr = lastSampleAddr_ - index * sampleSize_;
  if (addr < flashStorageStart_) {
    addr += flashStorageLength_;
  }
  uint32_t* ptr = (uint32_t*)(&data);
  return ESP.flashRead(addr, ptr, sampleSize_);
}

template <typename T>
bool FlashSamples<T>::StoreSample(const T& data) {
  uint8_t status = 0;
  // Handle empty flash versus non-empty
  if (empty_) {
    firstSampleAddr_ = flashStorageStart_;
    lastSampleAddr_ = flashStorageStart_;
    empty_ = false;
  } else {
    lastSampleAddr_ = WrapAddress(lastSampleAddr_ + sampleSize_);
  }

  // If everything went well, the flash memory space where we need
  // to write should already have been initialized
  uint32_t* ptr = (uint32_t*)(&data);
  bool result = ESP.flashWrite(lastSampleAddr_, ptr, sampleSize_);
  if (!result) {
    Serial.println("Error writing to flash :-(");
    Serial.println(lastSampleAddr_);
    status += 1;
  }

  // Potentially prepare the next sector for writting: this is necessary
  // to perform in advance in order to retrieve the end of the buffer
  // after a booting (othwerwise there would be no way to distinguish
  // between a last sample at the end of a buffer and the next sample
  // that would be the oldest one).
  uint32_t currentSector = lastSampleAddr_ / flashSectorSize_;
  uint32_t nextAddress = WrapAddress(lastSampleAddr_ + sampleSize_);
  uint32_t nextSector = nextAddress / flashSectorSize_;
  if (currentSector != nextSector) {
    Serial.print("Erase sector starting at addr = ");
    Serial.println(nextAddress, HEX);
    if (firstSampleAddr_ == nextAddress) {
      firstSampleAddr_ = WrapAddress(nextAddress + flashSectorSize_);
    }
    bool result = ESP.flashEraseSector(nextSector);
    if (!result) {
      Serial.println("Error erasing sector :-(");
      status += 1;
    }
  }
  return (status == 0);
}

template <typename T>
void FlashSamples<T>::Info() {
  char msg[32];
  sprintf(msg, "Sample Size               : %u", sampleSize_);
  Serial.println(msg);
  sprintf(msg, "Flash Storage Start       : 0x%08X", flashStorageStart_);
  Serial.println(msg);
  sprintf(msg, "Flash Storage Length      : 0x%08X", flashStorageLength_);
  Serial.println(msg);
  sprintf(msg, "Number of Sectors in Use  : %u", SectorsInUse());
  Serial.println(msg);
  sprintf(msg, "Capacity (in samples)     : %u", NominalCapacity());
  Serial.println(msg);
  if (scanned_) {
    sprintf(msg, "First Sample Addr         : 0x%08X", firstSampleAddr_);
    Serial.println(msg);
    sprintf(msg, "Last Sample Addr          : 0x%08X", lastSampleAddr_);
    Serial.println(msg);
    size_t n = NumberOfSamples();
    if (n != UINT32_MAX) {
      sprintf(msg, "Number of Samples         : %u", n);
    } else {
      sprintf(msg, "Number of Samples         : N/A");
    }
    Serial.println(msg);
  } else {
    Serial.println("Flash not scanned yet!");
  }
}

#endif
