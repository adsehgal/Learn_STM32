/*
 * config.c
 *
 *  Created on: Mar 16, 2022
 *      Author: adityasehgal
 */

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_gpio.h"

#include "config.h"

#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif	//NVIC_PRIORITYGROUP_0

#define LED_PIN LL_GPIO_PIN_5
#define LED_PORT GPIOA

uint32_t systick = 0;

void SysTick_Handler(void) {
	systick++;
}

static void clockConfig(void) {
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
	while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2)
		;
	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);

	LL_RCC_HSI_SetCalibTrimming(16);
	LL_RCC_HSI_Enable();
	while (!LL_RCC_HSI_IsReady())
		;
	LL_RCC_LSI_Enable();
	while (!LL_RCC_LSI_IsReady())
		;

	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_8, 84,
	LL_RCC_PLLP_DIV_2);
	LL_RCC_PLL_Enable();
	while (!LL_RCC_PLL_IsReady())
		;

	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
	while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
		;

	LL_Init1msTick(84000000);
	LL_SetSystemCoreClock(84000000);
	LL_RCC_SetTIMPrescaler(LL_RCC_TIM_PRESCALER_TWICE);

}

void config_systemSetup(void) {
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	NVIC_SetPriority(SysTick_IRQn,
			NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));

	clockConfig();

	SysTick_Config(SystemCoreClock / 1000);

//	NVIC_EnableIRQ(SysTick_IRQn);
}
