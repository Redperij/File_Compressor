#pragma warning(disable:4996) //disabling warning

#include "main.h"
#include "logger.h"

/*
* Prints log to the user.
* const char *filename_log - name of the log file.
*/
int view_log(const char *filename_log) {
	FILE *log_file = fopen(filename_log, "r");
	LOGGED_FILE *entries = NULL;
	size_t size;

	if (log_file == NULL) {
		printf("Log is empty. No files are encrypted.\n");
		return 1;
	}

	size = read_log(log_file, &entries);
	//TODO: Show the log file. (not demanded, but why not?)


	for (size_t i = 0; i < size; i++) {
#pragma warning (disable:6001) //Nope, they are initialized. size won't increase if they are not.
			free(entries[i].comp_name);
			free(entries[i].orig_name);
#pragma warning (default:6001)
		}
	free(entries);
	entries = NULL;

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
	temp = crc;
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
* Checks log entries for given filename.
* const char *filename_log - name of the log file.
* const char *comp_filename - string with the name of the comressed file.
* True - entry exists, false - no such entry found.
*/
bool log_check_entry(const char *filename_log, const char *comp_filename) {
	FILE *log_file = fopen(filename_log, "r");
	if (log_file == NULL) { //File must exist
		return false;
	}
	else {
		uint8_t colon_count = 0; //Counts colons met.
		char *line = malloc(MAX_LINESIZE); //line from log file.
		char *line_ptr; //Pointer to the place in line.

		if (line != NULL) {
			while (!feof(log_file) && !ferror(log_file)) {
				colon_count = 0;
				if (fgets(line, MAX_LINESIZE, log_file) != NULL) {
					line_ptr = line - 1; //Line pointer to the start. (one element back because of moving it forward in the start of the loop)
					while (colon_count < 4) { //Stop on the fourth colon.
						line_ptr++;
						if (*line_ptr == ':') {
							colon_count++;
						}
					}
					line_ptr += 2; //Moving pointer 2 elements forward on the start of the compressed file name.
					clear_newlines(&line_ptr);
					if (!strcmp(line_ptr, comp_filename)) {
						free(line);
						line = NULL;

						rewind(log_file);
						fclose(log_file);
						return true;
					}
				}
			}
		}
		free(line);
		line = NULL;
	}
	rewind(log_file);
	fclose(log_file);
    return false;
}

/*
* Retrieves original name of the compressed file from the log.
* const char *filename_log - name of the log file.
* const char *comp_filename - string with the name of the comressed file.
* Returns NULL pointer on failure or string with the name of the original file.
*/
char *log_retrieve_name(const char *filename_log, const char *comp_name) {
	FILE *log_file = fopen(filename_log, "r");
	LOGGED_FILE *entries = NULL;
	size_t size;
	char *orig_name = NULL;
	if (log_file == NULL) {
		return NULL;
	}

	size = read_log(log_file, &entries);
	fclose(log_file);

	for (size_t i = 0; i < size; i++) {
		if (!strcmp(entries[i].comp_name, comp_name)) {
			orig_name = entries[i].orig_name;
			free(entries[i].comp_name);
		}
		else {
			free(entries[i].comp_name);
			free(entries[i].orig_name);
		}
	}

	free(entries);
	entries = NULL;
	return orig_name;
}

/*
* Appends entry in a log file.
* Rewrites entry if the same name is encountered.
* const char *filename_log - name of the log file.
* const char *orig_name - string with the name of the original file.
* const size_t orig_size - size of the original file.
* const uint16_t crc - 2 byte crc of the original file.
* const size_t comp_size - size of the compressed file.
* const char *comp_name - string with the name of the comressed file.
* Returns 1 on error. 0 on success.
*/
int log_add_entry(const char *filename_log, const char *orig_name, const size_t orig_size, const uint16_t crc, const size_t comp_size, const char *comp_name) {
	FILE *log_file;
	if (log_check_entry(filename_log, comp_name)) {
		LOGGED_FILE *entries = NULL;
		size_t count = 0;
		//SAME ENTRY
		//It is possible that compressed file was rewritten with contents of different file. (e.g. "a.txt" and "a.png" both will produce "a.bmp")
		log_file = fopen(filename_log, "r");
		count = read_log(log_file, &entries);
		if (count == 0) {
			printf("Error occured while reading log file.\nPossible causes:\n1. Incorrect log formatting. It is not expected to be modified manually.\n2. Memory error.\n\n");
			return 1;
		}
		else {
			size_t i = 0;
			char *temp_string;
			//Copy original name.
			for (i = 0; strcmp(entries[i].comp_name, comp_name); i++);
			temp_string = realloc(entries[i].orig_name, (strlen(orig_name) + 1) * sizeof(char));
			if (temp_string == NULL) { //Realloc failed. Memory error.
				return 1;
			}
			entries[i].orig_name = temp_string; //Getting pointer back.

			//Rewriting entry.
			strcpy(entries[i].orig_name, orig_name);
			entries[i].orig_size = orig_size;
			entries[i].crc = crc;
			entries[i].comp_size = comp_size;
		}
		fclose(log_file);

		log_file = fopen(filename_log, "w");
		//Filling file with entries from scratch.
		for (size_t i = 0; i < count; i++) {
			fprintf(log_file, "%s : %d : %d : %d : %s\n", entries[i].orig_name, entries[i].orig_size, entries[i].crc, entries[i].comp_size, entries[i].comp_name);
		}
		fclose(log_file);

		//Free memory allocated for entries.
		for (size_t i = 0; i < count; i++) {
#pragma warning (disable:6001) //Nope, they are initialized. count won't increase if they are not.
			free(entries[i].comp_name);
			free(entries[i].orig_name);
#pragma warning (default:6001)
		}
		free(entries);
		entries = NULL;
	}
	else {
		//NEW ENTRY
		log_file = fopen(filename_log, "a");
		fprintf(log_file, "%s : %d : %d : %d : %s\n", orig_name, orig_size, crc, comp_size, comp_name);
		fclose(log_file);
	}
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
		line_ptr = line;
		pos = 0;
		unsigned int crc;

		clear_newlines(&line);
		//Getting original name.
		while (*line_ptr != ':') {
			line_ptr++;
			pos++;
		}
		colon_count++;
		entries[0][count].orig_name = malloc(pos * sizeof(char));
		if (entries[0][count].orig_name == NULL) { //Memory error.
			free(line);
			line = NULL;
			return count;
		}
		for (size_t i = 0; i < pos - 1; i++) {
			entries[0][count].orig_name[i] = line[i];
		}
		entries[0][count].orig_name[pos - 1] = '\0';
		pos += 2; //Saving position of the first number.

		//Getting compressed name.
		while (colon_count < 4) {
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
		line_ptr = &line[pos];
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