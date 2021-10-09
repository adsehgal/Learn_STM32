/*
 * uart.c
 *
 *  Created on: Aug 10, 2021
 *      Author: adityasehgal
 */

#include <string.h>
#include <ctype.h>
#include "uart.h"
#include "cksum.h"
#include "lfs_interface.h"

extern UART_HandleTypeDef huart3;
extern CRC_HandleTypeDef hcrc;

extern struct lfs_config cfg;
extern lfs_t lfs;
extern lfs_file_t file;
extern lfs_dir_t dir;
extern const char *rootDir;

#define UART_RX_BUFF_SIZE 128

#define CMD_NUM_CMDS 10
#define CLI_HELP	"HELP"
#define CLI_TOUCH	"TOUCH"
#define CLI_VI		"VI"
#define CLI_LS		"LS"
#define CLI_CAT		"CAT"
#define CLI_RM		"RM"
#define CLI_FORMAT	"FORMAT"
#define CLI_HEXDUMP "HEXDUMP"
#define CLI_MV		"MV"
#define CLI_CKSUM	"CKSUM"
const uartCmds cmds[] = {	//
		//
				{ 					//help
				.cmdName = CLI_HELP,									//
						.cmdTakesParam1 = 0,							//
						.cmdTakesParam2 = 0,							//
						.param2Num = 0,									//
						.cmdDesc = "Lists all available functions",	//
				},//
				{ 					//touch - create new file
				.cmdName = CLI_TOUCH,									//
						.cmdTakesParam1 = 1,							//
						.cmdTakesParam2 = 0,							//
						.param2Num = 0,									//
						.cmdDesc = "Creates a new empty file",			//
				},//
				{ 					//vi - open file to write
				.cmdName = CLI_VI,										//
						.cmdTakesParam1 = 1,							//
						.cmdTakesParam2 = 1,							//
						.param2Num = 1,									//
						.cmdDesc = "Write data to file",				//
				},//
				{ 					//ls - list files
				.cmdName = CLI_LS,										//
						.cmdTakesParam1 = 0,							//
						.cmdTakesParam2 = 0,							//
						.param2Num = 0,									//
						.cmdDesc = "lists all available files",		//
				},//
				{ 					//cat - read entire file
				.cmdName = CLI_CAT,										//
						.cmdTakesParam1 = 1,							//
						.cmdTakesParam2 = 0,							//
						.param2Num = 0,									//
						.cmdDesc = "Reads out the entire file at once",	//
				},//
				{ 					//rm - removes the file
				.cmdName = CLI_RM,										//
						.cmdTakesParam1 = 1,							//
						.cmdTakesParam2 = 0,							//
						.param2Num = 0,									//
						.cmdDesc = "Removes the file",					//
				},//
				{ 					//format
				.cmdName = CLI_FORMAT,									//
						.cmdTakesParam1 = 0,							//
						.cmdTakesParam2 = 0,							//
						.param2Num = 0,									//
						.cmdDesc = "Erases the entire flash storage device",//
				},//
				{					//hexdump
						.cmdName = CLI_HEXDUMP, 						//
						.cmdTakesParam1 = 1, 							//
						.cmdTakesParam2 = 1,							//
						.param2Num = 1,									//
						.cmdDesc =
								"Prints out the hexdump of file, optional byte limit",	//
				},//
				{ 					//format
				.cmdName = CLI_MV,										//
						.cmdTakesParam1 = 1,							//
						.cmdTakesParam2 = 1,							//
						.param2Num = 0,									//
						.cmdDesc = "Renames file",						//
				},//
				{ 					//help
				.cmdName = CLI_CKSUM,									//
						.cmdTakesParam1 = 1,							//
						.cmdTakesParam2 = 0,							//
						.param2Num = 0,									//
						.cmdDesc = "Calculates CRC32 value of specified file",	//
				},		//

		};

uint8_t uartRecvChar[1];

typedef struct {
	uint8_t rxFlag;
	uint8_t rxBuff[UART_RX_BUFF_SIZE];
	uint8_t rxLastCmd[UART_RX_BUFF_SIZE];
} structUartRx;

structUartRx uartRx = { .rxFlag = 0, };

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	if (uartRecvChar[0] == '\n') {
		uartRx.rxFlag = 1;

		//only decode if actual text was sent
		//otherwise ignore
		if (uartRx.rxBuff[0] && uartRx.rxBuff[0] != '\n') {
			memcpy(uartRx.rxLastCmd, uartRx.rxBuff, sizeof(uartRx.rxBuff));
			uartUiDecode();
		} else {
			memset(uartRx.rxBuff, 0, sizeof(uartRx.rxBuff));
			uartRx.rxFlag = 0;
		}
		memset(uartRx.rxBuff, 0, sizeof(uartRx.rxBuff));
	} else if (uartRecvChar[0] == '\r') {
//		do nothing
	} else if (uartRecvChar[0] == 0x03) {	//end of text

		HAL_UART_Abort(huart);
		memset(uartRx.rxBuff, 0, sizeof(uartRx.rxBuff));
		printf("\n");

	} else if (uartRecvChar[0] == 0x1E) {	//previous command
//		memset(uartRx.rxBuff, 0, sizeof(uartRx.rxBuff));
		memcpy(uartRx.rxBuff, uartRx.rxLastCmd, sizeof(uartRx.rxLastCmd));
		printf("\n%s", uartRx.rxBuff);

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
		gap++;

//		Add an extra spaces if this command does not need a file_name
		if (!cmds[i].cmdTakesParam1) {
			gap += strlen("[file_name]");
		}
		if (!cmds[i].cmdTakesParam2) {
			gap += strlen("[num_bytes]");
		}

		if (cmds[i].param2Num == 1) {
			printf("\t%s %s %s %*c ", cmds[i].cmdName,
					cmds[i].cmdTakesParam1 ? "[file_name]" : "",
					cmds[i].cmdTakesParam2 ? "[num_bytes]" : "", gap, ':');
		} else {
			printf("\t%s %s %s %*c ", cmds[i].cmdName,
					cmds[i].cmdTakesParam1 ? "[file_name]" : "",
					cmds[i].cmdTakesParam2 ? "[file_name]" : "", gap, ':');
		}
		printf("%s\n", cmds[i].cmdDesc);
	}
	printf("\n");
}

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
		printf("%02X%c", contents[i - 1], (i % 16 == 0) ? '\n' : ' ');
	}

	printf("\n\n");
}

static void uartPrintMissingDescriptor(char *func, char *descriptor) {
	printf("[%s] Please provide a %s.\n\n", func, descriptor);
}

static void uartRxFile(char *name, uint32_t size) {
	// temporarily disable UART interrupts to receive full file
	HAL_NVIC_DisableIRQ(USART3_IRQn);

	uint8_t recv[size];

	// unable to rx file in interrupt/dma mode, so using blocking function as of now
	HAL_UART_Receive(&huart3, recv, size, HAL_MAX_DELAY);
	printf("\nRecvd %lu bytes\n", size);
	int err = lfs_file_write(&lfs, &file, (uint8_t*) recv, size);
	printf("Wrote %d of %lu bytes \n\n", err, size);

	//re-enable UART interrupts
	HAL_NVIC_EnableIRQ(USART3_IRQn);
}

void uartUiDecode(void) {
	char delimiter[4] = " ";
	char *token = 0;
	char *originalStr = 0;
	memcpy((uint8_t*) originalStr, uartRx.rxBuff, sizeof(uartRx.rxBuff));
	token = strtok((char*) uartRx.rxBuff, delimiter);

	/**
	 * HELP
	 */
	if (!strcasecmp(token, CLI_HELP)) {

		uartPrintHelp();

	}
	/**
	 * TOUCH
	 */
	else if (!strcasecmp(token, CLI_TOUCH)) {

		token = strtok(NULL, delimiter);
		if (token == NULL) {
			uartPrintMissingDescriptor(CLI_TOUCH, "file name");
			return;
		}

		lfsCreateFile(token);
		printf("\n");

	}
	/**
	 * VI
	 */
	else if (!strcasecmp(token, CLI_VI)) {

		char *fileName = strtok(NULL, delimiter);

		if (fileName == NULL) {
			uartPrintMissingDescriptor(CLI_VI, "file name");
			return;
		}

		char *size_s = strtok(NULL, delimiter);
		uint32_t fileSize = (uint32_t) atol(size_s);
		if (fileSize < 1) {
			uartPrintMissingDescriptor(CLI_VI, "file size");
			return;
		}

		lfs_file_close(&lfs, &file);
		lfs_file_open(&lfs, &file, fileName,
				LFS_O_CREAT | LFS_O_RDWR | LFS_O_APPEND);
		uartRxFile(token, fileSize);
		lfs_file_rewind(&lfs, &file);
		lfs_file_close(&lfs, &file);

	}
	/**
	 * LS
	 */
	else if (!strcasecmp(token, CLI_LS)) {

		lfsLs();

	}
	/**
	 * CAT
	 */
	else if (!strcasecmp(token, CLI_CAT)) {

		token = strtok(NULL, delimiter);

		if (token == NULL) {
			uartPrintMissingDescriptor(CLI_CAT, "file name");
			return;
		}

		char *contents = 0;
		int32_t size = lfsReadFile(token, (uint8_t*) contents);

		if (size < 0) {
			printf("\n");
			return;
		}

		uartPrintFileRead(token, contents, size);

	}
	/**
	 * RM
	 */
	else if (!strcasecmp(token, CLI_RM)) {

		token = strtok(NULL, delimiter);
		if (token == NULL) {
			uartPrintMissingDescriptor(CLI_RM, "file name");
			return;
		}

		printf("Removing %s...\n", token);
		lfs_remove(&lfs, token);
		printf("Removing %s complete\n\n", token);

	}
	/**
	 * FORMAT
	 */
	else if (!strcasecmp(token, CLI_FORMAT)) {

		printf("Formatting Device...\n");
		lfsEraseDevice();
		printf("Format complete\n\n");

	}
	/**
	 * HEXDUMP
	 */
	else if (!strcasecmp(token, CLI_HEXDUMP)) {
		token = strtok(NULL, delimiter);
		if (token == NULL) {
			uartPrintMissingDescriptor(CLI_HEXDUMP, "file name");
			return;
		}

		char *fileName = 0;
		strcpy(fileName, token);

		int32_t fileSize = 0;
		int32_t size = 0;
//		lfs_file_open(&lfs, &file, fileName, LFS_O_RDONLY);
//		fileSize = lfs_file_size(&lfs, &file);
//		lfs_file_close(&lfs, &file);
		fileSize = lfsGetFileSize(fileName);

		token = strtok(NULL, delimiter);
		if (token == NULL) {
			size = fileSize;
		} else {
			size = atoi(token);
		}

		uint8_t contents[fileSize];
		memset(contents, 0, fileSize);
		int err = lfsReadFile(fileName, contents);

		if (err < 1) {
			printf("\n");
			return;
		}

		uartprintHexdump(fileName, contents, size);

	}
	/**
	 * MV
	 */
	else if (!strcasecmp(token, CLI_MV)) {

		char *fileName1 = strtok(NULL, delimiter);
		if (fileName1 == NULL) {
			uartPrintMissingDescriptor(CLI_MV, "file name");
			return;
		}

		char *fileName2 = strtok(NULL, delimiter);
		if (fileName2 == NULL) {
			uartPrintMissingDescriptor(CLI_MV, "file name");
			return;
		}

		lfs_rename(&lfs, fileName1, fileName2);
		printf("Moved %s to %s\n\n", fileName1, fileName2);

	}
	/**
	 * CKSUM
	 */
	else if (!strcasecmp(token, CLI_CKSUM)) {

		char *fileName = strtok(NULL, delimiter);
		if (fileName == NULL) {
			uartPrintMissingDescriptor(CLI_CKSUM, "file name");
			return;
		}

		uint32_t fileSize = lfsGetFileSize(fileName);
		uint8_t data[fileSize];
		memset(data, 0, fileSize);
		int err = lfsReadFile(fileName, data);
		if (err < 1) {
			return;
		}

		uint32_t crc = cksumCalcCrc(data, fileSize);
		printf("%lu %ld %s\n\n", crc, fileSize, fileName);
	}
	/**
	 * EMPTY LINE
	 */
	else if (!strcasecmp(token, "\n")) {

		return;

	}
	/**
	 * INVALID
	 */
	else {

		printf("\"%s\" is an unkown command\n", originalStr); //uartRx.rxBuff);

	}
}
