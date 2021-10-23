#pragma warning(disable:4996) //disabling warning

#include "main.h"
#include "RLE.h"

#ifndef SPECIAL_CHARACTER
#define SPECIAL_CHARACTER 0x80
#endif // !SPECIAL_CHARACTER


/*
* CRC-calculation.
* const uint8_t *data_p - data array for which crc is calculated.
* unsigned int length - length of the data array.
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
* uint8_t **array - pointer to the data array, which will be rle coded. (changed and resized)
* size_t *size - pointer to the current size of an array. Will be changed according to the new array size.
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
		created_array[q] = SPECIAL_CHARACTER; //First add special character.
		//Simple case.
		if (array[0][i] != SPECIAL_CHARACTER) {
			created_array[q + 1] = 0x01; //Second byte shows count of repetitions.
			created_array[q + 2] = array[0][i]; //Third byte means repeating character.
			i++; //Moving one byte forward.
			//Loop through all occurances until it is possible to fill their count in one byte.
			while (i < *size && created_array[q + 1] <= 255 && created_array[q + 2] == array[0][i]) {
				created_array[q + 1]++; //Adding it to the count.
				i++; //Moving to next byte.
			}
			q += 3; //Moving three bytes forward in the new array.
		}
		//Special character case.
		else {
			created_array[q + 1] = 0x00; //Assume it is only one.
			i++; //Moving to next byte.
			if (i < *size && array[0][i] == SPECIAL_CHARACTER) { //Met it again.
				created_array[q + 1] = 0x02; //Now there are two of them.
				created_array[q + 2] = SPECIAL_CHARACTER; //Append itself after number of repetitions as demanded in the rle.
				i++; //Moving to next byte.
				//Loop through all occurances until it is possible to fill their count in one byte.
				while (i < *size && created_array[q + 1] <= 255 && array[0][i] == SPECIAL_CHARACTER) {
					created_array[q + 1]++; //Adding it to the count.
					i++; //Moving to next byte.
				}
				q++; //Moving one byte forward in the new array.
			}
			q += 2; //Moving two bytes forward in the new array.
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
* uint8_t **array - pointer to the data array, which will be rle coded. (changed and resized)
* size_t *size - pointer to the current size of an array. Will be changed according to the new array size.
* Returns 16 bit crc on success and 0 on failure.
*/
uint16_t rle_unpack(uint8_t **array, size_t *size) {
	uint16_t crc = 0; //crc from the header of the file.
	size_t new_size = 0; //Size of the newly created array. Will be passed to the size pointer.
	uint8_t *new_array = NULL;

	//Two first bytes are always crc.
	crc += array[0][0];
	crc = crc << 8;
	crc += array[0][1];

	//Removing first 2 bytes from an array.
	for (size_t i = 2; i < *size; i++) {
		array[0][i - 2] = array[0][i];
	}
	*size -= 2;

	//Decoding.
	for (size_t i = 0; i < *size; i++) {
		//If special character was met on expected position.
		if (array[0][i] == SPECIAL_CHARACTER) {
			i++;
			//Byte after special character is 0 -> Exception. One special character must be added in place of these two bytes.
			if (i < *size && array[0][i] == 0x00) {
				new_size++; //Only one character is added.
				//Allocating additional memory.
				uint8_t *temp_arr = realloc(new_array, new_size * sizeof(uint8_t));
				if (temp_arr == NULL) { //Reallocation failed. Memory error.
					free(new_array);
					new_array = NULL;
					return 0;
				}
				new_array = temp_arr; //Retrieve pointer.
				new_array[new_size - 1] = SPECIAL_CHARACTER;
			}
			//Simple case, 2 bytes after special character have special meaning.
			else if (i < *size){
				size_t pos = new_size; //Remember the current position in the new data array.
				new_size += array[0][i]; //Increase size by the number specified in byte.
				i++; //Move one byte forward.
				if (i < *size) {
					//Allocating additional memory.
					uint8_t *temp_arr = realloc(new_array, new_size * sizeof(uint8_t));
					if (temp_arr == NULL) { //Reallocation failed. Memory error.
						free(new_array);
						new_array = NULL;
						return 0;
					}
					new_array = temp_arr; //Retrieve pointer.

					//Adding all same bytes to the newly allocated space. 
					for (size_t q = pos; q < new_size; q++) {
						new_array[q] = array[0][i];
					}
				}
				else { //File ended before giving specific char to append? -> Decrease new_size and do nothing.
					new_size -= array[0][i - 1];
				}
			}
		}
	}
	//Get new size.
	*size = new_size;
	//Reallocating memory.
	uint8_t *temp_arr = realloc(*array, (*size) * sizeof(uint8_t));
	if (temp_arr == NULL) { //Reallocation failed. Memory error.
		free(new_array);
		new_array = NULL;
		return 0;
	}
	*array = temp_arr; //Retrieve pointer.
	//Rewrite contents to the returned array.
	for (size_t i = 0; i < (*size); i++) {
		array[0][i] = new_array[i];
	}

	free(new_array);
	new_array = NULL;

	return crc;
}