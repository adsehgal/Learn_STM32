/*
 * uart.h
 *
 *  Created on: Aug 10, 2021
 *      Author: adityasehgal
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "main.h"

#define CMD_NAME_SIZE 64
#define CMD_DESC_SIZE 128

typedef struct {
	uint8_t cmdName[CMD_NAME_SIZE];
	uint8_t cmdTakesParam1;
	uint8_t cmdTakesParam2;
	uint8_t param2Num;	//0=name, 1=numbytes //only considered if param2 is needed
	uint8_t cmdDesc[CMD_DESC_SIZE];
} uartCmds;

void uartUiDecode(void);

#endif /* INC_UART_H_ */
