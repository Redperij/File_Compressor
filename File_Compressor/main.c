#pragma warning(disable:4996) //disabling warning

#include "main.h"
#include "RLE.h"
#include "logger.h"

int main() {
	int command = 0;
	char *filename = NULL;

	print_menu();
	while (command != 5) {
		printf("Input: ");
		command = get_command(&filename);
		switch (command)
		{
		case 1: //rle pack
			handle_rle_pack(filename);
			free(filename);
			filename = NULL;
			break;
		case 2: //rle unpack
			handle_rle_unpack(filename);
			free(filename);
			filename = NULL;
			break;
		case 3: //view log
			view_log(LOG_FILENAME);
			break;
		case 4: //print menu
			print_menu();
			break;
		case 5: //exit
			printf("Exitting program.\n");
			break;
		default:
			//NO ERRORS, ONLY MEMES
			break;
		}

	}

	return 0;
}

/*
* Handles file compression sequence.
* const char *filename_to_pack - name of the file to pack.
* Returns 1 in case of an error. 0 on success.
*/
static int handle_rle_pack(const char *filename_to_pack) {
	FILE *file_to_pack = fopen(filename_to_pack, "rb");
	uint8_t *fbytes = NULL; //Array of bytes from the file.
	size_t size = 0; //Size of an array of bytes.
	size_t orig_size = 0; //Size of the original data array.
	char *new_filename = NULL; //New name for file.
	int error = 0;
	uint16_t crc; //crc of the original data.

	new_filename = malloc((strlen(filename_to_pack) + 1) * sizeof(char));

	if (new_filename == NULL) {
		return 1;
	}

	strcpy(new_filename, filename_to_pack); //Getting old name first.

	error = file_in(file_to_pack, &fbytes, &size); //Getting file contents.
	fclose(file_to_pack); //Closing file.
	if (!error) {

#if DEBUG
		printf("Data array:\n");
		for (unsigned int i = 0; i < size; i++) {
			printf("%2x ", fbytes[i]);
		}
		printf("\n\n");
#endif

		crc = crc16(fbytes, size); //Counting crc.

#if DEBUG
		printf("CRC = %2x\n", crc);
#endif

		orig_size = size;
		rle_pack(&fbytes, &size); //Compressing file contents.

#if DEBUG
		printf("New array:\n");
		for (unsigned int i = 0; i < size; i++) {
			printf("%2x ", fbytes[i]);
		}
		printf("\n\n");
#endif

		get_new_name(&new_filename); //Getting new file name (One with .bmp extension).

		file_to_pack = fopen(new_filename, "wb");
		file_out(file_to_pack, fbytes, size, crc);
		fclose(file_to_pack);
		
		free(fbytes);
		fbytes = NULL;

		log_add_entry(LOG_FILENAME, filename_to_pack, orig_size, crc, size + 2, new_filename, true);
		printf("Successfully packed \"%s\" (%d b) file into \"%s\" (%d b)\n\n", filename_to_pack, orig_size, new_filename, size + 2);
	}

	free(new_filename);
	new_filename = NULL;
	
	return 0;
}

/*
* Handles file decompression sequence.
* const char *filename_to_unpack - name of the file to unpack. (must be logged, if it is not - nothing will be done)
* Returns 1 in case of an error. 0 on success.
*/
static int handle_rle_unpack(const char *filename_to_unpack) {
	FILE *file_to_unpack = fopen(filename_to_unpack, "rb");
	if (file_to_unpack == NULL) {
		return 1;
	}
	FILE *created_file;
	uint8_t *fbytes = NULL; //Array of bytes from the file.
	size_t size = 0; //Size of an array of bytes.
	size_t comp_size = 0; //Used to save compressed file size before decompressing file.
	uint16_t crc = 0; //Checksum of the original file.
	char *new_filename = NULL; //New name for file.
	int check = 0;

	//Don't give to unpack any file

	check = file_in(file_to_unpack, &fbytes, &size);
	fclose(file_to_unpack);
	comp_size = size;

#if DEBUG
	printf("Data array:\n");
	for (unsigned int i = 0; i < size; i++) {
		printf("%2x ", fbytes[i]);
	}
	printf("\n\n");
#endif

	if (!check) {
		crc = rle_unpack(&fbytes, &size);

		if (fbytes == NULL) { //Must receive at least something to work with.
			printf("WARNING!\nFile \"%s\" (%d b) cannot be unpacked.\n", filename_to_unpack, size);
			return 1;
		}

		#if DEBUG
			printf("New array:\n");
			for (unsigned int i = 0; i < size; i++) {
				printf("%2x ", fbytes[i]);
			}
			printf("\n\n");

			printf("CRC from file: %2x\nCounted CRC: %2x\n", crc, crc16(fbytes, size));
		#endif
		
		if (crc != crc16(fbytes, size)) {
			printf("WARNING! Incorrect checksum.\nFile \"%s\" (%d b) cannot be unpacked.\n", filename_to_unpack, comp_size);
			free(fbytes);
			fbytes = NULL;
			return 1;
		}

		#if DEBUG
			else {
				printf("Checksum is correct.\n");
			}
		#endif

		//There is entry in the log. Retrieve original name with extension.
		if (log_check_entry(LOG_FILENAME, filename_to_unpack, crc)) {
			char *original_name = NULL;
			//Retrieve name from log.
			original_name = log_retrieve_name(LOG_FILENAME, filename_to_unpack, crc);
			if (original_name == NULL) {
				return 1;
			}
			//Write contents into the file.
			created_file = fopen(original_name, "wb");
			if (created_file != NULL) {
				file_out(created_file, fbytes, size, 0);
				fclose(created_file);
				//Log.
				log_add_entry(LOG_FILENAME, original_name, size, crc, comp_size, filename_to_unpack, false);
				printf("Successfully unpacked \"%s\" (%d b) file into \"%s\" (%d b)\n\n", filename_to_unpack, comp_size, original_name, size);
			}
			free(original_name);
			original_name = NULL;
		}
		//No log entry found. Just remove extension.
		else {
			char *original_name = malloc(MAX_FILENAME * sizeof(char));
			if (original_name == NULL) {
				return 1;
			}
			printf("No log entry. File without extension will be created.\n");
			strcpy(original_name, filename_to_unpack); //Copy current name.
			original_name[strlen(original_name) - 4] = '\0'; //Terminate string before the extension.

			//Write contents into the file.
			created_file = fopen(original_name, "wb");
			if (created_file != NULL) {
				file_out(created_file, fbytes, size, 0);
				fclose(created_file);
				//Log.
				log_add_entry(LOG_FILENAME, original_name, size, crc, comp_size, filename_to_unpack, false);
				printf("Successfully unpacked \"%s\" file into \"%s\"\n\n", filename_to_unpack, original_name);
			}
			free(original_name);
			original_name = NULL;
		}

		free(fbytes);
		fbytes = NULL;
	}

	return 0;
}

/*
* Prints menu on the console screen.
*/
static void print_menu() {
	printf("\
 ---------------------------------------------------------------------------------\n\
|List of available commands:                                                      |\n\
|rle pack [filename] (compresses the specified file)                              |\n\
|rle unpack [filename] (compresses the specified file)                            |\n\
|rle log (view log)                                                               |\n\
|? (display menu)                                                                 |\n\
|q (Exit the program. Also available: Q / Quit /quit / QUIT / Exit / exit / EXIT )|\n\
 ---------------------------------------------------------------------------------\n");
}

/*
* Gets correct command from the user.
* char **filename - ponter to the string with the filename. Name of the file received by the command will be returned here.
* Returns number of command and a filename of the existing file, if received any.
*/
static int get_command(char **filename) {
	FILE *file;
	char *command = (char *)malloc(MAX_COMMAND * sizeof(char)); //Command string.
	if (command == NULL) {
		return 0;
	}
	char *command_ptr = NULL; //Pointer to the command string.
	if (*filename != NULL) {
		free(*filename);
		*filename = NULL; //Nulling out filename.
	}

	while (1) {
		command_ptr = fgets(command, MAX_COMMAND, stdin); //Getting command string.
		if (command_ptr != NULL) {
			char *temp = (char *)malloc(MAX_COMMAND * sizeof(char));
			if (temp == NULL) {
				return 0;
			}
			//Looking for reasanable command.
			if (sscanf(command_ptr, "%s", temp) == 1) {
				// rle
				if (!strcmp(temp, "rle")) {
					//Must be followed by other command
					if (sscanf(command_ptr, "%*s %s", temp) == 1) {
						// pack [filename]
						if (!strcmp(temp, "pack")) {
							command_get_filename(command, filename); //Getting filename from command.
							if (*filename != NULL) {
								//Try to open the file.
								file = fopen(*filename, "rb");
								//File found -> can continue.
								if (file != NULL) {
									rewind(file);
									fclose(file);
									file = NULL;

									free(temp);
									temp = NULL;
									free(command);
									command = NULL;
									return 1;
								}
								//File wasn't found.
								else {
									printf("Unable to open the \"%s\" file. It must be in the same directory with the .c file.\n\nInput: ", *filename);
								}
								//Nulling filename.
								free(*filename);
								*filename = NULL;
							}
							//No filename provided.
							else {
								printf("No filename provided.\n\nInput: ");
							}
						}
						// unpack [filename]
						else if (!strcmp(temp, "unpack")) {
							command_get_filename(command, filename); //Getting filename from command.
							if (*filename != NULL) {
								//Try to open the file.
								file = fopen(*filename, "rb");
								//File found -> can continue.
								if (file != NULL) {
									rewind(file);
									fclose(file);
									file = NULL;

									free(temp);
									temp = NULL;
									free(command);
									command = NULL;
									return 2;
								}
								//File wasn't found.
								else {
									printf("Unable to open the \"%s\" file. It must be in the same directory with the .c file.\n\nInput: ", *filename);
								}
								//Nulling filename.
								free(*filename);
								*filename = NULL;
							}
							//No filename provided.
							else {
								printf("No filename provided.\n\nInput: ");
							}
						}
						// log
						else if (!strcmp(temp, "log")) {
							free(temp);
							temp = NULL;
							free(command);
							command = NULL;
							return 3;
						}
						else {
							printf("Unrecognized command: \"%s\"\n\nPrint menu with commands using ?\n\nInput: ", temp);
						}
					}
					// rle - nothing followed.
					else {
						printf("Incorrect number of arguments.\n\nPrint menu with commands using ?\n\nInput: ");
					}
				}
				// ?
				else if (!strcmp(temp, "?")) {
					free(temp);
					temp = NULL;
					free(command);
					command = NULL;
					return 4;
				}
				// q / Q / Quit / quit / QUIT / Exit / exit / EXIT
				else if (!strcmp(temp, "q") || !strcmp(temp, "Q")
					|| !strcmp(temp, "Quit") || !strcmp(temp, "quit") || !strcmp(temp, "QUIT")
					|| !strcmp(temp, "Exit") || !strcmp(temp, "exit") || !strcmp(temp, "EXIT")) {

					free(temp);
					temp = NULL;
					free(command);
					command = NULL;
					return 5;
				}
				else {
					printf("Unrecognized command: \"%s\"\n\nPrint menu with commands using ?\n\nInput: ", temp);
				}
			}
			free(temp);
			temp = NULL;
		}
	}
}

/*
* Gets filename from command string.
* const char *command_string - string with command. (e.g "rle pack a.txt")
* char **filename - pointer to the name of the file, which will be retuned.
*/
static void command_get_filename(const char *command_string, char **filename) {
	int length = 0;
	char *filename_ptr = NULL;

	if (*filename != NULL) {
		free(*filename);
		*filename = NULL;
	}
	*filename = (char *)malloc(MAX_FILENAME * sizeof(char));
	if (*filename == NULL) {
		return;
	}

	//finding 'pack' part
	filename_ptr = strstr(command_string, "pack");

	//Suspending several warnings.
	if (filename_ptr == NULL) {
		return;
	}

	//Moving away from the word.
	filename_ptr += 4;
	//Move until meet another word -> this is the start of the filename.
	while (isspace(*filename_ptr)) filename_ptr++;
	//Getting length. We may have nothing in the string.
	length = strlen(filename_ptr);
	
	//Moving filename to the *filename string.
	for (int i = 0; i < length; i++) {
		filename[0][i] = filename_ptr[i];
	}
	//Terminating string ion the end or on '\n'
#pragma warning (disable:6001) //It is initialized.
	if (filename[0][length - 1] == '\n') filename[0][length - 1] = '\0';
	else filename[0][length] = '\0';
#pragma warning (default:6001)

	//If have nothing -> Null the pinter not to be confused.
	if (length == 0) {
		*filename = NULL;
	}
}