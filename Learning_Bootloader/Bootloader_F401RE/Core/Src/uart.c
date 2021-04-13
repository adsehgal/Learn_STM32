/*
 * uart.c
 *
 *  Created on: Apr 12, 2021
 *      Author: adityasehgal
 */

#include "uart.h"

void printMsg(char *format, ...)
{
	char str[80];

	/*Extract the the argument list using VA apis */
	va_list args;
	va_start(args, format);
	vsprintf(str, format, args);
	HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
	va_end(args);
}
