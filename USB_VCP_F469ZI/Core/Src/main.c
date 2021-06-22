/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifdef __GNUC__	//retarget printf to uart3
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
usbDatum usbStruct;	//defined in usbd_cdc_if.h
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART3_UART_Init();
	MX_USB_DEVICE_Init();
	/* USER CODE BEGIN 2 */
	HAL_GPIO_WritePin(LEDG_GPIO_Port, LEDG_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LEDB_GPIO_Port, LEDB_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LEDO_GPIO_Port, LEDO_Pin, GPIO_PIN_SET);
	for (int i = 0; i < 10; i++) {
		HAL_GPIO_TogglePin(LEDG_GPIO_Port, LEDG_Pin);
		HAL_Delay(50);
	}
	HAL_Delay(1000);

//	sprintf((char*) usbStruct.txBuff,
//			"USB VCP Starting...\n	Compiled on: %s %s\n\n",
//			__DATE__, __TIME__);
	printf("USB VCP Starting...\n	Compiled on: %s %s\n\n",
	__DATE__, __TIME__);

//	printf((char*) usbStruct.txBuff);
//	CDC_Transmit_FS(usbStruct.txBuff, sizeof(usbStruct.txBuff));

	sprintf((char*) usbStruct.rxBuff, "help");
	uiStringHandler();

	usbStruct.rxLastIndex = 0;
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		if (usbStruct.intFlag == 1) {
			uiStringHandler();
			HAL_GPIO_TogglePin(LEDG_GPIO_Port, LEDG_Pin);
//			CDC_Transmit_FS(usbStruct.rxBuff, (uint16_t) usbStruct.rxLen);
//			printf("%s\n", usbStruct.rxBuff);
			usbStruct.intFlag = 0;
			memset(usbStruct.rxBuff, '\0', usbStruct.rxLen);
			usbStruct.rxLastIndex = 0;
		}
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 180;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 6;
	RCC_OscInitStruct.PLL.PLLR = 6;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Activate the Over-Drive mode
	 */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
	PeriphClkInitStruct.PLLSAI.PLLSAIN = 144;
	PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV6;
	PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLSAIP;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief USART3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART3_UART_Init(void) {

	/* USER CODE BEGIN USART3_Init 0 */

	/* USER CODE END USART3_Init 0 */

	/* USER CODE BEGIN USART3_Init 1 */

	/* USER CODE END USART3_Init 1 */
	huart3.Instance = USART3;
	huart3.Init.BaudRate = 115200;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart3) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART3_Init 2 */

	/* USER CODE END USART3_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOK_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, LEDR_Pin | LEDO_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LEDB_GPIO_Port, LEDB_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LEDG_GPIO_Port, LEDG_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(OTG_FS1_PowerSwitchOn_GPIO_Port,
	OTG_FS1_PowerSwitchOn_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : OTG_FS1_OverCurrent_Pin */
	GPIO_InitStruct.Pin = OTG_FS1_OverCurrent_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(OTG_FS1_OverCurrent_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LEDR_Pin LEDO_Pin */
	GPIO_InitStruct.Pin = LEDR_Pin | LEDO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pin : LEDB_Pin */
	GPIO_InitStruct.Pin = LEDB_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LEDB_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : LEDG_Pin */
	GPIO_InitStruct.Pin = LEDG_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LEDG_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : OTG_FS1_PowerSwitchOn_Pin */
	GPIO_InitStruct.Pin = OTG_FS1_PowerSwitchOn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(OTG_FS1_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
PUTCHAR_PROTOTYPE {	//retarget printf to uart3 AND USB VCP
	//UART printf for debugging
	CDC_Transmit_FS((uint8_t*) &ch, 1);
	return HAL_UART_Transmit(&huart3, (uint8_t*) &ch, 1, HAL_MAX_DELAY);
}

void uiStringHandler(void) {
	char *token;
//	printf("recv: %s\n", (char*) usbStruct.rxBuff);
	token = strtok((char*) usbStruct.rxBuff, " ");
	if (token != NULL) {
//		HELP CHECK
		if (!strcasecmp("help", (char*) usbStruct.rxBuff)) {
			token = strtok(NULL, " ");
			if (token != NULL) {
				printf("INVALID COMMAND!!\n\n");
			} else {
				printf("List of commands:\n");
				printf("\t\"HELP\" : list commands\n");
				printf("\t\"WHOAMI\" : Name of developer\n");
				printf("\t\"CDATE\" : Date of compilation\n");
				printf("\t\"CTIME\" : Time of compilation\n");
				printf("\t\"HALV\" : HAL version\n"); //HAL_GetHalVersion()
				printf("\t\"DEVID\" : Device ID\n\n"); //HAL_GetDEVID()
			}
		}
//		WHOAMI CHECK
		else if (!strcasecmp("WHOAMI", (char*) usbStruct.rxBuff)) {
			token = strtok(NULL, " ");
			if (token != NULL) {
				printf("INVALID COMMAND!!\n\n");
			} else {
				printf("\tI am Aditya Sehgal!\n\n");
			}
		}

//		CDATE CHECK
		else if (!strcasecmp("CDATE", (char*) usbStruct.rxBuff)) {
			token = strtok(NULL, " ");
			if (token != NULL) {
				printf("INVALID COMMAND!!\n\n");
			} else {
				printf("\tThis program was compiled on %s\n\n", __DATE__);
			}
		}

//		CTIME CHECK
		else if (!strcasecmp("CTIME", (char*) usbStruct.rxBuff)) {
			token = strtok(NULL, " ");
			if (token != NULL) {
				printf("INVALID COMMAND!!\n\n");
			} else {
				printf("\tThis program was compiled at %s\n\n", __TIME__);
			}
		}

//		HALV CHECK
		else if (!strcasecmp("HALV", (char*) usbStruct.rxBuff)) {
			token = strtok(NULL, " ");
			if (token != NULL) {
				printf("INVALID COMMAND!!\n\n");
			} else {
				printf("\tThis program is using HAL V: %ld\n\n",
						HAL_GetHalVersion());
			}
		}

//		HALV CHECK
		else if (!strcasecmp("DEVID", (char*) usbStruct.rxBuff)) {
			token = strtok(NULL, " ");
			if (token != NULL) {
				printf("INVALID COMMAND!!\n\n");
			} else {
				printf("\tDevice ID: %ld\n\n", HAL_GetDEVID());
			}
		}

//		Invalid command
		else {
			printf("INVALID COMMAND!!\n\n");
		}
	} else {
		printf("INVALID COMMAND!!\n\n");
	}
}
/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM6 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM6) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
		HAL_GPIO_TogglePin(LEDR_GPIO_Port, LEDR_Pin);
		HAL_Delay(50);
	}
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
