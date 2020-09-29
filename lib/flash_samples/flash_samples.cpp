#include "flash_samples.h"

#include <Arduino.h>
#include <flash_hal.h>

FlashSamples::FlashSamples(size_t sampleSize, size_t samplesLength,
                           uint32_t startOffset)
    : sampleSize_(sampleSize), scanned_(false), empty_(false) {
  // startOffset needs to be aligned with a sector!
  if (startOffset % SPI_FLASH_SEC_SIZE != 0) {
    startOffset = SPI_FLASH_SEC_SIZE * (1 + startOffset / SPI_FLASH_SEC_SIZE);
  }
  flashStorageLength_ = sampleSize * samplesLength;
  uint32_t available = FS_PHYS_SIZE - startOffset;
  if (available < 2 * SPI_FLASH_SEC_SIZE) {
    Serial.println("FlashSamples request is not valid:");
    Serial.println("  less than 2 secors available from the specified offset!");
    Serial.println("Stop now");
    while (1)
      ;
  }
  // we need at the very least 2 sectors to avoid loosing data
  // when erasing a used sector
  if (flashStorageLength_ < 2 * SPI_FLASH_SEC_SIZE) {
    flashStorageLength_ = 2 * SPI_FLASH_SEC_SIZE;
  }
  if (flashStorageLength_ > available) {
    flashStorageLength_ = available;
  }
  // normalize the length to an entire number of sectors
  if (flashStorageLength_ % SPI_FLASH_SEC_SIZE != 0) {
    flashStorageLength_ =
        SPI_FLASH_SEC_SIZE * (1 + flashStorageLength_ / SPI_FLASH_SEC_SIZE);
  }
  flashStorageStart_ = FS_PHYS_ADDR + startOffset;
  // Upper bound of the desired storage: this is the address AFTER the requested
  // storage!
  flashStorageEnd_ = flashStorageStart_ + flashStorageLength_;
  // flashStorageEnd_ = flashStorageStart_ + flashStorageLength_ - sampleSize_;
  firstSampleAddr_ = flashStorageStart_;
  lastSampleAddr_ = flashStorageStart_;
}

void FlashSamples::Begin(bool erase) {
  // For some debug scenarios, we may want to clear the flash first!
  if (erase) {
    uint32_t sector = flashStorageStart_ / SPI_FLASH_SEC_SIZE;
    for (uint32_t s = 0; s < SectorsInUse(); s++) {
      ESP.flashEraseSector(sector);
      sector++;
    }
  }

  uint32_t current = 0xFFFFFFFF;
  uint32_t previous = 0xFFFFFFFF;
  bool firstFound = false;
  bool lastFound = false;
  uint32_t addr = flashStorageStart_;
  for (uint32_t i = 0; i < flashStorageLength_; i += sampleSize_) {
    // Only check the first four byte of each sample
    // (do not write sample starting with 32 bits set to one!)
    ESP.flashRead(addr, &current, 4);
    if (previous == 0xFFFFFFFF && current != 0xFFFFFFFF) {
      if (!firstFound) {
        firstSampleAddr_ = addr;
        firstFound = true;
      }
    }
    if (previous != 0xFFFFFFFF && current == 0xFFFFFFFF) {
      if (!lastFound) {
        lastSampleAddr_ = addr - sampleSize_;
        lastFound = true;
      }
    }
    previous = current;
    addr += sampleSize_;
  }
  if (firstSampleAddr_ == lastSampleAddr_) {
    empty_ = true;
  }
  scanned_ = true;
}

size_t FlashSamples::NumberOfSamples() {
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

bool FlashSamples::ReadSample(size_t index, uint32_t* data) {
  if (index > NumberOfSamples() - 1) {
    return false;
  }
  uint32_t addr = lastSampleAddr_ - index * sampleSize_;
  if (addr < flashStorageStart_) {
    addr += flashStorageLength_;
  }
  return ESP.flashRead(addr, data, sampleSize_);
}

bool FlashSamples::StoreSample(uint32_t* data) {
  uint8_t status = 0;
  if (!empty_) {
    // increment only if the flash is not completely empty!
    lastSampleAddr_ += sampleSize_;
  }
  empty_ = false;
  if (lastSampleAddr_ >= flashStorageEnd_) {
    lastSampleAddr_ -= flashStorageLength_;
  }
  uint32_t endAddress = lastSampleAddr_ + sampleSize_;
  if (endAddress >= flashStorageEnd_) {
    endAddress -= flashStorageLength_;
  }
  uint32_t currentSector = lastSampleAddr_ / SPI_FLASH_SEC_SIZE;
  uint32_t necessarySector = endAddress / SPI_FLASH_SEC_SIZE;
  if (currentSector != necessarySector) {
    // Serial.print("lastSampleAddr : ");
    // Serial.println(lastSampleAddr_);
    // Serial.print("!!!! Erase sector : ");
    // Serial.println(necessarySector);
    bool result = ESP.flashEraseSector(necessarySector);
    if (!result) {
      status += 1;
    }
    firstSampleAddr_ += SPI_FLASH_SEC_SIZE;
    if (firstSampleAddr_ >= flashStorageEnd_) {
      firstSampleAddr_ -= flashStorageLength_;
    }
  }
  bool result = ESP.flashWrite(lastSampleAddr_, data, sampleSize_);
  if (!result) {
    status += 1;
  }
  return (status == 0);
}

size_t FlashSamples::SectorsInUse() {
  return (flashStorageEnd_ - flashStorageStart_) / SPI_FLASH_SEC_SIZE;
}

size_t FlashSamples::NominalCapacity() {
  return (SectorsInUse() - 1) * SPI_FLASH_SEC_SIZE / sampleSize_;
}

void FlashSamples::Info() {
  char msg[32];
  sprintf(msg, "Sample Size               : %u", sampleSize_);
  Serial.println(msg);
  sprintf(msg, "Flash Storage Start       : 0x%08X", flashStorageStart_);
  Serial.println(msg);
  sprintf(msg, "Flash Storage End         : 0x%08X", flashStorageEnd_);
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
