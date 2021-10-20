#pragma warning(disable:4996) //disabling warning

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
int file_out(FILE *file, const uint8_t *data, const size_t size);

/*
* Gets new name for the file.
*/
int get_new_name(char **string);

/*
* Checks log entries for given filename.
*/
bool log_check_entry(const char *filename);

/*
* Retrieves original name of the compressed file from the log.
*/
int log_retrieve_name(char **string);