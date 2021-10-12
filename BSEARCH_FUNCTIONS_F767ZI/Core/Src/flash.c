/*
 * flash.c
 *
 *  Created on: Oct 11, 2021
 *      Author: root
 */

#include <flash.h>
#include <stdio.h>
#include "main.h"

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
	HAL_SPI_DeInit(HANDLE_SPI);

	*HANDLE_SPI.Instance = SPI1;
	*HANDLE_SPI.Init.Mode = SPI_MODE_MASTER;
	*HANDLE_SPI.Init.Direction = SPI_DIRECTION_2LINES;
	*HANDLE_SPI.Init.DataSize = SPI_DATASIZE_8BIT;
	*HANDLE_SPI.Init.CLKPolarity = SPI_POLARITY_LOW;
	*HANDLE_SPI.Init.CLKPhase = SPI_PHASE_1EDGE;
	*HANDLE_SPI.Init.NSS = SPI_NSS_SOFT;
	*HANDLE_SPI.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	*HANDLE_SPI.Init.FirstBit = SPI_FIRSTBIT_MSB;
	*HANDLE_SPI.Init.TIMode = SPI_TIMODE_DISABLE;
	*HANDLE_SPI.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	*HANDLE_SPI.Init.CRCPolynomial = 7;
	*HANDLE_SPI.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	*HANDLE_SPI.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
	if (HAL_SPI_Init(HANDLE_SPI) != HAL_OK) {
		Error_Handler();
	}

//	actually send out the data now
#ifdef SPI_USE_DMA
	HAL_SPI_Transmit_DMA(HANDLE_SPI, data, size);
	while (!spi.txComp)
		;
	spi.txComp = 0;
#else
	HAL_SPI_Transmit(HANDLE_SPI, data, size, 500);
#endif
}

static void spiRecvBytes(uint8_t *data, uint32_t size) {

//	the flash chip requires we read out data on the falling clock edges
//	so we make sure SPI is setup to do that
	HAL_SPI_DeInit(HANDLE_SPI);

	*HANDLE_SPI.Instance = SPI1;
	*HANDLE_SPI.Init.Mode = SPI_MODE_MASTER;
	*HANDLE_SPI.Init.Direction = SPI_DIRECTION_2LINES;
	*HANDLE_SPI.Init.DataSize = SPI_DATASIZE_8BIT;
	*HANDLE_SPI.Init.CLKPolarity = SPI_POLARITY_LOW;
	*HANDLE_SPI.Init.CLKPhase = SPI_PHASE_2EDGE;
	*HANDLE_SPI.Init.NSS = SPI_NSS_SOFT;
	*HANDLE_SPI.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	*HANDLE_SPI.Init.FirstBit = SPI_FIRSTBIT_MSB;
	*HANDLE_SPI.Init.TIMode = SPI_TIMODE_DISABLE;
	*HANDLE_SPI.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	*HANDLE_SPI.Init.CRCPolynomial = 7;
	*HANDLE_SPI.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	*HANDLE_SPI.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
	if (HAL_SPI_Init(HANDLE_SPI) != HAL_OK) {
		Error_Handler();
	}
//	actually read out the data now
#ifdef SPI_USE_DMA
	HAL_SPI_Receive_DMA(HANDLE_SPI, data, size);
	while (!spi.rxComp)
		;
	spi.rxComp = 0;
#else
	HAL_SPI_Receive(HANDLE_SPI, data, size, 500);
#endif

}

static void flashWriteEnable(void) {

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	uint8_t data[1] = { FLASH_CMD_WREN };
	spiSendBytes(data, 1);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

}

static void flashWriteDisable(void) {

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	uint8_t data[1] = { FLASH_CMD_WRDIS };
	spiSendBytes(data, 1);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

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

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	uint8_t data[1] = { FLASH_CMD_READ_STATUS_REG };
	spiSendBytes(data, 1);

	uint8_t status = 0;

	do {
		spiRecvBytes(&status, 1);
		SYS_DELAY(1);
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

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	uint8_t data[1] = { FLASH_CMD_READ_STATUS_REG };
	spiSendBytes(data, 1);

	uint8_t status = 0;

	do {
		spiRecvBytes(&status, 1);
		SYS_DELAY(1);
	} while ((status & 0x01));

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

	if (size < FLASH_ATTR_PAGE_SIZE) {
		size = FLASH_ATTR_PAGE_SIZE;
	}

	for (uint32_t i = 0; i < size / FLASH_ATTR_PAGE_SIZE; i++) {

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

		for (int j = 0; j < FLASH_ATTR_PAGE_SIZE; j++) {
			spiSendBytes((uint8_t*) &buff[j + (FLASH_ATTR_PAGE_SIZE * i)], 1);
		}

		HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);

		flashWaitForWriteEnd(0);
		address += FLASH_ATTR_PAGE_SIZE;

	}
}

void flashWriteSector(uint8_t *buff, uint8_t sectorAddr, uint8_t numSectors) {
//	pages to write = numsectors*sectorsize/pagesize
//	uint32_t pagesToWrite = (numSectors * FLASH_ATTR_SECTOR_SIZE)
//			/ FLASH_ATTR_PAGE_SIZE;
//	for (int i = 0; i < pagesToWrite; i++) {
//		flashWriteBytes(buff, sectorAddr, 1);
//		buff += FLASH_ATTR_PAGE_SIZE;
//		sectorAddr += FLASH_ATTR_PAGE_SIZE;
//		printf("Written page# %d\n", i);
//	}

	for (int i = 0;
			i < numSectors * (FLASH_ATTR_SECTOR_SIZE / FLASH_ATTR_PAGE_SIZE);
			i++) {
		flashWriteBytes(buff, sectorAddr, 1);
		sectorAddr += FLASH_ATTR_PAGE_SIZE;
		buff += FLASH_ATTR_PAGE_SIZE;
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
}

void flashReadSector(uint8_t *buff, uint32_t sector) {
	flashWaitForBusy();
	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);

	sector *= FLASH_ATTR_SECTOR_SIZE;
	uint8_t data[1] = { FLASH_CMD_READ_FAST };
	data[0] = (sector & 0xFF0000) >> 16;
	spiSendBytes(data, 1);
	data[0] = (sector & 0x00FF00) >> 8;
	spiSendBytes(data, 1);
	data[0] = (sector & 0x0000FF) >> 0;
	spiSendBytes(data, 1);
	data[0] = FLASH_CMD_DUMMY;
	spiSendBytes(data, 1);

	spiRecvBytes(buff, FLASH_ATTR_SECTOR_SIZE);

	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);
	flashWaitForBusy();
}
