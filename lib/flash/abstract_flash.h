#ifndef AAQIM_ABSTRACT_FLASH_H
#define AAQIM_ABSTRACT_FLASH_H

#include <stdint.h>
#include <stdlib.h>

/** Minimal abstraction for the flash method we need.
 */
class AbstractFlash {
 public:
  virtual bool flashEraseSector(uint32_t sector) = 0;
  virtual bool flashWrite(uint32_t offset, uint32_t* data, size_t size) = 0;
  virtual bool flashRead(uint32_t offset, uint32_t* data, size_t size) = 0;
};

#endif
