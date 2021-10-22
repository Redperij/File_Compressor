#pragma warning(disable:4996) //disabling warning

typedef struct logged_file {
	char *orig_name;
	size_t orig_size;
	uint16_t crc;
	size_t comp_size;
	char *comp_name;
} LOGGED_FILE;

#define MAX_LINESIZE ((MAX_FILENAME * 2 * sizeof(char)) + sizeof(uint16_t) + (2 * sizeof(size_t)))

/*
* Prints log to the user.
*/
int view_log(const char *filename_log);

/*
* Reads all file contents into data array.
*/
int file_in(FILE *file, uint8_t **data, size_t *size);

/*
* Writes all data contents into the file.
*/
int file_out(FILE *file, const uint8_t *data, const size_t size, const uint16_t crc);

/*
* Gets new name for the file.
*/
int get_new_name(char **string);

/*
* Checks log entries for given filename.
* True - entry exists, false - no such entry found.
*/
bool log_check_entry(const char *filename_log, const char *comp_filename);

/*
* Retrieves original name of the compressed file from the log.
*/
int log_retrieve_name(const char *filename_log, char **string);

/*
* Appends entry in a log file.
* Rewrites entry if the same name is encountered.
*/
int log_add_entry(const char *filename_log, const char *orig_name, const size_t orig_size, const uint16_t crc, const size_t comp_size, const char *comp_name);

/*
* Clears out all '\n' and '\r' from the string.
*/
void clear_newlines(char **string);

/*
* Reads opened log file into the LOGGED_FILE structure array.
* Returns number of read entries. (lines)
*/
size_t read_log(FILE *log_file, LOGGED_FILE **entries);