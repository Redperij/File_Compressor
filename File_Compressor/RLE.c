#pragma warning(disable:4996) //disabling warning

#include "main.h"
#include "RLE.h"

/*
* CRC-calculation.
*/
uint16_t crc16(const uint8_t *data_p, unsigned int length) {
	uint8_t x;
	uint16_t crc = 0xFFFF;
	while (length--) {
		x = crc >> 8 ^ *data_p++;
		x ^= x >> 4;
		crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x << 5)) ^ ((uint16_t)x);
	}
	return crc;
}

/*
* Encodes given data array by rle rules.
*/
size_t rle_pack(uint8_t **array, size_t *size) {

	return 0;
}

/*
* Decodes given data array by rle rules.
*/
size_t rle_unpack(uint8_t **array, size_t *size) {

	return 0;
}