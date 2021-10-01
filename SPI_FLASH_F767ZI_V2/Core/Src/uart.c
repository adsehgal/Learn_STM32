/*
 * uart.c
 *
 *  Created on: Aug 10, 2021
 *      Author: adityasehgal
 */

#include <string.h>
#include <ctype.h>
#include "uart.h"
#include "lfs_interface.h"

extern UART_HandleTypeDef huart3;
extern CRC_HandleTypeDef hcrc;

extern struct lfs_config cfg;
extern lfs_t lfs;
extern lfs_file_t file;
extern lfs_dir_t dir;
extern const char *rootDir;

#define UART_RX_BUFF_SIZE 128

const uartCmds cmds[] = {	//
		//
				{ 					//help
				.cmdName = CLI_HELP,									//
						.cmdTakesname = 0,								//
						.cmdDesc = "Lists all available functions",		//
				},//
				{ 					//touch - create new file
				.cmdName = CLI_TOUCH,									//
						.cmdTakesname = 1,								//
						.cmdDesc = "Creates a new file",				//
				},//
				{ 					//vi - open file to write
				.cmdName = CLI_VI,										//
						.cmdTakesname = 1,								//
						.cmdDesc = "Opens the file to write",			//
				},//
				{ 					//ls - list files
				.cmdName = CLI_LS,										//
						.cmdTakesname = 0,								//
						.cmdDesc = "lists all available files",			//
				},//
				{ 					//cat - read entire file
				.cmdName = CLI_CAT,										//
						.cmdTakesname = 1,								//
						.cmdDesc = "Reads out the entire file at once",	//
				},//
				{ 					//rm - removes the file
				.cmdName = CLI_RM,										//
						.cmdTakesname = 1,								//
						.cmdDesc = "Removes the file",					//
				},//
				{ 					//format
				.cmdName = CLI_FORMAT,									//
						.cmdTakesname = 0,								//
						.cmdDesc = "Erases the entire flash storage device",//
				},//
				{					//hexdump
				.cmdName = CLI_HEXDUMP, .cmdTakesname = 1, .cmdDesc =
						"Prints out the hexdump of the requested file", },	//

		};

uint8_t uartRecvChar[1];

typedef struct {
	uint8_t rxFlag;
	uint8_t rxBuff[UART_RX_BUFF_SIZE];
} structUartRx;

structUartRx uartRx;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (uartRecvChar[0] == '\n') {
		uartRx.rxFlag = 1;

		//only decode if actual text was sent
		//otherwise ignore
		if (uartRx.rxBuff[0] && uartRx.rxBuff[0] != '\n') {
			uartUiDecode();
		} else {
			memset(uartRx.rxBuff, 0, sizeof(uartRx.rxBuff));
			uartRx.rxFlag = 0;
		}
		memset(uartRx.rxBuff, 0, sizeof(uartRx.rxBuff));
	} else if (uartRecvChar[0] == '\r') {
//		do nothing
	} else {

		if (uartRecvChar[0] == 0x08) {	//backspace handle
			uartRx.rxBuff[strlen((char*) uartRx.rxBuff) - 1] = '\0';
		} else
			strncat((char*) uartRx.rxBuff, (char*) uartRecvChar, 1);
	}

	uartRecvChar[0] = 0;
	HAL_UART_Receive_IT(huart, uartRecvChar, 1);	//wait for next interrupt

}

static void uartPrintHelp(void) {
	int numCmds = CMD_NUM_CMDS;
	int nameLen = strlen((char*) cmds[0].cmdName);

//	find the number of max spaces to add to format the column
	for (int i = 1; i < numCmds; i++) {
		int tempLen = strlen((char*) cmds[i].cmdName);
		if (tempLen > nameLen) {
			nameLen = tempLen;
		}
	}

	printf("\nAvailable Commands:\n");

//	find the number of spaces to add after considering the name being printed
	for (int i = 0; i < numCmds; i++) {
		int gap = nameLen - strlen((char*) cmds[i].cmdName);

//		Add an extra spaces if this command does not need a file_name
		if (!cmds[i].cmdTakesname) {
			gap += strlen("[file_name]");
		}

		printf("\t%s %s %*c ", cmds[i].cmdName,
				cmds[i].cmdTakesname ? "[file_name]" : "", gap, ':');
		printf("%s\n", cmds[i].cmdDesc);
	}
	printf("\n");
}

//static void uartFuncNotImpl(char *name) {
//	printf("\"%s\" function not implemented yet\n", name);
//}

static void uartPrintFileRead(char *name, char *contents, int32_t size) {
	if (!size) {
		return;
	}

	printf("%s[%ld]:\n", name, size);
	for (int32_t i = 1; i <= size; i++) {
		printf("%c", contents[i - 1]);
	}

	printf("\n\n");
}

static void uartprintHexdump(char *name, uint8_t *contents, int32_t size) {
	if (!size) {
		return;
	}
	printf("%s[%ld]:\n", name, size);

	for (int32_t i = 1; i <= size; i++) {

		if (((i - 1) % 16) == 0) {
			printf("\t0x%08lX    ", i - 1);
		}
		if (contents[i - 1] != 0)
			printf("%02X%c", contents[i - 1], (i % 16 == 0) ? '\n' : ' ');
	}

	printf("\n\n");
}

static void uartPrintMissingDescriptor(char *func, char *descriptor) {
	printf("[%s] Please provide a %s.\n\n", func, descriptor);
}

void uartUiDecode(void) {
	char delimiter[4] = " ";
	char *token = 0;
	char *originalStr = 0;
	memcpy((uint8_t*) originalStr, uartRx.rxBuff, sizeof(uartRx.rxBuff));
	token = strtok((char*) uartRx.rxBuff, delimiter);

	if (!strcasecmp(token, CLI_HELP)) {

		uartPrintHelp();

	} else if (!strcasecmp(token, CLI_TOUCH)) {

		token = strtok(NULL, delimiter);
		if (token == NULL) {
			uartPrintMissingDescriptor(CLI_TOUCH, "file name");
			return;
		}
//		lfs_dir_rewind(&lfs, &dir);
		lfs_file_close(&lfs, &file);	//close any open files
		lfs_file_open(&lfs, &file, token, LFS_O_CREAT);
		lfs_file_rewind(&lfs, &file);
		lfs_file_close(&lfs, &file);

	} else if (!strcasecmp(token, CLI_VI)) {

		token = strtok(NULL, delimiter);
		if (token == NULL) {
			uartPrintMissingDescriptor(CLI_VI, "file name");
			return;
		}
		lfs_file_open(&lfs, &file, token, LFS_O_CREAT);
		lfs_file_close(&lfs, &file);

	} else if (!strcasecmp(token, CLI_LS)) {

		lfsLs();

	} else if (!strcasecmp(token, CLI_CAT)) {

		token = strtok(NULL, delimiter);

		if (token == NULL) {
			uartPrintMissingDescriptor(CLI_CAT, "file name");
			return;
		}

		char *contents = 0;
		int32_t size = lfsReadFile(token, contents);

		uartPrintFileRead(token, contents, size);

	} else if (!strcasecmp(token, CLI_RM)) {

		token = strtok(NULL, delimiter);
		if (token == NULL) {
			uartPrintMissingDescriptor(CLI_RM, "file name");
			return;
		}

		printf("Removing %s...\n", token);
		lfs_remove(&lfs, token);
		printf("Removing %s complete\n", token);

	} else if (!strcasecmp(token, CLI_FORMAT)) {

		printf("Formatting Device...\n");
		lfsEraseDevice();
		printf("Format complete\n");

	} else if (!strcasecmp(token, CLI_HEXDUMP)) {
		token = strtok(NULL, delimiter);
		if (token == NULL) {
			uartPrintMissingDescriptor(CLI_HEXDUMP, "file name");
			return;
		}

		lfs_file_open(&lfs, &file, token, LFS_O_RDONLY);
		int32_t size = lfs_file_size(&lfs, &file);
		lfs_file_close(&lfs, &file);
		uint8_t contents[size];
		memset(contents, 0, size);
		lfsReadFile(token, (char*) contents);

		uartprintHexdump(token, contents, size);

	} else if (!strcasecmp(token, "\n")) {

		return;

	} else {

		printf("\"%s\" is an unkown command\n", originalStr);//uartRx.rxBuff);

	}
}
