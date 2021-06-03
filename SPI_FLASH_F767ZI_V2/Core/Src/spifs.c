/*
 * spifs.c
 *
 *  Created on: Jun 2, 2021
 *      Author: adityasehgal
 */

#include "spifs.h"
#include <stdio.h>
#include "main.h"

extern SPI_HandleTypeDef hspi1;

#define FLASH_CS_PIN SPI_CS_Pin
#define FLASH_CS_PORT SPI_CS_GPIO_Port

static uint8_t spifsSendRecvByte(uint8_t data) {

	uint8_t ret;
	HAL_SPI_TransmitReceive(&hspi1, &data, &ret, 1, HAL_MAX_DELAY);
	return ret;

}

void spifsWriteEnable(void) {

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	spifsSendRecvByte(SPI_FS_CMD_WRITE_ENABLE);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

	HAL_Delay(1);
}

void spifsWriteDisable(void) {

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	spifsSendRecvByte(SPI_FS_CMD_WRITE_DISABLE);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

	HAL_Delay(1);
}

uint8_t spifsReadStatusRegister(void) {

	uint8_t status = 0;

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	spifsSendRecvByte(SPI_FS_CMD_READ_STATUS_REG);
	status = spifsSendRecvByte(SPI_FS_CMD_DUMMY);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

	return status;

}

void spifsWriteStatusRegister(uint16_t data) {

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	spifsSendRecvByte(SPI_FS_CMD_WRITE_STATUS_REG);

	spifsSendRecvByte((data >> 8) & 0x00FF);
	spifsSendRecvByte(data & 0x00FF);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

}

void spifsWaitForWriteEnd(void) {

	HAL_Delay(1);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	spifsSendRecvByte(SPI_FS_CMD_READ_STATUS_REG);

	uint8_t status = 0;

	do {

		status = spifsSendRecvByte(SPI_FS_CMD_DUMMY);
		HAL_Delay(1);

	} while ((status & 0x01) == 0x01);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);
}

uint32_t spifsReadID(void) {

	uint32_t temp = 0, temp0 = 0, temp1 = 0, temp2 = 0;

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	spifsSendRecvByte(SPI_FS_CMD_READ_ID);

	temp0 = spifsSendRecvByte(SPI_FS_CMD_DUMMY);
	temp1 = spifsSendRecvByte(SPI_FS_CMD_DUMMY);
	temp2 = spifsSendRecvByte(SPI_FS_CMD_DUMMY);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

	temp = (temp0 << 16) | (temp1 << 8) | temp2;

	return temp;
}

void spifsEraseChip(void) {

	spifsWriteEnable();

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);
	spifsSendRecvByte(SPI_FS_CMD_CHIP_ERASE);
	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

	spifsWaitForWriteEnd();

	spifsWriteDisable();

	HAL_Delay(10);
}

void spifsWriteByte(uint8_t pBuffer, uint32_t addr) {

	spifsWaitForWriteEnd();

	spifsWriteEnable();

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	spifsSendRecvByte(SPI_FS_CMD_PAGE_PROGRAM);

	spifsSendRecvByte((addr & 0xFF0000) >> 16);
	spifsSendRecvByte((addr & 0xFF00) >> 8);
	spifsSendRecvByte(addr & 0xFF);
	spifsSendRecvByte(pBuffer);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

	spifsWaitForWriteEnd();

//	spifsWriteDisable();

}

void spifsReadByte(uint8_t *pBuffer, uint32_t addr) {

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	spifsSendRecvByte(SPI_FS_CMD_READ);

	spifsSendRecvByte((addr & 0xFF0000) >> 16);
	spifsSendRecvByte((addr & 0xFF00) >> 8);
	spifsSendRecvByte(addr & 0xFF);
	spifsSendRecvByte(0);

	*pBuffer = spifsSendRecvByte(SPI_FS_CMD_DUMMY);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

}
