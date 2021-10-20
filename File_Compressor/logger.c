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
* Returns 1 on error. 0 on success.
*/
int file_out(FILE *file, const uint8_t *data, const size_t size) {
	if (file == NULL) return 1;

	for (size_t i = 0; i < size; i++) {
		fwrite(&data[i], 1, 1, file);
	}

    return 0;
}

/*
* Gets new name for the file. Cuts the end of the name in case of reaching the limit. (Shouldn't happen, at least not on a daily basis)
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
	string[0][pos + 1] = 'b';
	string[0][pos + 2] = 'm';
	string[0][pos + 3] = 'p';
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