// pio test -f flash_info

#include <Arduino.h>
#include <FS.h>
#include "flash_hal.h"

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
  // FSInfo fsi;
  // SPIFFS.info(fsi);
  // info("FS totalBytes", fsi.totalBytes);
  // info("FS blockSize", fsi.blockSize);
  // info("FS pageSize", fsi.pageSize);

  info("FLASH_SECTOR_SIZE", FLASH_SECTOR_SIZE);
  info("FLASH_BLOCK_SIZE", FLASH_BLOCK_SIZE);
  info("_SPIFFS_start", (uint32_t)&_SPIFFS_start);
  info("_SPIFFS_end", (uint32_t)&_SPIFFS_end);
  info("SPIF_FLASH_SEC_SIZE", SPI_FLASH_SEC_SIZE);

  char msg[32];
  info("_FS_block", (uint32_t)&_FS_block);
  uint32_t fsStart = (uint32_t)&_FS_start;
  info("_FS_start", fsStart);
  sprintf(msg, "_FS_start = 0x%08X", fsStart);
  Serial.println(msg);
  info("_FS_end", (uint32_t)&_FS_end);
  info("_FS_page", (uint32_t)&_FS_page);

  info("FS_PHYS_ADDR", FS_PHYS_ADDR);
  info("FS_PHYS_SIZE", FS_PHYS_SIZE);
  info("FS_PHYS_PAGE", FS_PHYS_PAGE);
  info("FS_PHYS_BLOCK", FS_PHYS_BLOCK);

  uint32_t offset = FS_PHYS_ADDR;
  uint32_t number;
  for (size_t i = 0; i < 16; i++) {
    bool status = ESP.flashRead(offset, &number, 4);
    sprintf(msg, "#%d -> 0x%08X (status=%d)", i, number, status);
    Serial.println(msg);
    offset += 4;
  }

  // Read test
  offset = FS_PHYS_ADDR;
  uint32_t count = 0;
  unsigned long start = millis();
  for (uint32_t i=0; i<FS_PHYS_SIZE/4; i++) {
    ESP.flashRead(offset, &number, 4);
    if ( number != 0xFFFFFFFF ) {
      count++;
    }
    offset += 4;
  }
  unsigned long stop = millis();
  info("Elapsed time to read all flash (ms)", stop-start);
  info("number of used flash", count);

  Serial.println("done.");

  /* Results:

Flash ChipId: 1458270
Flash ChipSize: 4194304
Flash RealSize: 4194304
FLASH_SECTOR_SIZE: 4096
FLASH_BLOCK_SIZE: 65536
_SPIFFS_start: 1078984704
_SPIFFS_end: 1080008704
SPIF_FLASH_SEC_SIZE: 4096
_FS_block: 8192
_FS_start: 1078984704
_FS_start = 0x40500000
_FS_end: 1080008704
_FS_page: 256
FS_PHYS_ADDR: 3145728
FS_PHYS_SIZE: 1024000
FS_PHYS_PAGE: 256
FS_PHYS_BLOCK: 8192
#0 -> 0xFFFFFFFF (status=1)
#1 -> 0xFFFFFFFF (status=1)
#2 -> 0xFFFFFFFF (status=1)
#3 -> 0xFFFFFFFF (status=1)

*/

}

void loop() {}
