/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void uiStringHandler(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define OTG_FS1_OverCurrent_Pin GPIO_PIN_7
#define OTG_FS1_OverCurrent_GPIO_Port GPIOB
#define USB_FS1_P_Pin GPIO_PIN_12
#define USB_FS1_P_GPIO_Port GPIOA
#define LEDR_Pin GPIO_PIN_5
#define LEDR_GPIO_Port GPIOD
#define USB_FS1_N_Pin GPIO_PIN_11
#define USB_FS1_N_GPIO_Port GPIOA
#define LEDB_Pin GPIO_PIN_3
#define LEDB_GPIO_Port GPIOK
#define LEDO_Pin GPIO_PIN_4
#define LEDO_GPIO_Port GPIOD
#define LEDG_Pin GPIO_PIN_6
#define LEDG_GPIO_Port GPIOG
#define OTG_FS1_PowerSwitchOn_Pin GPIO_PIN_2
#define OTG_FS1_PowerSwitchOn_GPIO_Port GPIOB
#define WAKEUP_Pin GPIO_PIN_0
#define WAKEUP_GPIO_Port GPIOA
#define STLK_RX_Pin GPIO_PIN_10
#define STLK_RX_GPIO_Port GPIOB
#define STLK_TX_Pin GPIO_PIN_11
#define STLK_TX_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/