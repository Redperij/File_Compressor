#pragma warning(disable:4996) //disabling warning

/*
* CRC-calculation.
*/
uint16_t crc16(const uint8_t *data_p, unsigned int length);

/*
* Encodes given data array by rle rules.
*/
size_t rle_pack(uint8_t **array, size_t *size);

/*
* Decodes given data array by rle rules.
*/
size_t rle_unpack(uint8_t **array, size_t *size);