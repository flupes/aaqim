#ifndef AAQIM_SIM_FLASH_H
#define AAQIM_SIM_FLASH_H

#include "abstract_flash.h"

#include <string.h>

#if !defined(ARDUINO)

const uint32_t SPI_FLASH_SEC_SIZE = 0x1000;

const uint32_t FS_PHYS_ADDR = 0x03000000;
const uint32_t FS_PHYS_SIZE = 0x000FA000;

class SimFlash : public AbstractFlash {
 public:
  SimFlash() {
    for (uint32_t i = 0; i < FS_PHYS_SIZE; i++) {
      memory_[i] = 0xFF;
    }
  }

  bool flashEraseSector(uint32_t sector) {
    uint32_t addr = sector * SPI_FLASH_SEC_SIZE;
    if (FS_PHYS_ADDR <= addr &&
        (addr + SPI_FLASH_SEC_SIZE) < (FS_PHYS_ADDR + FS_PHYS_SIZE)) {
      for (uint32_t i = 0; i < SPI_FLASH_SEC_SIZE; i++) {
        memory_[addr - FS_PHYS_ADDR] = 0xFF;
        addr++;
      }
      return true;
    } else {
      return false;
    }
  }

  bool flashWrite(uint32_t offset, uint32_t* data, size_t size) {
    if (FS_PHYS_ADDR <= offset &&
        (offset + size) < (FS_PHYS_ADDR + FS_PHYS_SIZE)) {
      uint32_t index = offset - FS_PHYS_ADDR;
      for (uint32_t i = 0; i < size; i++) {
        if (memory_[index] != 0xFF) {
          return false;
        }
        index++;
      }
      memcpy(memory_ + offset - FS_PHYS_ADDR, data, size);
      return true;
    } else {
      return false;
    }
  }

  bool flashRead(uint32_t offset, uint32_t* data, size_t size) {
    if (FS_PHYS_ADDR <= offset &&
        (offset + size) < (FS_PHYS_ADDR + FS_PHYS_SIZE)) {
      memcpy(data, memory_ + offset - FS_PHYS_ADDR, size);
      return true;
    } else {
      return false;
    }
  }

 protected:
  uint8_t memory_[FS_PHYS_SIZE];
};

#endif  // if !defined(ARDUINO)

#endif