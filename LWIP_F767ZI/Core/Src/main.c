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
#include "cmsis_os.h"
#include "lwip.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
#include <string.h>
#include "lwip/api.h"
#include "sockets.h"
#include "stdarg.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SERVER_IP1  192 //server ip address
#define SERVER_IP2  168
#define SERVER_IP3  1
#define SERVER_IP4  50
#define SERVER_PORT	1234 //server listen port
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

UART_HandleTypeDef huart3;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
const char *message = "Hello UDP message!\n\r";

#define TELNET_BUF_SIZE 512
// Telnet states
#define STATE_NORMAL 0
#define STATE_IAC    1
#define STATE_OPT    2
#define STATE_SB     3
#define STATE_OPTDAT 4
#define STATE_SE     5

#define TERM_UNKNOWN 0
#define TERM_CONSOLE 1
#define TERM_VT100   2

// Telnet special characters
#define TELNET_SE    240   // End of subnegotiation parameters
#define TELNET_NOP   241   // No operation
#define TELNET_MARK  242   // Data mark
#define TELNET_BRK   243   // Break
#define TELNET_IP    244   // Interrupt process
#define TELNET_AO    245   // Abort output
#define TELNET_AYT   246   // Are you there
#define TELNET_EC    247   // Erase character
#define TELNET_EL    248   // Erase line
#define TELNET_GA    249   // Go ahead
#define TELNET_SB    250   // Start of subnegotiation parameters
#define TELNET_WILL  251   // Will option code
#define TELNET_WONT  252   // Won't option code
#define TELNET_DO    253   // Do option code
#define TELNET_DONT  254   // Don't option code
#define TELNET_IAC   255   // Interpret as command

// Telnet options
#define TELOPT_TRANSMIT_BINARY      0  // Binary Transmission (RFC856)
#define TELOPT_ECHO                 1  // Echo (RFC857)
#define TELOPT_SUPPRESS_GO_AHEAD    3  // Suppress Go Ahead (RFC858)
#define TELOPT_STATUS               5  // Status (RFC859)
#define TELOPT_TIMING_MARK          6  // Timing Mark (RFC860)
#define TELOPT_NAOCRD              10  // Output Carriage-Return Disposition (RFC652)
#define TELOPT_NAOHTS              11  // Output Horizontal Tab Stops (RFC653)
#define TELOPT_NAOHTD              12  // Output Horizontal Tab Stop Disposition (RFC654)
#define TELOPT_NAOFFD              13  // Output Formfeed Disposition (RFC655)
#define TELOPT_NAOVTS              14  // Output Vertical Tabstops (RFC656)
#define TELOPT_NAOVTD              15  // Output Vertical Tab Disposition (RFC657)
#define TELOPT_NAOLFD              16  // Output Linefeed Disposition (RFC658)
#define TELOPT_EXTEND_ASCII        17  // Extended ASCII (RFC698)
#define TELOPT_TERMINAL_TYPE       24  // Terminal Type (RFC1091)
#define TELOPT_NAWS                31  // Negotiate About Window Size (RFC1073)
#define TELOPT_TERMINAL_SPEED      32  // Terminal Speed (RFC1079)
#define TELOPT_TOGGLE_FLOW_CONTROL 33  // Remote Flow Control (RFC1372)
#define TELOPT_LINEMODE            34  // Linemode (RFC1184)
#define TELOPT_AUTHENTICATION      37  // Authentication (RFC1416)

struct term {
	int type;
	int cols;
	int lines;
};

struct telbuf {
	unsigned char data[TELNET_BUF_SIZE];
	unsigned char *start;
	unsigned char *end;
};
struct termstate {
	int sock;
	int state;
	int code;
	unsigned char optdata[256];
	int optlen;
	struct term term;
	struct telbuf bi;
	struct telbuf bo;
};
static struct termstate tstate;
struct termstate *ts = &tstate;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
void StartDefaultTask(void const *argument);

/* USER CODE BEGIN PFP */
static int telnet_write(char *str, int size, bool flush) {
	int i;
	int n;
	int len;
	fd_set data_write;
	struct timeval tv;
	unsigned char *ptr = (unsigned char*) str;

	n = 0;
	do {
		len = TELNET_BUF_SIZE - (int) (ts->bo.start - ts->bo.data);
		if (size < len)
			len = size;
		if (len >= 0) {
			memcpy(ts->bo.start, str, len);
			size -= len;
			ts->bo.start += len;
			str += len;
		}
		if (((int) (ts->bo.start - ts->bo.data) >= TELNET_BUF_SIZE) || flush) {
			len = n = 0;
			ptr = ts->bo.data;
			do {
				FD_ZERO(&data_write);
				FD_SET(ts->sock, &data_write);
				tv.tv_sec = 1;
				tv.tv_usec = 0;
				i = select(FD_SETSIZE, NULL, &data_write, NULL, &tv);
				if ((i > 0) && (FD_ISSET(ts->sock, &data_write) != 0)) {
					n = send(ts->sock, ptr,
							(int ) (ts->bo.start - ts->bo.data) - len, 0);
					if (n >= 0) {
						ptr += n;
						len += n;
					}
				}
			} while ((n >= 0) && (len < (int) (ts->bo.start - ts->bo.data))
					&& (i > 0));
			ts->bo.start = ts->bo.data;
		}
	} while (size > 0);

	return n < 0 ? n : len;
}

static void telnet_putc(char c) {
	if (c == '\n')
		telnet_putc('\r');
	if (ts->sock > 0)
		telnet_write(&c, 1, (c == '\n') ? true : false);
}

void telnet_puts(char *str) {
	// Send each character in the string.
	while (*str)
		telnet_putc(*(str++));
}

void telnet_printf(const char *fmt, ...) {
	va_list arp;
	char buffer[128];

	va_start(arp, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, arp);
	va_end(arp);

	telnet_puts(buffer);
}

static void telnet_sendopt(struct termstate *ts, int code, int option) {
	unsigned char buf[3];
	buf[0] = TELNET_IAC;
	buf[1] = (unsigned char) code;
	buf[2] = (unsigned char) option;
	send(ts->sock, buf, 3, 0);
}

// Runs the shell by printing the prompt and processing each shell command typed in.
void shell_process() {
//	char cmdbuf[64];
//	char uptime_str[32];

// Display version information.

//	telnet_printf("%s\n", 2.2);
//	telnet_printf("%s\n", "F2");
	telnet_printf("Hello\n\n");
//	// Display the "sysinfo" kept on the file system with unique information for each robot.
//	shell_ouput_file("sysinfo");
//
//	if (health_get_watchdog_reset()) {
//		telnet_printf("\n*** BOARD WAS RESET BY WATCHDOG ***\n");
//	}
//	// Send a prompt.
//	telnet_puts("> ");
//	telnet_flush();

// Get a string.
//	while (telnet_gets(cmdbuf, sizeof(cmdbuf), 0, true)) {
//		// Process the line as a command.
//		if (!shell_command(cmdbuf))
//			break;
//
//		// Send a prompt.
//		telnet_puts("> ");
//		telnet_flush();
//	}
}

static void telnet_shell(int s) {
	int off;

	// TCP_NODELAY is used to disable the Nagle buffering algorithm. It
	// should only be set for applications such as telnet that send frequent
	// small bursts of information without getting an immediate response,
	// where timely delivery of data is required.
	off = 0;
	setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char* ) &off, sizeof(off));

	// Initialize terminal state.
	memset(ts, 0, sizeof(struct termstate));
	ts->sock = s;
	ts->state = STATE_NORMAL;
	ts->term.type = TERM_VT100;
	ts->term.cols = 80;
	ts->term.lines = 25;

	// Send initial option that we'll echo on this side.
//	telnet_sendopt(ts, TELNET_WILL, TELOPT_ECHO);

	// Reset the output buffer.
	ts->bo.start = ts->bo.data;

	// Run the shell.
	shell_process();

	// Close the socket.
	if (ts->sock >= 0) {
		close(ts->sock);
		ts->sock = -1;
	}

}

void printMsg(const char *fmt, ...) {
	va_list arp;
	char buffer[128];

	va_start(arp, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, arp);
	va_end(arp);

	HAL_UART_Transmit(&huart3, (uint8_t*) buffer, strlen(buffer),
			HAL_MAX_DELAY);
}
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
	/* USER CODE BEGIN 2 */
	printMsg("Starting LWIP\n");
	/* USER CODE END 2 */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* definition and creation of defaultTask */
	osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 256);
	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

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

	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();
	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 96;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
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
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3;
	PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
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
	huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
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
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LD1_Pin | LD3_Pin | LD2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : USER_Btn_Pin */
	GPIO_InitStruct.Pin = USER_Btn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
	GPIO_InitStruct.Pin = LD1_Pin | LD3_Pin | LD2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const *argument) {
	/* init code for LWIP */
	MX_LWIP_Init();
	/* USER CODE BEGIN 5 */
	int sock, newconn, size;
	struct sockaddr_in address, remotehost;

	// Create a TCP socket.
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printMsg("TELNET: cannot create socket");
		return;
	}

	// Bind to port 23 at any interface.
	address.sin_family = AF_INET;
	address.sin_port = htons(23);
	address.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0) {
		printMsg("TELNET: cannot bind socket");
		return;
	}

	// Listen for incoming connections (TCP listen backlog = 1).
	listen(sock, 1);

	size = sizeof(remotehost);

	/* Infinite loop */
	for (;;) {
		newconn = accept(sock, (struct sockaddr* )&remotehost,
				(socklen_t* )&size);
		if (newconn > 0)
			telnet_shell(newconn);
//		telnet_printf("Hello\n");
//		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin); //toggle running led
//		osDelay(500);
	}
	/* USER CODE END 5 */
}

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
