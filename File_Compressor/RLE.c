#pragma warning(disable:4996) //disabling warning

#include "main.h"
#include "RLE.h"

#ifndef SPECIAL_CHARACTER
#define SPECIAL_CHARACTER 0x80
#endif // !SPECIAL_CHARACTER


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
* Returns 1 on error. 0 on success.
*/
size_t rle_pack(uint8_t **array, size_t *size) {
	if (*size == 0) return 1; //Array size must be bigger than zero.
	unsigned int i = 0; //Marks place in the *array
	unsigned int q = 0; //Marks place in created_array
	uint8_t *created_array = malloc((*size) * 3 * sizeof(uint8_t)); //In the worst case created array will be 3 times longer than previously.

	if (created_array == NULL) { //Memory allocation failed.
		return 1;
	}

	while (i < *size) {
		created_array[q] = SPECIAL_CHARACTER;
		if (array[0][i] != SPECIAL_CHARACTER) {
			created_array[q + 1] = 0x01;
			created_array[q + 2] = array[0][i];
			i++;
			while (created_array[q + 2] == array[0][i] && i < *size) {
				created_array[q + 1]++;
				i++;
			}
			q += 3;
		}
		else {
			created_array[q + 1] = 0x00;
			i++;
			if (array[0][i] == SPECIAL_CHARACTER && i < *size) {
				created_array[q + 1] = 0x02;
				created_array[q + 2] = SPECIAL_CHARACTER;
				i++;
				while (array[0][i] == SPECIAL_CHARACTER && i < *size) {
					created_array[q + 1]++;
					i++;
				}
				q++;
			}
			q += 2;
		}
	}
	free(*array);
	*array = NULL;

	*size = q; //Getting new size.
	*array = realloc(created_array, (*size) * sizeof(uint8_t)); //Shrinking new array to the contents and assigning it to the returned array pointer.
	if (*array == NULL) { //If reallocation will fail for some reason -> Take full created array.
		*array = created_array;
	}
	return 0;
}

/*
* Decodes given data array by rle rules.
*/
size_t rle_unpack(uint8_t **array, size_t *size) {

	return 0;
}