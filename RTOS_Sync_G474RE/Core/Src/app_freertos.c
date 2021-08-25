/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : app_freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern UART_HandleTypeDef hlpuart1;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
uint32_t timerCounter = 0;
/* USER CODE END Variables */
osThreadId idleTaskHandle;
osMessageQId queueTestHandle;
osTimerId timerTestHandle;
osMutexId mutexTestHandle;
osMutexId recursiveMutexTestHandle;
osSemaphoreId binarySemTestHandle;
osSemaphoreId countingSemTestHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

osThreadId printfTaskHandle;
void StartPrintfTask(void const *argument);

osThreadId binSemTaskHandle;
void StartBinSemTask(void const *argument);

/* USER CODE END FunctionPrototypes */

void StartIdleTask(void const *argument);
void timerTestCallback(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	printf("FreeRTOS Sync Starting\n");
	printf("\tBuilt on %s %s\n\n", __DATE__, __TIME__);

	/* USER CODE END Init */
	/* Create the mutex(es) */
	/* definition and creation of mutexTest */
	osMutexDef(mutexTest);
	mutexTestHandle = osMutexCreate(osMutex(mutexTest));

	/* Create the recursive mutex(es) */
	/* definition and creation of recursiveMutexTest */
	osMutexDef(recursiveMutexTest);
	recursiveMutexTestHandle = osRecursiveMutexCreate(
			osMutex(recursiveMutexTest));

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* Create the semaphores(s) */
	/* definition and creation of binarySemTest */
	osSemaphoreDef(binarySemTest);
	binarySemTestHandle = osSemaphoreCreate(osSemaphore(binarySemTest), 1);

	/* definition and creation of countingSemTest */
	osSemaphoreDef(countingSemTest);
	countingSemTestHandle = osSemaphoreCreate(osSemaphore(countingSemTest), 5);

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* Create the timer(s) */
	/* definition and creation of timerTest */
	osTimerDef(timerTest, timerTestCallback);
	timerTestHandle = osTimerCreate(osTimer(timerTest), osTimerPeriodic, NULL);

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* Create the queue(s) */
	/* definition and creation of queueTest */
	osMessageQDef(queueTest, 16, uint16_t);
	queueTestHandle = osMessageCreate(osMessageQ(queueTest), NULL);

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* definition and creation of idleTask */

	osThreadDef(idleTask, StartIdleTask, osPriorityNormal, 0, 128);
	idleTaskHandle = osThreadCreate(osThread(idleTask), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */

	osThreadDef(printfTask, StartPrintfTask, osPriorityNormal, 0, 128);
	printfTaskHandle = osThreadCreate(osThread(printfTask), NULL);

	osThreadDef(binSemTask, StartBinSemTask, osPriorityNormal, 0, 128);
	binSemTaskHandle = osThreadCreate(osThread(binSemTask), NULL);
	/* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartIdleTask */
/**
 * @brief  Function implementing the idleTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartIdleTask */
void StartIdleTask(void const *argument) {
	/* USER CODE BEGIN StartIdleTask */

	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	osTimerStart(timerTestHandle, 500);

	/* Infinite loop */
	for (;;) {
		osDelay(1);
	}
	/* USER CODE END StartIdleTask */
}

/* timerTestCallback function */
void timerTestCallback(void const *argument) {
	/* USER CODE BEGIN timerTestCallback */
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	char helloStr[128] = { 0 };
	sprintf(helloStr, "Timer Print %lu\n", timerCounter++);
//	HAL_UART_Transmit(&hlpuart1, (uint8_t*) helloStr, sizeof(helloStr),
//	HAL_MAX_DELAY);
//	osMessagePut(queueTestHandle, timerCounter, HAL_MAX_DELAY);
	/* USER CODE END timerTestCallback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void StartPrintfTask(void const *argument) {
	char printStr[256];
	for (;;) {
//		osEvent msg = osMessageGet(queueTestHandle, HAL_MAX_DELAY);
//		uint32_t huh = msg.value.v;
//		printf("queue = %lu\n", huh);
		osDelay(1);
	}
}

void StartBinSemTask(void const *argument) {

	for (;;) {
//		osSemaphoreWait(binarySemTestHandle, 0);
//		printf("Binary Semaphore Acquired\n\n");

//		osSemaphoreWait(binarySemTestHandle, HAL_MAX_DELAY);

	}
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
