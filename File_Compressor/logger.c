#pragma warning(disable:4996) //disabling warning

#include "main.h"
#include "logger.h"

#ifndef COMPRESSED_KEY
#define COMPRESSED_KEY "Compressed" //Must start from 'C'
#endif // !COMPRESSED_KEY
#ifndef DECOMPRESSED_KEY
#define DECOMPRESSED_KEY "Decompressed" //Must not start from 'C'
#endif // !DECOMPRESED_KEY

/*
* Prints log to the user.
* const char *filename_log - name of the log file.
* Returns 1 on error. 0 on success.
*/
int view_log(const char *filename_log) {
	FILE *log_file = fopen(filename_log, "r");
	LOGGED_FILE *entries = NULL; //Array with entries from log file.
	size_t size; //size of the entries array.

	if (log_file == NULL) {
		printf("Log is empty. No files are encrypted.\n");
		return 1;
	}

	size = read_log(log_file, &entries);
	printf("Operation     | Original name of the file      | Original size   | CRC  | Compressed size | Compressed name of the file   \n");
	for (int i = 0; i < size; i++) {
		printf("%-13s | %-30s | %13d b | %2x | %13d b | %-30s\n", entries[i].compressed ? COMPRESSED_KEY : DECOMPRESSED_KEY, entries[i].orig_name, entries[i].orig_size, entries[i].crc, entries[i].comp_size, entries[i].comp_name);
	}
	printf("\n\n");

	//Free memory.
	for (size_t i = 0; i < size; i++) {
#pragma warning (disable:6001) //Nope, they are initialized. size won't increase if they are not.
			free(entries[i].comp_name);
			free(entries[i].orig_name);
#pragma warning (default:6001)
		}
	free(entries);
	entries = NULL;

	rewind(log_file);
	fclose(log_file);
	return 0;
}

/*
* Reads all file contents into data array.
* FILE *file - opened file in binary format to read from.
* uint8_t **data - ponter to the data array to read to.
* size_t *size - pointer to the size of the data array. (size will be returned to that)
* Returns: '2' in case of memory mistake. '1' in case if no file was provided. '0' in case of success.
*/
int file_in(FILE *file, uint8_t **data, size_t *size) {
	if (file == NULL) return 1; //There must be file.
	if (*data != NULL) { //data must be empty
		free(*data);
		*data = NULL;
	}

	*size = 0; //Size starts from 0

	*data = malloc(1 * sizeof(uint8_t)); //Initial memory allocation for data.
	if (*data == NULL) { //Allocation failed -> memory mistake.
		return 2;
	}
	
	//Reading file byte by byte.
	fread(&data[0][*size], 1, 1, file);
	while (!feof(file) && !ferror(file)) {
		uint8_t *temp_arr = NULL;
		(*size)++;

		temp_arr = realloc(*data, (*size + 1) * sizeof(uint8_t));
		if (temp_arr == NULL) { //Reallocation failed -> memory mistake.
			return 2;
		}
		*data = temp_arr; //Getting pointer back.
		fread(&data[0][*size], 1, 1, file);
	}

	if (*size == 0) { //File was empty.
		free(*data);
		*data = NULL;
	}

    return 0;
}

/*
* Writes all data contents into the file.
* FILE *file - opened file in binary format to write data to.
* const uint8_t *data - array with data to write.
* const size_t size - size of the data array.
* const uint16_t crc - checksum to pu as a header. If specified as 0 nothing will be added before the data.
* Returns 1 on error. 0 on success.
*/
int file_out(FILE *file, const uint8_t *data, const size_t size, const uint16_t crc) {
	if (file == NULL) return 1; //file must be provided.
	//Add crc as a header. (I have no idea, why fwrite() was wirting it backwards)
	uint8_t temp = crc >> 8;
	if (crc != 0) fwrite(&temp, sizeof(uint8_t), 1, file);
	temp = crc; //Loss of data planned.
	if (crc != 0) fwrite(&temp, sizeof(uint8_t), 1, file);
	//Write all data contents to the file.
	for (size_t i = 0; i < size; i++) {
		fwrite(&data[i], 1, 1, file);
	}

    return 0;
}

/*
* Gets new name for the file. Cuts the end of the name in case of reaching the limit. (Shouldn't happen, at least not on a daily basis)
* char **string - pointer to the string with the old name.
* Returns 1 on error. 0 on success.
*/
int get_new_name(char **string) {
	size_t pos = strlen(*string) - 1;
	char *temp_str = NULL;

	while (string[0][pos] != '.' && pos != 0) pos--; //Finding position of the current file extension.
	if (pos == 0) pos = strlen(*string); //No file extension -> create one.

	//Memory reallocation
	temp_str = realloc(*string, (pos + 5) * sizeof(char)); //Maximum elements to add is 5. ( 4, actually, since filename must be at least 1 symbol + '\0' , 5 is taken to suspend the warning.)
	if (temp_str == NULL) {
		return 1;
	}
	*string = temp_str; //getting pointer back.


	while (pos + 3 >= MAX_FILENAME) pos--; //Cutting the end of the file name in order to fit the maximum available name. (too lazy to make it better way)

	//Building up the extension and terminating string.
	string[0][pos] = '.';
	string[0][pos + 1] = 'r';
	string[0][pos + 2] = 'l';
	string[0][pos + 3] = 'e';
	string[0][pos + 4] = '\0';
	
	//Memory reallocation
	temp_str = realloc(*string, (strlen(*string) + 1) * sizeof(char));
	if (temp_str == NULL) {
		return 1;
	}
	*string = temp_str; //getting pointer back.

    return 0;
}

/*
* Checks log entries for given filename and checksum.
* const char *filename_log - name of the log file.
* const char *comp_filename - string with the name of the comressed file.
* const uint16_t crc - Checksum must match in order to recognize file.
* Returns 'true' if entry exists, 'false' if no such entry found.
*/
bool log_check_entry(const char *filename_log, const char *comp_filename, const uint16_t crc) {
	FILE *log_file = fopen(filename_log, "r");
	LOGGED_FILE *entries = NULL;
	size_t size = 0;
	bool flag = false; //flag signals if entry was met.

	if (log_file == NULL) {
		return false;
	}

	size = read_log(log_file, &entries);
	rewind(log_file);
	fclose(log_file);

	if (size == 0) return false;

	//Only entry with the same compressed name and checksum will return true.
	for (size_t i = 0; i < size; i++) {
		if (!strcmp(entries[i].comp_name, comp_filename) && crc == entries[i].crc && entries[i].compressed) {
			flag = true;
			break; //No point to continue.
		}
	}


	for (size_t i = 0; i < size; i++) {
#pragma warning (disable:6001) //Nope, they are initialized. size won't increase if they are not.
		free(entries[i].comp_name);
		free(entries[i].orig_name);
#pragma warning (default:6001)
	}
	free(entries);
	entries = NULL;

    return flag;
}

/*
* Retrieves last original name of the compressed file with the same crc from the log.
* const char *filename_log - name of the log file.
* const char *comp_filename - string with the name of the comressed file.
* const uint16_t crc - Checksum must match in order to recognize file.
* Returns NULL pointer on failure or string with the name of the original file.
*/
char *log_retrieve_name(const char *filename_log, const char *comp_name, const uint16_t crc) {
	FILE *log_file = fopen(filename_log, "r");
	LOGGED_FILE *entries = NULL;
	size_t size;
	char *orig_name = malloc(MAX_FILENAME * sizeof(char));
	if (log_file == NULL || orig_name == NULL) {
		return NULL;
	}

	size = read_log(log_file, &entries);
	rewind(log_file);
	fclose(log_file);
#pragma warning (disable:6001) //Nope, they are initialized. size won't increase if they are not.
	if (entries != NULL) {
		//Only entry with the same compressed name and checksum will take original name.
		for (size_t i = 0; i < size; i++) {
			if (entries[i].comp_name != NULL && entries[i].orig_name != NULL && entries[i].compressed) {
				if (!strcmp(entries[i].comp_name, comp_name) && crc == entries[i].crc) {
					strcpy(orig_name, entries[i].orig_name);
				}
			}
			//Free memory.
			free(entries[i].comp_name);
			free(entries[i].orig_name);
		}
#pragma warning (default:6001)
		free(entries);
		entries = NULL;
	}
	else {
		free(orig_name);
		orig_name = NULL;
	}

	return orig_name;
}

/*
* Appends entry in a log file.
* const char *filename_log - name of the log file.
* const char *orig_name - string with the name of the original file.
* const size_t orig_size - size of the original file.
* const uint16_t crc - 2 byte crc of the original file.
* const size_t comp_size - size of the compressed file.
* const char *comp_name - string with the name of the comressed file.
* const bool compressed - true if this file was compressed, false in case it is decompressed.
* Returns 1 on error. 0 on success.
*/
int log_add_entry(const char *filename_log, const char *orig_name, const size_t orig_size, const uint16_t crc, const size_t comp_size, const char *comp_name, const bool compressed) {
	FILE *log_file;
	char *operation = compressed ? COMPRESSED_KEY : DECOMPRESSED_KEY;
	//NEW ENTRY
	log_file = fopen(filename_log, "a");
	fprintf(log_file, "%s : %s : %d : %d : %d : %s\n", operation, orig_name, orig_size, crc, comp_size, comp_name);
	fclose(log_file);
	return 0;
}

/*
* Clears out all '\n' and '\r' from the string.
* char **string - string to clear from newline characters.
*/
void clear_newlines(char **string) {
	if (*string == NULL) return; //String doesn't exist -> escape.

	size_t i = 0;
	size_t w = strlen(*string);
	for (i = 0; i < w; i++) {
		if (string[0][i] == '\n' || string[0][i] == '\r') {
			for (size_t q = i + 1; q <= w; q++) {
				string[0][q - 1] = string[0][q];
			}
			w--;
			i--;
		}
	}
}

/*
* Reads opened log file into the LOGGED_FILE structure array.
* FILE *log_file - opened log file for reading.
* LOGGED_FILE **entries - NULL pointer to the entries array. Every single logged file data will be returned into this dynamicly allocated array.
* Returns number of read entries. (lines)
*/
size_t read_log(FILE *log_file, LOGGED_FILE **entries) {
	size_t count = 0; //Count of lines (entries)
	char *line = malloc(MAX_LINESIZE + 1); //Line from log.
	char *line_ptr; //Pointer to certain char in line.
	size_t pos = 0; //Position on line
	if (*entries != NULL) {
		free(*entries);
		*entries = NULL;
	}
	*entries = malloc(sizeof(LOGGED_FILE));

	if (*entries == NULL || line == NULL) { //Memory error
		return count;
	}

	while (fgets(line, MAX_LINESIZE, log_file) != NULL && line != NULL) {
		LOGGED_FILE *temp_entry = NULL;
		uint8_t colon_count = 0;
		unsigned int crc;
		char *helping_line_ptr = line; //Since there are two filenames, which can have whitespaces in them - reading is pretty difficult.
		pos = 0;

		clear_newlines(&line);
		//Figuring out if the file was compressed or decompressed by this program. (enough to check for 'C')
		if(line[0] == 'C') entries[0][count].compressed = true;
		else entries[0][count].compressed = false;
		//Getting original name.
		while (colon_count < 1) {
			helping_line_ptr++;
			if (*helping_line_ptr == ':') colon_count++;
		}
		helping_line_ptr += 2; //Moving line on the start of original name.
		line_ptr = helping_line_ptr; //Getting line pointer to continue.
		while (colon_count < 2) {
			line_ptr++;
			pos++;
			if (*line_ptr == ':') colon_count++;
		}
		entries[0][count].orig_name = malloc(pos * sizeof(char));
		if (entries[0][count].orig_name == NULL) { //Memory error.
			free(line);
			line = NULL;
			return count;
		}
		for (size_t i = 0; i < pos - 1; i++) {
			entries[0][count].orig_name[i] = helping_line_ptr[i];
		}
		entries[0][count].orig_name[pos - 1] = '\0';
		pos += 2; //Saving position of the first number.

		//Getting compressed name.
		while (colon_count < 5) {
			line_ptr++;
			if (*line_ptr == ':') colon_count++;
		}
		line_ptr += 2; //Start of the compressed name.
		entries[0][count].comp_name = malloc((strlen(line_ptr) + 1) * sizeof(char));
		if (entries[0][count].comp_name == NULL) { //Memory error.
			free(line);
			line = NULL;
			return count;
		}
		strcpy(entries[0][count].comp_name, line_ptr);
		line_ptr -= 3; //Returning back before the ':'
		*line_ptr = '\0'; //Changing ' ' into '\0'
		//Getting numbers.
		line_ptr = &helping_line_ptr[pos];
		if (sscanf(line_ptr, "%d : %d : %d", &entries[0][count].orig_size, &crc, &entries[0][count].comp_size) != 3) {
			free(line);
			line = NULL;
			return count; //Incorrect formatting!? Someone messed up log file.
		}
		entries[0][count].crc = (uint16_t)crc;

		//Counting it as a full line.
		count++;
		//Reallocating memory for entries array.
		temp_entry = realloc(*entries, (count + 1) * sizeof(LOGGED_FILE));
		if (temp_entry == NULL) { //Reallocation failed. Memory error.
			free(line);
			line = NULL;
			return count;
		}
		*entries = temp_entry; //Getting pointer back.
	}

	free(line);
	line = NULL;

	return count;
}