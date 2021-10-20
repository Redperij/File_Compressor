#pragma warning(disable:4996) //disabling warning

#include "main.h"
#include "logger.h"

/*
* Prints log to the user.
*/
int view_log(const char *filename_log) {
	FILE *log_file = fopen(filename_log, "r");
	if (log_file == NULL) {
		printf("Log is empty. No files are encrypted.\n");
		return 1;
	}
	
	//Show the log file. (not demanded, but why not?)

	fclose(log_file);
	return 0;
}

/*
* Reads all file contents into data array.
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
*/
int file_out(FILE *file, const uint8_t *data, const size_t size) {

    return 0;
}

/*
* Gets new name for the file.
*/
int get_new_name(char **string) {

    return 0;
}

/*
* Checks log entries for given filename.
*/
bool log_check_entry(const char *filename) {
    
    return false;
}

/*
* Retrieves original name of the compressed file from the log.
*/
int log_retrieve_name(char **string) {

	return 0;
}