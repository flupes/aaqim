// pio test -f flash_info

#include <Arduino.h>
#include <FS.h>

extern "C" {
    #include "spi_flash.h"
}

extern "C" uint32_t _SPIFFS_end;
extern "C" uint32_t _SPIFFS_start;

void info(const char *str, uint32_t value) {
  Serial.print(str);
  Serial.print(": ");
  Serial.println(value);
}

void setup() {
  Serial.begin(115200);
  info("Flash ChipId", ESP.getFlashChipId());
  info("Flash ChipSize", ESP.getFlashChipSize());
  info("Flash RealSize", ESP.getFlashChipRealSize());
  FSInfo fsi;
  SPIFFS.info(fsi);
  info("FS totalBytes", fsi.totalBytes);
  info("FS blockSize", fsi.blockSize);
  info("FS pageSize", fsi.pageSize);

  info("FLASH_SECTOR_SIZE", FLASH_SECTOR_SIZE);
  info("FLASH_BLOCK_SIZE", FLASH_BLOCK_SIZE);
  info("_SPIFFS_end", (uint32_t)&_SPIFFS_end);
  info("_SPIFFS_start", (uint32_t)&_SPIFFS_start);
  info("SPIF_FLASH_SEC_SIZE", SPI_FLASH_SEC_SIZE);

  Serial.println("done.");
  // Result:
  // Flash ChipId: 1458415
  // Flash ChipSize: 4194304
  // Flash RealSize: 4194304
  // FS totalBytes: 4277137406
  // FS blockSize: 8192
  // FS pageSize: 256
}

void loop() {}
