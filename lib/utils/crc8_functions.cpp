#include "crc8_functions.h"

#include "crc_tables.h"

uint8_t crc8_maxim(const uint8_t *data, uint16_t datalen) {
  uint8_t crc = 0;
  if (datalen) do {
      crc = crc_table_maxim[crc ^ *data];
      data++;
    } while (--datalen);
  return crc;
}
