/*
 * spi.c
 *
 *  Created on: 16 Mar 2022
 *      Author: adityasehgal
 */

#include <stddef.h>

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_dma.h"

#include "spi.h"

#define USE_DMA 1

/**
 * SPI1 on APB2 running at sys freq = 84MHz
 * SPI1 can be on DMA2 Channel3 RX on Stream0/2 and TX on Stream3/5 DMA2 on AHB1
 * SPI1_SCK		--> PB3
 * SPI1_MISO	--> PB4
 * SPI1_MOSI	--> PB5
 * SPI1_CS		--> PB10	// Hardware CS/NSS signal, PB10 selected due to proximity to other pins on the Nucleo Board
 */

#define SPI_BASE		SPI1
#define SPI_SCK_PIN		LL_GPIO_PIN_3
#define SPI_MISO_PIN	LL_GPIO_PIN_4
#define SPI_MOSI_PIN	LL_GPIO_PIN_5
#define SPI_CS_PIN		LL_GPIO_PIN_10
#define SPI_PORT		GPIOB
#define SPI_GPIO_ALT	LL_GPIO_AF_5

#if USE_DMA
#define DMA_BASE DMA2
#define DMA_CHANNEL LL_DMA_CHANNEL_3
#define DMA_STREAM_RX LL_DMA_STREAM_0
#define DMA_STREAM_TX LL_DMA_STREAM_3

static uint8_t rxFlag = 0;
static uint8_t txFlag = 0;
#endif

#define DESELECT_CHIP() LL_GPIO_SetOutputPin(SPI_PORT, SPI_CS_PIN);
#define SELECT_CHIP() LL_GPIO_ResetOutputPin(SPI_PORT, SPI_CS_PIN);

#if USE_DMA

void configDMA(void) {
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

	// RX DMA Interrupt enable
	NVIC_SetPriority(DMA2_Stream0_IRQn,
			NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
	NVIC_EnableIRQ(DMA2_Stream0_IRQn);

	// TX DMA Interrupt enable
	NVIC_SetPriority(DMA2_Stream3_IRQn,
			NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
	NVIC_EnableIRQ(DMA2_Stream3_IRQn);

	// General DMA setup for RX and TX
	LL_DMA_InitTypeDef dma = { 0 };
	dma.Channel = DMA_CHANNEL;
	dma.FIFOMode = LL_DMA_FIFOMODE_DISABLE;
	dma.FIFOThreshold = LL_DMA_FIFOTHRESHOLD_FULL;
	dma.MemBurst = LL_DMA_MBURST_SINGLE;
	dma.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
	dma.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
	dma.Mode = LL_DMA_MODE_NORMAL;
	dma.PeriphBurst = LL_DMA_PBURST_SINGLE;
	dma.PeriphOrM2MSrcAddress = LL_SPI_DMA_GetRegAddr(SPI_BASE);
	dma.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
	dma.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
	dma.Priority = LL_DMA_PRIORITY_HIGH;

	// Place holder for TX DMA setup
	// Actual address setup happens in the write function
	dma.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
	dma.MemoryOrM2MDstAddress = 0;
	dma.NbData = 0;
	LL_DMA_Init(DMA_BASE, DMA_STREAM_TX, &dma);

	// Place holder for RX DMA setup
	// Actual address setup happens in the read function
	dma.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
	dma.MemoryOrM2MDstAddress = 0;
	dma.NbData = 0;
	LL_DMA_Init(DMA_BASE, DMA_STREAM_RX, &dma);

}

// RX IRQ
void DMA2_Stream0_IRQHandler(void) {
	if (LL_DMA_IsActiveFlag_TC0(DMA_BASE)) {
		if (LL_DMA_IsEnabledIT_TC(DMA_BASE, DMA_STREAM_RX)) {
			rxFlag = 1;
			LL_DMA_ClearFlag_TC0(DMA_BASE);
			LL_SPI_DisableDMAReq_RX(SPI_BASE);
		}
	}
}

// TX IRQ
void DMA2_Stream3_IRQHandler(void) {
	if (LL_DMA_IsActiveFlag_TC3(DMA_BASE)) {
		if (LL_DMA_IsEnabledIT_TC(DMA_BASE, DMA_STREAM_TX)) {
			txFlag = 1;
			LL_DMA_ClearFlag_TC3(DMA_BASE);
			LL_SPI_DisableDMAReq_TX(SPI_BASE);
		}
	}
}

#endif

/**
 * @fn void writeInternal(uint8_t*, uint32_t)
 * @brief		Internal SPI write function
 * @param buf	Pointer to array to write to SPI
 * @param size	Number of elements to write
 */
static void writeInternal(uint8_t *buf, uint32_t size) {
#if USE_DMA
	LL_SPI_EnableDMAReq_TX(SPI_BASE);

	LL_DMA_ConfigAddresses(DMA_BASE, DMA_STREAM_TX, (uint32_t) buf,
			LL_SPI_DMA_GetRegAddr(SPI_BASE),
			LL_DMA_GetDataTransferDirection(DMA_BASE, DMA_STREAM_TX));

	LL_DMA_SetDataLength(DMA_BASE, DMA_STREAM_TX, size);

	LL_DMA_EnableIT_TC(DMA_BASE, DMA_STREAM_TX);

	LL_SPI_Enable(SPI_BASE);
	LL_DMA_EnableStream(DMA_BASE, DMA_STREAM_TX);

	while (!txFlag)
		;
	txFlag = 0;
#else
	for (uint32_t i = 0; i < size; i++) {
		LL_SPI_TransmitData8(SPI_BASE, buf[i]);
		while (!LL_SPI_IsActiveFlag_TXE(SPI_BASE))
			;
	}
#endif
}

/**
 * @fn void readInternal(uint8_t*, uint32_t)
 * @brief		Internal SPI read function
 * @param buf	Pointer to array where read values will be stored
 * @param size	Number of elements to be read
 */
static void readInternal(uint8_t *buf, uint32_t size) {
#if USE_DMA

	LL_SPI_EnableDMAReq_RX(SPI_BASE);

	LL_DMA_ConfigAddresses(DMA_BASE, DMA_STREAM_RX, (uint32_t) buf,
			LL_SPI_DMA_GetRegAddr(SPI_BASE),
			LL_DMA_GetDataTransferDirection(DMA_BASE, DMA_STREAM_RX));

	LL_DMA_SetDataLength(DMA_BASE, DMA_STREAM_RX, size);

	LL_DMA_EnableIT_TC(DMA_BASE, DMA_STREAM_RX);

	LL_SPI_Enable(SPI_BASE);
	LL_DMA_EnableStream(DMA_BASE, DMA_STREAM_RX);

	while (!rxFlag)
		;
	rxFlag = 0;

#else
	for (uint32_t i = 0; i < size; i++) {
		buf[i] = LL_SPI_ReceiveData8(SPI_BASE);
		while (!LL_SPI_IsActiveFlag_RXNE(SPI_BASE))
			;
	}
#endif
}

void spi_write(uint8_t *buf, uint32_t size) {

	SELECT_CHIP();
	writeInternal(buf, size);
	DESELECT_CHIP();

}

void spi_read(uint8_t *buf, uint32_t size) {

	SELECT_CHIP();
	readInternal(buf, size);
	DESELECT_CHIP();

}

void spi_readWrite(uint8_t *wbuf, uint32_t wSize, uint8_t *rbuf, uint32_t rSize) {
	SELECT_CHIP();
	writeInternal(wbuf, wSize);
	readInternal(rbuf, rSize);
	DESELECT_CHIP();
}

void spi_init(void) {

	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

	LL_GPIO_InitTypeDef gpio = { 0 };
	LL_SPI_InitTypeDef spi = { 0 };

// Common SPI GPIO Init
	gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	gpio.Pull = LL_GPIO_PULL_NO;
	gpio.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;

// SPI CS GPIO Init
	SELECT_CHIP();
	gpio.Pin = SPI_CS_PIN;
	gpio.Mode = LL_GPIO_MODE_OUTPUT;
	LL_GPIO_Init(SPI_PORT, &gpio);

// SPI GPIO Init
	gpio.Pin = SPI_SCK_PIN | SPI_MISO_PIN | SPI_MOSI_PIN;
	gpio.Mode = LL_GPIO_MODE_ALTERNATE;
	gpio.Alternate = SPI_GPIO_ALT;
	LL_GPIO_Init(SPI_PORT, &gpio);

	spi.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV4;
	spi.BitOrder = LL_SPI_MSB_FIRST;
	spi.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
	spi.CRCPoly = 0;
	spi.ClockPhase = LL_SPI_PHASE_1EDGE;
	spi.ClockPolarity = LL_SPI_POLARITY_HIGH;
	spi.DataWidth = LL_SPI_DATAWIDTH_8BIT;
	spi.Mode = LL_SPI_MODE_MASTER;
	spi.NSS = LL_SPI_NSS_SOFT;
	spi.TransferDirection = LL_SPI_FULL_DUPLEX;
	LL_SPI_Init(SPI_BASE, &spi);
	LL_SPI_SetStandard(SPI_BASE, LL_SPI_PROTOCOL_MOTOROLA);
#if USE_DMA
	configDMA();
#else
	LL_SPI_Enable(SPI_BASE);
#endif

}
