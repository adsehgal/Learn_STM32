/*
 * uart.c
 *
 *  Created on: Aug 10, 2021
 *      Author: adityasehgal
 */

#include <string.h>
#include "uart.h"
#include "lfs_interface.h"

extern struct lfs_config cfg;
extern lfs_t lfs;
extern lfs_file_t file;

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
				},					//

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
		uartUiDecode();
		memset(uartRx.rxBuff, 0, sizeof(uartRx.rxBuff));
	} else if (uartRecvChar[0] == '\r') {
//		do nothing
	} else {
		strncat((char*) uartRx.rxBuff, (char*) uartRecvChar, 1);
	}

	uartRecvChar[0] = 0;
	HAL_UART_Receive_IT(huart, uartRecvChar, 1);	//wait for next interrupt

}

static void uartPrintHelp(void) {
	int numCmds = CMD_NUM_CMDS;
	int nameLen = strlen((char*) cmds[0].cmdName);

//	find the number of max spaces to add to format the colum
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
}

static void uartFuncNotImpl(char *name) {
	printf("\"%s\" function not implemented yet\n", name);
}

static void uartPrintFileRead(char *name, char *contents, int size) {
	printf("%s:\n\t ", name);
	for (int i = 1; i <= size; i++) {
		printf("%c", contents[i - 1]);
//		if (!(i % size)) {
//			printf("\n\t");
//		}
	}
	printf("\n\n");
}

void uartUiDecode(void) {
	char delimiter[4] = " ";
	char *token = 0;

	token = strtok((char*) uartRx.rxBuff, delimiter);

	if (!strcasecmp(token, CLI_HELP)) {
		uartPrintHelp();
	} else if (!strcasecmp(token, CLI_TOUCH)) {
		uartFuncNotImpl(token);
	} else if (!strcasecmp(token, CLI_VI)) {
		uartFuncNotImpl(token);
	} else if (!strcasecmp(token, CLI_LS)) {
		lfsLs();
	} else if (!strcasecmp(token, CLI_CAT)) {

		token = strtok(NULL, delimiter);
		char *contents = 0;

		lfs_file_open(&lfs, &file, token, LFS_O_RDWR | LFS_O_CREAT);
		lfs_soff_t size = lfs_file_size(&lfs, &file);

		lfs_file_read(&lfs, &file, contents, size);

		lfs_file_close(&lfs, &file);

		//	lfs_unmount(&lfs);
		printf("Read [%d]: %s\n", size, contents);

		uartPrintFileRead(token, contents, size);

	} else if (!strcasecmp(token, CLI_RM)) {
		uartFuncNotImpl(token);
	} else if (!strcasecmp(token, CLI_FORMAT)) {
		printf("Formatting Device...\n");
		lfsEraseDevice();
		printf("Format complete\n");
	} else {
		printf("\"%s\" is an unkown command\n", token);
	}
}
