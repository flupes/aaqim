#ifndef AAQIM_CRC8_FUNCTIONS_H
#define AAQIM_CRC8_FUNCTIONS_H

// Software CRC-8/MAXIM calculation using a lookup table.
//
// More information on the CRC-8 Maxim/Dow 1-Wire:
//   https://reveng.sourceforge.io/crc-catalogue/1-15.htm#crc.cat-bits.8
// 
// Code extracted from (to be used on native platform too):
//   https://github.com/FrankBoesing/FastCRC
//
// Selection of the CRC should be using:
//   https://users.ece.cmu.edu/~koopman/roses/dsn04/koopman04_crc_poly_embedded.pdf
// The CRC-8/MAXIM  should provide a HD=4 up to 85 bits words, but drop to HD=2 for 
// 128 bits :-(
// So it look like a 0x97 CRC would be a better solution for our samples that are
// 15 bytes (without the CRC) = 120 bits, since it provides a HD=4 until 119 bits!
// 

#include "crc_tables.h"

uint8_t crc8_maxim(const uint8_t *data, uint16_t datalen)
{
	uint8_t crc = 0;
	if (datalen) do {
		crc = crc_table_maxim[crc ^ *data];
		data++;
	} while (--datalen);
	return crc;
}

#endif
