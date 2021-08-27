/*
 * flash.c
 *
 *  Created on: Jun 2, 2021
 *      Author: adityasehgal
 */

#include <flash.h>
#include <stdio.h>
#include "main.h"

extern SPI_HandleTypeDef hspi1;

#define FLASH_CS_PIN SPI_CS_Pin
#define FLASH_CS_PORT SPI_CS_GPIO_Port

//#define SPI_USE_DMA

typedef struct {
	uint8_t rxComp;
	uint8_t txComp;
	uint8_t txRxComp;
} spiFlags;

spiFlags spi = { 0 };

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	spi.txRxComp = 1;
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
	spi.rxComp = 1;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	spi.txComp = 1;
}

static void spiSendBytes(uint8_t *data, uint32_t size) {

//	the flash chip requires we send out data on the rising clock edges
//	so we make sure SPI is setup to do that
	HAL_SPI_DeInit(&hspi1);

	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 7;
	hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
	if (HAL_SPI_Init(&hspi1) != HAL_OK) {
		Error_Handler();
	}

//	actually send out the data now
#ifdef SPI_USE_DMA
	HAL_SPI_Transmit_DMA(&hspi1, data, size);
	while (!spi.txComp)
		;
	spi.txComp = 0;
#else
	HAL_SPI_Transmit(&hspi1, data, size, 500);
#endif
}

static void spiRecvBytes(uint8_t *data, uint32_t size) {

//	the flash chip requires we read out data on the falling clock edges
//	so we make sure SPI is setup to do that
	HAL_SPI_DeInit(&hspi1);

	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 7;
	hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
	if (HAL_SPI_Init(&hspi1) != HAL_OK) {
		Error_Handler();
	}
//	actually read out the data now
#ifdef SPI_USE_DMA
	HAL_SPI_Receive_DMA(&hspi1, data, size);
	while (!spi.rxComp)
		;
	spi.rxComp = 0;
#else
	HAL_SPI_Receive(&hspi1, data, size, 500);
#endif

}

static void flashWriteEnable(void) {

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	uint8_t data[1] = { FLASH_CMD_WREN };
	spiSendBytes(data, 1);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

//	SYS_DELAY(1);
//	HAL_Delay(1);
}

static void flashWriteDisable(void) {

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	uint8_t data[1] = { FLASH_CMD_WRDIS };
	spiSendBytes(data, 1);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

//	SYS_DELAY(1);
//	HAL_Delay(1);
}

uint8_t flashReadStatusReg(void) {

	uint8_t status = 0;

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	uint8_t data[1] = { FLASH_CMD_READ_STATUS_REG };
	spiSendBytes(data, 1);

	spiRecvBytes(&status, 1);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

	return status;

}

void flashWriteStatusReg(uint16_t data) {

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	uint8_t send[1] = { FLASH_CMD_WRITE_STATUS_REG };
	spiSendBytes(send, 1);

	send[0] = (data >> 8) & 0x00FF;
	spiSendBytes(send, 1);
	send[0] = (data >> 0) & 0x00FF;
	spiSendBytes(send, 1);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

}

static void flashWaitForWriteEnd(uint8_t sendWren) {

//	SYS_DELAY(1);
//	HAL_Delay(1);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	uint8_t data[1] = { FLASH_CMD_READ_STATUS_REG };
	spiSendBytes(data, 1);

	uint8_t status = 0;

	do {
		spiRecvBytes(&status, 1);
//		SYS_DELAY(1);
//		HAL_Delay(1);
		if (sendWren) {
			data[0] = FLASH_CMD_WREN;
			spiSendBytes(data, 1);
			data[0] = FLASH_CMD_READ_STATUS_REG;
			spiSendBytes(data, 1);
		}

	} while ((status & 0x01) == 0x01);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);
}

static void flashWaitForBusy(void) {

//	SYS_DELAY(1);
//	HAL_Delay(1);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	uint8_t data[1] = { FLASH_CMD_READ_STATUS_REG };
	spiSendBytes(data, 1);

	uint8_t status = 0;

	do {
		spiRecvBytes(&status, 1);
//		SYS_DELAY(1);
//		HAL_Delay(1);
	} while ((status & 0x02));

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);
}

uint32_t flashReadId(void) {

	uint8_t send[1] = { FLASH_CMD_READ_ID };
	uint8_t recv[3] = { 0 };

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	spiSendBytes(send, 1);
	spiRecvBytes(recv, 3);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

	uint32_t temp = (recv[0] << 16) | (recv[1] << 8) | recv[2];

	return temp;
}

void flashEraseChip(void) {

	flashWriteEnable();

	flashWaitForWriteEnd(1);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	uint8_t send[1] = { FLASH_CMD_CHIP_ERASE };
	spiSendBytes(send, 1);
	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

	flashWaitForBusy();

	flashWriteDisable();
}

void flashEraseSector(uint32_t addr) {
	flashWriteEnable();

	flashWaitForWriteEnd(1);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	uint8_t send[1] = { FLASH_CMD_SECTOR_ERASE };
	spiSendBytes(send, 1);

	uint8_t address[3] = { 0 };
	address[0] = (addr & 0xFF0000) >> 16;
	address[1] = (addr & 0x00FF00) >> 8;
	address[2] = (addr & 0x0000FF) >> 0;
	spiSendBytes(address, 3);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

	flashWaitForBusy();

	flashWriteDisable();
}

void flashEraseBlock(uint32_t addr) {
	flashWriteEnable();

	flashWaitForWriteEnd(1);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	uint8_t send[1] = { FLASH_CMD_BLOCK_ERASE_32K };
	spiSendBytes(send, 1);

	uint8_t address[3] = { 0 };
	address[0] = (addr & 0xFF0000) >> 16;
	address[1] = (addr & 0x00FF00) >> 8;
	address[2] = (addr & 0x0000FF) >> 0;
	spiSendBytes(address, 3);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

	flashWaitForBusy();

	flashWriteDisable();
}

void flashWriteByte(uint8_t buff, uint32_t addr) {

	flashWaitForWriteEnd(0);

	flashWriteEnable();

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	uint8_t data[1] = { FLASH_CMD_PAGE_PROGRAM };
	spiSendBytes(data, 1);

	data[0] = (addr & 0xFF0000) >> 16;
	spiSendBytes(data, 1);
	data[0] = (addr & 0x00FF00) >> 8;
	spiSendBytes(data, 1);
	data[0] = (addr & 0x0000FF) >> 0;
	spiSendBytes(data, 1);
	data[0] = buff;
	spiSendBytes(data, 1);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

	flashWaitForWriteEnd(0);
}

void flashWriteBytes(uint8_t *buff, uint32_t address, uint32_t size) {
	/**
	 * 	Page size = 256 bytes
	 * 	Page count = 8192
	 *
	 * 	Sector size = 4096 bytes
	 * 	Sector count = 512
	 *
	 * 	Block size = 32K bytes or 64K bytes
	 * 	Block count = 32 for 64K blocks
	 *
	 * 	Flash only programs 256 bytes at a time - 1 page
	 * 	so if size > 255, in a for loop, keep programming 256 bytes
	 */

	for (uint32_t i = 0; i < size / 256; i++) {

		flashWaitForWriteEnd(0);

		flashWriteEnable();

		HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

		uint8_t data[1] = { FLASH_CMD_PAGE_PROGRAM };
		spiSendBytes(data, 1);

		data[0] = (address & 0xFF0000) >> 16;
		spiSendBytes(data, 1);
		data[0] = (address & 0x00FF00) >> 8;
		spiSendBytes(data, 1);
		data[0] = (address & 0x0000FF) >> 0;
		spiSendBytes(data, 1);

		spiSendBytes(buff, size - (i * 256));

		HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

		flashWaitForWriteEnd(0);

		address += 256;
		buff += 256;
	}
}

void flashReadByte(uint8_t *buff, uint32_t addr) {

	flashWaitForWriteEnd(0);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	uint8_t data[1] = { FLASH_CMD_READ_FAST };
	spiSendBytes(data, 1);

	data[0] = (addr & 0xFF0000) >> 16;
	spiSendBytes(data, 1);
	data[0] = (addr & 0x00FF00) >> 8;
	spiSendBytes(data, 1);
	data[0] = (addr & 0x0000FF) >> 0;
	spiSendBytes(data, 1);
	data[0] = FLASH_CMD_DUMMY;
	spiSendBytes(data, 1);

	spiRecvBytes(buff, 1);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

	flashWaitForWriteEnd(0);

}

void flashReadBytes(uint8_t *buff, uint32_t addr, uint32_t size) {
//	flashWaitForWriteEnd(0);
	flashWaitForBusy();
	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	uint8_t data[1] = { FLASH_CMD_READ_FAST };
	spiSendBytes(data, 1);

	data[0] = (addr & 0xFF0000) >> 16;
	spiSendBytes(data, 1);
	data[0] = (addr & 0x00FF00) >> 8;
	spiSendBytes(data, 1);
	data[0] = (addr & 0x0000FF) >> 0;
	spiSendBytes(data, 1);
	data[0] = FLASH_CMD_DUMMY;
	spiSendBytes(data, 1);

	spiRecvBytes(buff, size);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

	flashWaitForBusy();

//	flashWaitForWriteEnd(0);
}
