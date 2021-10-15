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
#include "lfs_interface.h"
#include "crc.h"

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
		{ CLI_CAT, "Reads out the entire file at once",					//
				shellCAT, 1, 0, 0 },									//

				{ CLI_CKSUM, "Calculates cksum value of specified file",	//
						shellCKSUM, 1, 0, 0 },								//

				{ CLI_CRC32, "Calculates CRC32 value of specified file",	//
						shellCRC32, 1, 0, 0 },								//

				{ CLI_FORMAT, "Erases the entire flash storage device",	//
						shellFORMAT, 0, 0, 0 },							//

				{ CLI_HELP, "Lists all available functions",			//
						shellHELP, 0, 0, 0 },							//

				{ CLI_HEXDUMP,
						"Prints out the hexdump of file, optional byte limit",//
						shellHEXDUMP, 1, 1, 1 },						//

				{ CLI_LS, "lists all available files",					//
						shellLS, 0, 0, 0 },								//

				{ CLI_MV, "Renames file",								//
						shellMV, 1, 1, 0 },								//

				{ CLI_RM, "Removes the file",							//
						shellRM, 1, 0, 0 },								//

				{ CLI_TOUCH, "Creates a new empty file",				//
						shellTOUCH, 1, 0, 0 },							//

				{ CLI_VI, "Write data to file",							//
						shellVI, 1, 1, 1 },								//

		};

static void uartRxFile(char *name, uint32_t size) {
	// temporarily disable UART interrupts to receive full file
	HAL_NVIC_DisableIRQ(USART3_IRQn);

	uint8_t recv[size];

	// unable to rx file in interrupt/dma mode, so using blocking function as of now
	HAL_UART_Receive(&huart3, recv, size, HAL_MAX_DELAY);
	printf("\nRecvd %lu bytes\n", size);
	int err = lfsWriteFile(recv, size);
	printf("Wrote %d of %lu bytes \n\n", err, size);

	//re-enable UART interrupts
	HAL_NVIC_EnableIRQ(USART3_IRQn);
}

static void shellPrintError(char *func, char *desc) {
	printf("[%s] Please provide a %s\n\n", func, desc);
}

static void shellPrintFileRead(char *name, char *contents, int32_t size) {
	if (!size) {
		return;
	}

	printf("%s[%ld]:\n", name, size);
	for (int32_t i = 1; i <= size; i++) {
		printf("%c", contents[i - 1]);
	}

	printf("\n\n");
}

static void shellPrintHexdump(char *name, uint8_t *contents, int32_t size) {
	if (!size) {
		return;
	}
	printf("%s[%ld]:\n", name, size);

	for (int32_t i = 1; i <= size; i++) {

		if (((i - 1) % 16) == 0) {
			printf("\t0x%08lX    ", i - 1);
		}
		printf("%02X%c", contents[i - 1], (i % 16 == 0) ? '\n' : ' ');
	}

	printf("\n\n");
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

	command = (shellCommand*) bsearch(argv[0], commands,
			sizeof(commands) / sizeof(struct shellCommand_t),
			sizeof(struct shellCommand_t),
			(int (*)(const void*, const void*)) strcasecmp);

	if (command != 0) {
		ret = (command->funcptr)(argc, argv);
	} else {
		ret = 0;
		shellPrintError("SHELL", "valid command");
	}

	return ret;

}

/**
 * Shell commands
 */
static uint8_t shellCAT(int argc, char **argv) {

	UNUSED(argc);
	UNUSED(argv);

	if (argv[1][0] == 0) {
		shellPrintError(CLI_CAT, "file name");
		return 0;
	}

	uint32_t size = lfsGetFileSize(argv[1]);
	char contents[size];
	memset(contents, 0, size);
	int32_t err = lfsReadFile(argv[1], (uint8_t*) contents);

	if (err < 0) {
		printf("\n");
		return 0;
	}

	shellPrintFileRead(argv[1], contents, size);

	return 1;
}

static uint8_t shellCKSUM(int argc, char **argv) {

	UNUSED(argc);
	UNUSED(argv);

	if (argv[1][0] == 0) {
		shellPrintError(CLI_CKSUM, "file name");
		return 0;
	}

	uint32_t fileSize = lfsGetFileSize(argv[1]);
	uint8_t data[fileSize];
	memset(data, 0, fileSize);
	int err = lfsReadFile(argv[1], data);
	if (err < 1) {
		printf("\n");
		return 0;
	}

	uint32_t crc = cksumCalcCksum(data, fileSize);
	printf("%lu %ld %s\n\n", crc, fileSize, argv[1]);

	return 1;
}

static uint8_t shellCRC32(int argc, char **argv) {

	UNUSED(argc);
	UNUSED(argv);

	if (argv[1][0] == 0) {
		shellPrintError(CLI_CRC32, "file name");
		return 0;
	}

	uint32_t fileSize = lfsGetFileSize(argv[1]);
	uint8_t data[fileSize];
	memset(data, 0, fileSize);
	int err = lfsReadFile(argv[1], data);
	if (err < 1) {
		printf("\n");
		return 0;
	}

	uint32_t crc = cksumCalcCrc32(data, fileSize);
	printf("%lX\n\n", crc);

	return 1;
}

static uint8_t shellFORMAT(int argc, char **argv) {

	UNUSED(argc);
	UNUSED(argv);

	lfsEraseDevice();

	return 1;
}

static uint8_t shellHELP(int argc, char **argv) {

	UNUSED(argc);
	UNUSED(argv);

	uint32_t numCmds = sizeof(commands) / sizeof(struct shellCommand_t);
	uint32_t nameLen = strlen((char*) commands[0].name);

	//	find the number of max spaces to add to format the column
	for (int i = 1; i < numCmds; i++) {
		int tempLen = strlen((char*) commands[i].name);
		if (tempLen > nameLen) {
			nameLen = tempLen;
		}
	}

	printf("\nAvailable Commands:\n");

	//	find the number of spaces to add after considering the name being printed
	for (int i = 0; i < numCmds; i++) {
		int gap = nameLen - strlen((char*) commands[i].name);
		gap++;

		//		Add an extra spaces if this command does not need a file_name
		if (!commands[i].cmdTakesParam1) {
			gap += strlen("[file_name]");
		}
		if (!commands[i].cmdTakesParam2) {
			gap += strlen("[num_bytes]");
		}

		if (commands[i].param2IsNum == 1) {
			printf("\t%s %s %s %*c ", commands[i].name,
					commands[i].cmdTakesParam1 ? "[file_name]" : "",
					commands[i].cmdTakesParam2 ? "[num_bytes]" : "", gap, ':');
		} else {
			printf("\t%s %s %s %*c ", commands[i].name,
					commands[i].cmdTakesParam1 ? "[file_name]" : "",
					commands[i].cmdTakesParam2 ? "[file_name]" : "", gap, ':');
		}
		printf("%s\n", commands[i].desc);
	}
	printf("\n");

	return 1;
}

static uint8_t shellHEXDUMP(int argc, char **argv) {

	UNUSED(argc);
	UNUSED(argv);

	if (argv[1][0] == 0) {
		shellPrintError(CLI_HEXDUMP, "file name");
		return 0;
	}

	uint32_t size = 0;
	uint32_t fileSize = lfsGetFileSize(argv[1]);

	// Check number of bytes to show was specified
	if (argv[2][0] >= '0' && argv[2][0] <= '9') {
		size = atoi(argv[2]);
	} else {
		size = fileSize;
	}

	uint8_t contents[fileSize];
	memset(contents, 0, fileSize);
	int err = lfsReadFile(argv[1], contents);

	if (err < 1) {
		printf("\n");
		return 0;
	}

	shellPrintHexdump(argv[1], contents, size);

	return 1;
}

static uint8_t shellLS(int argc, char **argv) {

	UNUSED(argc);
	UNUSED(argv);
	lfsLs();

	return 1;
}

static uint8_t shellMV(int argc, char **argv) {

	UNUSED(argc);
	UNUSED(argv);

	if (argv[1][0] == 0) {
		shellPrintError(CLI_MV, "file name");
		return 0;
	}

	if (argv[2][0] == 0) {
		shellPrintError(CLI_MV, "file name");
		return 0;
	}

	lfsRenameFile(argv[1], argv[2]);
	printf("Moved %s to %s\n\n", argv[1], argv[2]);

	return 1;
}

static uint8_t shellRM(int argc, char **argv) {

	UNUSED(argc);
	UNUSED(argv);
	if (argv[1][0] == 0) {
		shellPrintError(CLI_RM, "file name");
		return 0;
	}

	printf("Removing %s...\n", argv[1]);
	lfsRemoveFile(argv[1]);
	printf("Removing %s complete\n\n", argv[1]);
	return 1;
}

static uint8_t shellTOUCH(int argc, char **argv) {

	UNUSED(argc);
	UNUSED(argv);

	lfsCreateFile(argv[1]);
	printf("\n");

	return 1;
}

static uint8_t shellVI(int argc, char **argv) {

	UNUSED(argc);
	UNUSED(argv);

	if (argv[1][0] == 0) {
		shellPrintError(CLI_VI, "file name");
		return 0;
	}

	if (!(argv[2][0] >= '0' && argv[2][0] <= '9')) {
		shellPrintError(CLI_VI, "file size");
		return 0;
	}
	uint32_t fileSize = (uint32_t) atol(argv[2]);

	lfsOpenFileForWrite(argv[1]);
	uartRxFile(argv[1], fileSize);
	lfsCloseFileForWrite();

	return 1;
}
/**
 * Shell commands: CLOSE
 */
