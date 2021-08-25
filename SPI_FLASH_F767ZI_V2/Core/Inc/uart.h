/*
 * uart.h
 *
 *  Created on: Aug 10, 2021
 *      Author: adityasehgal
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "main.h"
#include "lfs.h"

#define CMD_NAME_SIZE 32
#define CMD_DESC_SIZE 128
#define CMD_NUM_CMDS 7

#define CLI_HELP	"HELP"
#define CLI_TOUCH	"TOUCH"
#define CLI_VI		"VI"
#define CLI_LS		"LS"
#define CLI_CAT		"CAT"
#define CLI_RM		"RM"
#define CLI_FORMAT	"FORMAT"

typedef struct{
	uint8_t cmdName[CMD_NAME_SIZE];
	uint8_t cmdTakesname;
	uint8_t cmdDesc[CMD_DESC_SIZE];
} uartCmds;


void uartUiDecode(void);

#endif /* INC_UART_H_ */
