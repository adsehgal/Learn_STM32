/*
 * main.c
 *
 *  Created on: Mar 16, 2022
 *      Author: adityasehgal
 */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void) {
	while (1) {
	}
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void) {
	while (1) {
	}
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void) {
	while (1) {
	}
}

/**
 * @brief This function handles Pre-fetch fault, memory access fault.
 */
void BusFault_Handler(void) {
	while (1) {
	}
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void) {
	while (1) {
	}
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void) {
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void) {
}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void) {
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

