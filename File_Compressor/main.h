#pragma warning(disable:4996) //disabling warning

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG 1
#define MAX_FILENAME 256
#define MAX_COMMAND 266

#ifdef DEBUG
#define DEBUGPRINT(x) printf(x)
#else
#define DEBUGPRINT(x)
#endif // DEBUG

#define LOG_FILENAME "rle_compressed.log"

/*
* Handles file compression sequence.
*/
static int handle_rle_pack(const char *filename_to_pack);

/*
* Handles file decompression sequence.
*/
static int handle_rle_unpack(const char *filename_to_unpack);

/*
* Prints menu on the console screen.
*/
static void print_menu();

/*
* Gets correct command from the user.
* Returns number of command and a filename of the existing file, if received any.
*/
static int get_command(char **filename);

/*
* Gets filename from command string.
*/
static void command_get_filename(const char *command_string, char **filename);
