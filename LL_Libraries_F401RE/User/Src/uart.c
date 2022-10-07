/*
 * uart.c
 *
 *  Created on: 16 Mar 2022
 *      Author: adityasehgal
 */

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_usart.h"

#include "uart.h"

#define UART_BASE		USART2
#define UART_RX_PIN		LL_GPIO_PIN_3
#define UART_TX_PIN		LL_GPIO_PIN_2
#define UART_PORT		GPIOA
#define UART_GPIO_ALT	LL_GPIO_AF_7
#define UART_BAUD		115200

int _write(int file, char *ptr, int len) {
	for (int i = 0; i < len; i++) {
		LL_USART_TransmitData8(UART_BASE, (uint8_t) ptr[i]);
		while (!LL_USART_IsActiveFlag_TXE(UART_BASE))
			;
	}
	return len;
}

void uart_init(void) {
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

	LL_GPIO_InitTypeDef gpio = { 0 };
	LL_USART_InitTypeDef uart = { 0 };

	gpio.Pin = UART_RX_PIN | UART_TX_PIN;
	gpio.Mode = LL_GPIO_MODE_ALTERNATE;
	gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	gpio.Pull = LL_GPIO_PULL_NO;
	gpio.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	gpio.Alternate = UART_GPIO_ALT;
	LL_GPIO_Init(UART_PORT, &gpio);

	uart.BaudRate = UART_BAUD;
	uart.DataWidth = LL_USART_DATAWIDTH_8B;
	uart.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	uart.OverSampling = LL_USART_OVERSAMPLING_16;
	uart.Parity = LL_USART_PARITY_NONE;
	uart.StopBits = LL_USART_STOPBITS_1;
	uart.TransferDirection = LL_USART_DIRECTION_TX_RX;
	LL_USART_Init(UART_BASE, &uart);
	LL_USART_ConfigAsyncMode(UART_BASE);
	LL_USART_Enable(UART_BASE);

}
