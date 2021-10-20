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
			DEBUGPRINT("Case 1 execution\n");
			handle_rle_pack(filename);
			free(filename);
			filename = NULL;
			break;
		case 2: //rle unpack
			DEBUGPRINT("Case 2 execution\n");
			handle_rle_unpack(filename);
			free(filename);
			filename = NULL;
			break;
		case 3: //view log
			DEBUGPRINT("Case 3 execution\n");
			view_log(LOG_FILENAME);
			break;
		case 4: //print menu
			DEBUGPRINT("Case 4 execution\n");
			print_menu();
			break;
		case 5: //exit
			DEBUGPRINT("Case 5 execution\n");
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
* Returns 1 in case of an error. 0 on success.
*/
static int handle_rle_pack(const char *filename_to_pack) {
	FILE *file_to_pack = fopen(filename_to_pack, "rb");
	uint8_t *fbytes = NULL; //Array of bytes from the file.
	size_t size = 0; //Size of an array of bytes.
	char *new_filename = NULL; //New name for file.
	int check = 0;

	new_filename = malloc((strlen(filename_to_pack) + 1) * sizeof(char));

	if (new_filename == NULL) {
		return 1;
	}

	strcpy(new_filename, filename_to_pack); //Getting old name first.

	check = file_in(file_to_pack, &fbytes, &size); //Getting file contents.
	fclose(file_to_pack); //Closing file.
	if (!check) {
#if DEBUG
		printf("Data array:\n");
		for (unsigned int i = 0; i < size; i++) {
			printf("%c", fbytes[i]);
		}
		printf("\n\n");
#endif
		rle_pack(&fbytes, &size); //Compressing file contents.
#if DEBUG
		printf("New array:\n");
		for (unsigned int i = 0; i < size; i++) {
			printf("%2x ", fbytes[i]);
		}
		printf("\n\n");
#endif
		get_new_name(&new_filename); //Getting new file name (One with .bmp extension).
#if DEBUG
		printf("Creating %s file\n", new_filename);
#endif

		file_to_pack = fopen(new_filename, "wb");
		file_out(file_to_pack, fbytes, size);
		fclose(file_to_pack);
		
		//log_add_entry(); //filename_to_pack - .* + .bmp
	}
	
	
	return 0;
}

/*
* Handles file decompression sequence.
*/
static int handle_rle_unpack(const char *filename_to_unpack) {
	FILE *file_to_unpack = fopen(filename_to_unpack, "rb");
	uint8_t *fbytes = NULL; //Array of bytes from the file.
	size_t size = 0; //Size of an array of bytes.
	char *new_filename = NULL; //New name for file.
	bool entry = false;
	int check = 0;

	//entry = log_check_entry();
	check = file_in(filename_to_unpack, &fbytes, &size);
	if (!check) {
		//rle_unpack();
		//log_retrieve_name();
		//file_out();

		//log_remove_entry();
	}
	

	fclose(file_to_unpack);

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
	if (filename[0][length - 1] == '\n') filename[0][length - 1] = '\0';
	else filename[0][length] = '\0';

	//If have nothing -> Null the pinter not to be confused.
	if (length == 0) {
		*filename = NULL;
	}
}