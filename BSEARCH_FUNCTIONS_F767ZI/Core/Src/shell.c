/*
 * shell.c
 *
 *  Created on: Oct 11, 2021
 *      Author: root
 */

#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "shell.h"
#include "usart.h"

typedef uint8_t (*shellFunc)(int argc, char **argv);

#define SHELL_CMD_NAME_SIZE 32
#define SHELL_CMD_DESC_SIZE 128
typedef struct shellCommand_t {
	char name[SHELL_CMD_NAME_SIZE];
	char desc[SHELL_CMD_DESC_SIZE];
	shellFunc funcptr;
	uint8_t cmdTakesParam1;
	uint8_t cmdTakesParam2;
	uint8_t param2IsNum;
} shellCommand;

#define CMD_NUM_CMDS 11
#define CLI_CAT		"CAT"
#define CLI_CKSUM	"CKSUM"
#define CLI_CRC32 	"CRC32"
#define CLI_FORMAT	"FORMAT"
#define CLI_HELP	"HELP"
#define CLI_HEXDUMP "HEXDUMP"
#define CLI_LS		"LS"
#define CLI_MV		"MV"
#define CLI_RM		"RM"
#define CLI_TOUCH	"TOUCH"
#define CLI_VI		"VI"

static uint8_t shellCAT(int argc, char **argv);
static uint8_t shellCKSUM(int argc, char **argv);
static uint8_t shellCRC32(int argc, char **argv);
static uint8_t shellFORMAT(int argc, char **argv);
static uint8_t shellHELP(int argc, char **argv);
static uint8_t shellHEXDUMP(int argc, char **argv);
static uint8_t shellLS(int argc, char **argv);
static uint8_t shellMV(int argc, char **argv);
static uint8_t shellRM(int argc, char **argv);
static uint8_t shellTOUCH(int argc, char **argv);
static uint8_t shellVI(int argc, char **argv);

// Must be sorted in ascending order.
const shellCommand commands[] = { 	//
				{ CLI_CAT, "Reads out the entire file at once", shellCAT, 1, 0,
						0 },	//

				{ CLI_CKSUM, "Calculates cksum value of specified file",
						shellCKSUM, 1, 0, 0 },	//

				{ CLI_CRC32, "Calculates CRC32 value of specified file",
						shellCRC32, 1, 0, 0 },	//

				{ CLI_FORMAT, "Erases the entire flash storage device",
						shellFORMAT, 0, 0, 0 },	//

				{ CLI_HELP, "Lists all available functions", shellHELP, 0, 0, 0 },//

				{ CLI_HEXDUMP,
						"Prints out the hexdump of file, optional byte limit",
						shellHEXDUMP, 1, 1, 1 },	//

				{ CLI_LS, "lists all available files", shellLS, 0, 0, 0 },	//

				{ CLI_MV, "Renames file", shellMV, 1, 1, 0 },	//

				{ CLI_RM, "Removes the file", shellRM, 1, 0, 0 },	//

				{ CLI_TOUCH, "Creates a new empty file", shellTOUCH, 1, 0, 0 },	//

				{ CLI_VI, "Write data to file", shellVI, 1, 1, 1 },	//

		};

static uint8_t shellCAT(int argc, char **argv) {
	return 1;
}
static uint8_t shellCKSUM(int argc, char **argv) {
	return 1;
}
static uint8_t shellCRC32(int argc, char **argv) {
	return 1;
}
static uint8_t shellFORMAT(int argc, char **argv) {
	return 1;
}
static uint8_t shellHELP(int argc, char **argv) {
	return 1;
}
static uint8_t shellHEXDUMP(int argc, char **argv) {
	return 1;
}
static uint8_t shellLS(int argc, char **argv) {
	return 1;
}
static uint8_t shellMV(int argc, char **argv) {
	return 1;
}
static uint8_t shellRM(int argc, char **argv) {
	return 1;
}
static uint8_t shellTOUCH(int argc, char **argv) {
	return 1;
}
static uint8_t shellVI(int argc, char **argv) {
	return 1;
}

uint8_t shellProcess(uartRxStruct input) {

	int argc = 0;
	char *argv[UART_RX_NUM_WORDS];
	int ret = 1;
	shellCommand *command;

	//clear out commands
	memset(argv, 0, sizeof(argv));

	//assign commands to argv
	for (int i = 0; i < UART_RX_NUM_WORDS; i++) {
		argv[i] = (char*) input.rxWords[i];
	}

	command = (shellCommand*) bsearch(input.rxWords[0], commands,
			sizeof(commands) / sizeof(struct shellCommand_t),
			sizeof(struct shellCommand_t),
			(int (*)(const void*, const void*)) strcasecmp);

	ret = (command->funcptr)(argc, argv);

	return ret;

}
