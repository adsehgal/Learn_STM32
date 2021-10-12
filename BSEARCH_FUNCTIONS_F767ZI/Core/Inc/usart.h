/**
 ******************************************************************************
 * @file    usart.h
 * @brief   This file contains all the function prototypes for
 *          the usart.c file
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

#define UART_RX_NUM_WORDS 4
#define UART_RX_WORD_SIZE 64
#define UART_RX_STRING_SIZE (UART_RX_NUM_WORDS * UART_RX_WORD_SIZE)
typedef struct uartRx_t {
	uint8_t rxChar;									//char rcvd on UART
	uint8_t rxStringCompFlag;						//complete string rcvd flag

	uint8_t rxWordPos;					//track how many words have been rcvd
	uint8_t rxWords[UART_RX_NUM_WORDS][UART_RX_WORD_SIZE];	//stores rcvd words

	uint8_t rxString[UART_RX_STRING_SIZE];			//stores entire string rcvd
} uartRxStruct;

/* USER CODE END Includes */

extern UART_HandleTypeDef huart3;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_USART3_UART_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
