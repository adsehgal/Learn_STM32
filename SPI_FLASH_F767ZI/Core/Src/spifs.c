/*
 * spifs.c
 *
 *  Created on: Apr 29, 2021
 *      Author: adityasehgal
 */

#include <stdio.h>
#include "main.h"
#include "spifs.h"

extern SPI_HandleTypeDef hspi1;

HAL_StatusTypeDef spifsGetID(uint8_t *manufacturerId, uint8_t *memType,
		uint8_t *memDensity) {

	HAL_StatusTypeDef err = HAL_OK;

	uint8_t send[4] = { SPI_FS_CMD_READ_ID, SPI_FS_CMD_DUMMY,
	SPI_FS_CMD_DUMMY, SPI_FS_CMD_DUMMY };

	uint8_t recv[4] = { SPI_FS_CMD_DUMMY, SPI_FS_CMD_DUMMY, SPI_FS_CMD_DUMMY,
	SPI_FS_CMD_DUMMY };

	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET); //select flash

	err |= HAL_SPI_TransmitReceive(&hspi1, send, recv, 4, HAL_MAX_DELAY);
//	err |= HAL_SPI_Transmit(&hspi1, &send[0], 1, HAL_MAX_DELAY);
//	err |= HAL_SPI_Receive(&hspi1, &recv[0], 1, HAL_MAX_DELAY);
//	err |= HAL_SPI_Receive(&hspi1, &recv[1], 1, HAL_MAX_DELAY);
//	err |= HAL_SPI_Receive(&hspi1, &recv[2], 1, HAL_MAX_DELAY);

	if (err != HAL_OK) {
		printf("Transceive failed!\n");
	}

	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

	//recv[0] will contain the command sent
	*manufacturerId = recv[1];
	*memType = recv[2];
	*memDensity = recv[3];

	return err;
}
