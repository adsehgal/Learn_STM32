/*
 * uart.h
 *
 *  Created on: Apr 12, 2021
 *      Author: adityasehgal
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "main.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "stdarg.h"

extern UART_HandleTypeDef huart2;

void printMsg(char *format, ...);

#endif /* INC_UART_H_ */
