#pragma warning(disable:4996) //disabling warning

/*
* CRC-calculation.
* const uint8_t *data_p - data array for which crc is calculated.
* unsigned int length - length of the data array.
* Returns counted 16 bit crc.
*/
uint16_t crc16(const uint8_t *data_p, unsigned int length);

/*
* Encodes given data array by rle rules.
* uint8_t **array - pointer to the data array, which will be rle coded. (changed and resized)
* size_t *size - pointer to the current size of an array. Will be changed according to the new array size.
* Returns 1 on error. 0 on success.
*/
size_t rle_pack(uint8_t **array, size_t *size);

/*
* Decodes given data array by rle rules.
* uint8_t **array - pointer to the data array, which will be rle coded. (changed and resized). Returned as NULL if wasn't able to revert at least 1 char.
* size_t *size - pointer to the current size of an array. Will be changed according to the new array size.
* Returns 16 bit crc on success and 0 on failure.
*/
uint16_t rle_unpack(uint8_t **array, size_t *size);