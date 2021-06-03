/*
 * spifs.c
 *
 *  Created on: Apr 29, 2021
 *      Author: adityasehgal
 */

#include <stdio.h>
#include <string.h>
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

uint8_t spifsReadStatus(void) {

	HAL_StatusTypeDef err = HAL_OK;

	uint8_t send[1] = { SPI_FS_CMD_READ_STATUS_REG };

	uint8_t recv[1] = { 0 };

	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET); //select flash

//	err |= HAL_SPI_TransmitReceive(&hspi1, send, recv, 1, HAL_MAX_DELAY);
	err |= HAL_SPI_Transmit(&hspi1, send, 1, HAL_MAX_DELAY);
	err |= HAL_SPI_Receive(&hspi1, recv, 1, HAL_MAX_DELAY);

	if (err != HAL_OK) {
		printf("Transceive failed!\n");
	}

	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

	return recv[0];
}

static uint8_t W25qxx_Spi(uint8_t Data) {
	uint8_t ret;
	HAL_SPI_TransmitReceive(&hspi1, &Data, &ret, 1, 100);
	return ret;
}

void W25qxx_ReadByte(uint8_t *pBuffer, uint32_t Bytes_Address) {
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);
	W25qxx_Spi(0x0B);
	W25qxx_Spi((Bytes_Address & 0xFF0000) >> 16);
	W25qxx_Spi((Bytes_Address & 0xFF00) >> 8);
	W25qxx_Spi(Bytes_Address & 0xFF);
	W25qxx_Spi(0);
	*pBuffer = W25qxx_Spi(0xA5);
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);
}

void spifsRead(uint32_t byteAddr, uint8_t *data, uint32_t length) {

	if (!length)
		return;

	HAL_StatusTypeDef err = HAL_OK;

	uint8_t send[length];
	memset(send, SPI_FS_CMD_DUMMY, length);

//	uint8_t recv[length] = { 0 };

	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET); //select flash

	send[0] = SPI_FS_CMD_READ;
	err |= HAL_SPI_Transmit(&hspi1, send, length, HAL_MAX_DELAY);
	send[0] = byteAddr & 0x000000FF;
	send[1] = (byteAddr >> 8) & 0x000000FF;
	send[2] = (byteAddr >> 16) & 0x000000FF;
//	err |= HAL_SPI_TransmitReceive(&hspi1, send, data, length, HAL_MAX_DELAY);
	err |= HAL_SPI_Transmit(&hspi1, send, length, HAL_MAX_DELAY);
	err |= HAL_SPI_Receive(&hspi1, data, length, HAL_MAX_DELAY);

	if (err != HAL_OK) {
		printf("Transceive failed!\n");
	}

	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);
}

void spifsPageProgram(uint32_t byteAddr, uint8_t *data, uint32_t length) {

	if (!length)
		return;

	HAL_StatusTypeDef err = HAL_OK;

	uint8_t send[4] = { 0 };
//	uint8_t sendF[4] = { 0 };
//	uint8_t dataF[length];
	send[0] = SPI_FS_CMD_PAGE_PROGRAM;
	send[1] = (byteAddr >> 16) & 0x000000FF;
	send[2] = (byteAddr >> 8) & 0x000000FF;
	send[3] = (byteAddr >> 0) & 0x000000FF;

	for (uint32_t i = 0; i < length; i++) {
		HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET); //select flash

		err |= HAL_SPI_Transmit(&hspi1, send, 4, HAL_MAX_DELAY);
		err |= HAL_SPI_Transmit(&hspi1, (uint8_t*) 0xAA, 1, HAL_MAX_DELAY);

		if (err != HAL_OK) {
			printf("Transceive failed!\n");
		}

		HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);
	}

//	while (spifsReadStatus() & 0x01)
//		HAL_Delay(1);
}

void spifsWriteEnable(uint8_t val) {

	HAL_StatusTypeDef err = HAL_OK;

	uint8_t send[1] = { SPI_FS_CMD_WRITE_ENABLE };
	if (!val)
		send[0] = SPI_FS_CMD_WRITE_DISABLE;

	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET); //select flash

	err |= HAL_SPI_Transmit(&hspi1, send, 1, HAL_MAX_DELAY);

	if (err != HAL_OK) {
		printf("Transceive failed!\n");
	}

	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);
}

HAL_StatusTypeDef spifsEraseChip(void) {
	while (spifsReadStatus() & 0x01)
		HAL_Delay(1);

	HAL_StatusTypeDef err = HAL_OK;

	uint8_t send[1] = { SPI_FS_CMD_CHIP_ERASE };

	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET); //select flash

	err |= HAL_SPI_Transmit(&hspi1, send, 1, HAL_MAX_DELAY);

	if (err != HAL_OK) {
		printf("Transceive failed!\n");
	}

	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

	return err;

}
