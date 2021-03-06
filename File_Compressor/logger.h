#pragma warning(disable:4996) //disabling warning

typedef struct logged_file {
	bool compressed;
	char *orig_name;
	size_t orig_size;
	uint16_t crc;
	size_t comp_size;
	char *comp_name;
} LOGGED_FILE;

#define MAX_LINESIZE ((MAX_FILENAME * 2 * sizeof(char)) + sizeof(uint16_t) + (2 * sizeof(size_t)))

/*
* Prints log to the user.
* const char *filename_log - name of the log file.
* Returns 1 on error. 0 on success.
*/
int view_log(const char *filename_log);

/*
* Reads all file contents into data array.
* FILE *file - opened file in binary format to read from.
* uint8_t **data - ponter to the data array to read to.
* size_t *size - pointer to the size of the data array. (size will be returned to that)
* Returns: '2' in case of memory mistake. '1' in case if no file was provided. '0' in case of success.
*/
int file_in(FILE *file, uint8_t **data, size_t *size);

/*
* Writes all data contents into the file.
* FILE *file - opened file in binary format to write data to.
* const uint8_t *data - array with data to write.
* const size_t size - size of the data array.
* const uint16_t crc - checksum to pu as a header. If specified as 0 nothing will be added before the data.
* Returns 1 on error. 0 on success.
*/
int file_out(FILE *file, const uint8_t *data, const size_t size, const uint16_t crc);

/*
* Gets new name for the file. Cuts the end of the name in case of reaching the limit. (Shouldn't happen, at least not on a daily basis)
* char **string - pointer to the string with the old name.
* Returns 1 on error. 0 on success.
*/
int get_new_name(char **string);

/*
* Checks log entries for given filename and checksum.
* const char *filename_log - name of the log file.
* const char *comp_filename - string with the name of the comressed file.
* const uint16_t crc - Checksum must match in order to recognize file.
* Returns 'true' if entry exists, 'false' if no such entry found.
*/
bool log_check_entry(const char *filename_log, const char *comp_filename, const uint16_t crc);

/*
* Retrieves last original name of the compressed file with the same crc from the log.
* const char *filename_log - name of the log file.
* const char *comp_filename - string with the name of the comressed file.
* const uint16_t crc - Checksum must match in order to recognize file.
* Returns NULL pointer on failure or string with the name of the original file.
*/
char *log_retrieve_name(const char *filename_log, const char *comp_name, const uint16_t crc);

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
int log_add_entry(const char *filename_log, const char *orig_name, const size_t orig_size, const uint16_t crc, const size_t comp_size, const char *comp_name, const bool compressed);

/*
* Clears out all '\n' and '\r' from the string.
* char **string - string to clear from newline characters.
*/
void clear_newlines(char **string);

/*
* Reads opened log file into the LOGGED_FILE structure array.
* FILE *log_file - opened log file for reading.
* LOGGED_FILE **entries - NULL pointer to the entries array. Every single logged file data will be returned into this dynamicly allocated array.
* Returns number of read entries. (lines)
*/
size_t read_log(FILE *log_file, LOGGED_FILE **entries);