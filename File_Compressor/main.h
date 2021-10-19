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
*
*/
static int handle_rle_pack();
/*
*
*/
static int handle_rle_unpack();
/*
*
*/
static int view_log();
/*
* Prints menu on the console screen.
*/
static void print_menu();

static int get_command(FILE **file);

static void command_get_filename(const char *command_string, char **filename);
