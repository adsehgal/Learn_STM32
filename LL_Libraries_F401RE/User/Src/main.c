/*
 * main.c
 *
 *  Created on: Mar 16, 2022
 *      Author: adityasehgal
 */

#include <stdio.h>

#include "stm32f4xx_ll_utils.h"

#include "config.h"
#include "uart.h"
#include "spi.h"

#define SER_COMMS_ARR_LEN 255
uint8_t spiWrite[SER_COMMS_ARR_LEN] = { 0 };
uint8_t spiRead[SER_COMMS_ARR_LEN] = { 0 };

int main(void) {
	config_systemSetup();
	uart_init();
	spi_init();

	for (uint8_t i = 0; i< SER_COMMS_ARR_LEN; i++) {
		spiWrite[i] = 255 - i;
	}

	for (;;) {
		printf("Hello World\n");
		spi_write(spiWrite, SER_COMMS_ARR_LEN);

		spi_read(spiRead, SER_COMMS_ARR_LEN);

		LL_mDelay(1);

	}
}
