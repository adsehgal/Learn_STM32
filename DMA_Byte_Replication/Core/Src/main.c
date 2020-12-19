/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "stdarg.h"

const int DATA_SIZE = 4096;	//can not use #define to declare array size?


UART_HandleTypeDef huart2;
DMA_HandleTypeDef myDMA;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);

//Custom Functions
void printmsg(char *format,...);


int main(void)
{
	uint8_t srcData[DATA_SIZE];
	uint8_t dstData[DATA_SIZE];
	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();
	MX_DMA_Init();
	MX_USART2_UART_Init();

	for(int i = 0; i < DATA_SIZE; i++){
		srcData[i] = i%DATA_SIZE;
		dstData[i] = 0;
	}

	//&myDMA = DMA handle
	//HAL_DMA_XFER_CPLT_CB_ID = Interrupt on full transfer
	//	Found in stm32fxx_hal_dma.h -> HAL_DMA_CallbackIDTypeDef enum
//	HAL_DMA_RegisterCallback(&myDMA, HAL_DMA_XFER_CPLT_CB_ID, &DMA_ISR);

	HAL_DMA_Start(&myDMA, (uint32_t)srcData, (uint32_t)dstData, sizeof(srcData));

	printmsg("\n\nStarting DMA transfer\n\n");

	while (1)
	{
		printmsg("waiting for ISR...\n\n");
		HAL_StatusTypeDef DMA_Status = HAL_DMA_PollForTransfer(&myDMA, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
		if(DMA_Status == HAL_OK)
		{
			printmsg("| ");
			for(int i = 0; i < DATA_SIZE; i++){
				if(i%16 == 0 && i){
					printmsg("\n| ");
				}
				printmsg("0x%02X | ", dstData[i]);
			}
			while(1);
			printmsg("\n\n");
		}

	}
  /* USER CODE END 3 */
}

void printmsg(char *format,...) {
    char str[80];

    /*Extract the the argument list using VA apis */
    va_list args;
    va_start(args, format);
    vsprintf(str, format,args);
    HAL_UART_Transmit(&huart2,(uint8_t *)str, strlen(str),HAL_MAX_DELAY);
    va_end(args);
}

/**
 * ------------------------------------------------------------------------
 * -----------------------------SYS FUNCS----------------------------------
 * ------------------------------------------------------------------------
 */

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  * Configure DMA for memory to memory transfers
  *   myDMA
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* Configure DMA request myDMA on DMA2_Stream0 */
  myDMA.Instance = DMA2_Stream0;
  myDMA.Init.Channel = DMA_CHANNEL_0;
  myDMA.Init.Direction = DMA_MEMORY_TO_MEMORY;
  myDMA.Init.PeriphInc = DMA_PINC_ENABLE;
  myDMA.Init.MemInc = DMA_MINC_ENABLE;
  myDMA.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  myDMA.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  myDMA.Init.Mode = DMA_NORMAL;
  myDMA.Init.Priority = DMA_PRIORITY_LOW;
  myDMA.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
  myDMA.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  myDMA.Init.MemBurst = DMA_MBURST_SINGLE;
  myDMA.Init.PeriphBurst = DMA_PBURST_SINGLE;
  if (HAL_DMA_Init(&myDMA) != HAL_OK)
  {
    Error_Handler( );
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/