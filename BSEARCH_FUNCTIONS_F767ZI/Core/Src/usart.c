/**
 ******************************************************************************
 * @file    usart.c
 * @brief   This file provides code for the configuration
 *          of the USART instances.
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

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include <stdio.h>
#include <string.h>
#include "shell.h"
uartRxStruct uartRx;

#define DBG_RX_HANDLER 0

/* USER CODE END 0 */

UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_tx;
DMA_HandleTypeDef hdma_usart3_rx;

/* USART3 init function */

void MX_USART3_UART_Init(void) {

	/* USER CODE BEGIN USART3_Init 0 */

	/* USER CODE END USART3_Init 0 */

	/* USER CODE BEGIN USART3_Init 1 */

	/* USER CODE END USART3_Init 1 */
	huart3.Instance = USART3;
	huart3.Init.BaudRate = 115200;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart3) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART3_Init 2 */

	/* USER CODE END USART3_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle) {

	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	if (uartHandle->Instance == USART3) {
		/* USER CODE BEGIN USART3_MspInit 0 */

		/* USER CODE END USART3_MspInit 0 */
		/* USART3 clock enable */
		__HAL_RCC_USART3_CLK_ENABLE();

		__HAL_RCC_GPIOD_CLK_ENABLE();
		/**USART3 GPIO Configuration
		 PD8     ------> USART3_TX
		 PD9     ------> USART3_RX
		 */
		GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
		HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

		/* USART3 DMA Init */
		/* USART3_TX Init */
		hdma_usart3_tx.Instance = DMA1_Stream3;
		hdma_usart3_tx.Init.Channel = DMA_CHANNEL_4;
		hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_usart3_tx.Init.Mode = DMA_NORMAL;
		hdma_usart3_tx.Init.Priority = DMA_PRIORITY_MEDIUM;
		hdma_usart3_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		if (HAL_DMA_Init(&hdma_usart3_tx) != HAL_OK) {
			Error_Handler();
		}

		__HAL_LINKDMA(uartHandle, hdmatx, hdma_usart3_tx);

		/* USART3_RX Init */
		hdma_usart3_rx.Instance = DMA1_Stream1;
		hdma_usart3_rx.Init.Channel = DMA_CHANNEL_4;
		hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_usart3_rx.Init.Mode = DMA_NORMAL;
		hdma_usart3_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
		hdma_usart3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		if (HAL_DMA_Init(&hdma_usart3_rx) != HAL_OK) {
			Error_Handler();
		}

		__HAL_LINKDMA(uartHandle, hdmarx, hdma_usart3_rx);

		/* USART3 interrupt Init */
		HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USART3_IRQn);
		/* USER CODE BEGIN USART3_MspInit 1 */

		/* USER CODE END USART3_MspInit 1 */
	}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle) {

	if (uartHandle->Instance == USART3) {
		/* USER CODE BEGIN USART3_MspDeInit 0 */

		/* USER CODE END USART3_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_USART3_CLK_DISABLE();

		/**USART3 GPIO Configuration
		 PD8     ------> USART3_TX
		 PD9     ------> USART3_RX
		 */
		HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8 | GPIO_PIN_9);

		/* USART3 DMA DeInit */
		HAL_DMA_DeInit(uartHandle->hdmatx);
		HAL_DMA_DeInit(uartHandle->hdmarx);

		/* USART3 interrupt Deinit */
		HAL_NVIC_DisableIRQ(USART3_IRQn);
		/* USER CODE BEGIN USART3_MspDeInit 1 */

		/* USER CODE END USART3_MspDeInit 1 */
	}
}

/* USER CODE BEGIN 1 */
static void uartClearRcvdWords(uartRxStruct *data);
static void uartClearRcvdString(uartRxStruct *data);
static void uartRxProcess(uartRxStruct *data);

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {

	if (huart->Instance == INSTANCE_UART3) {

	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	if (huart->Instance == INSTANCE_UART3) {
		uartRxProcess(&uartRx);
		HAL_UART_Receive_IT(HANDLE_UART3, &uartRx.rxChar, 1);
	}

}

static void uartClearRcvdWords(uartRxStruct *data) {
	for (int i = 0; i < UART_RX_NUM_WORDS; i++) {
		memset(data->rxWords[i], 0, UART_RX_WORD_SIZE);
	}
}

static void uartClearRcvdString(uartRxStruct *data) {
	memset(data->rxString, 0, UART_RX_STRING_SIZE);
}

static void uartRxProcess(uartRxStruct *data) {

	//check for end of message
	if (data->rxChar == '\n') {

		data->rxStringCompFlag = 1;
#if DBG_RX_HANDLER
		printf("%s\n", data->rxString);
		for (int i = 0; i < UART_RX_NUM_WORDS; i++) {
			printf("Word%d: %s\n", i, data->rxWords[i]);
		}
		printf("\n\n");
#endif

		//call string handler here
		shellProcess(*data);

		uartClearRcvdString(data);
		uartClearRcvdWords(data);

		data->rxWordPos = 0;

	}
	//do nothing on carriage return
	else if (data->rxChar == '\r') {

	}
	//process space - increment word pos
	else if (data->rxChar == ' ') {
		//concatenate into entire string
		strncat((char*) data->rxString, (char*) &data->rxChar, 1);

		//separate words by space
		//only increment word count IF we can store more words
		//and ignore extra spaces between chars
		if ((data->rxWordPos < UART_RX_NUM_WORDS)) {
			if (data->rxWords[data->rxWordPos][0] != '\0') {
				data->rxWordPos++;
			}
		} else {
			data->rxWordPos = 0;
			uartClearRcvdWords(data);
		}
	}
	//handle backspace
	else if (data->rxChar == '\b') {
		//reduce strlen of rcvd string by null terminating 1 index early
		data->rxString[strlen((char*) data->rxString) - 1] = '\0';

		if (data->rxWords[data->rxWordPos][0] == '\0') {
			data->rxWordPos--;
		} else {
			data->rxWords[data->rxWordPos][strlen(
					(char*) data->rxWords[data->rxWordPos]) - 1] = '\0';
		}
	}
	//
	else {
		strncat((char*) data->rxString, (char*) &data->rxChar, 1);
		strncat((char*) data->rxWords[data->rxWordPos], (char*) &data->rxChar,
				1);
	}
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
