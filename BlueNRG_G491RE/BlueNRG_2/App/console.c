/******************** (C) COPYRIGHT 2020 STMicroelectronics ********************
 * File Name          : console.c
 * Author             : SRA
 * Version            : V1.0.0
 * Date               : 06-March-2020
 * Description        : Manage the console
 *******************************************************************************
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME.
 * AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
 * CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 ******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "console.h"
#include "central.h"
#include "stm32g4xx_nucleo.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define UartHandle hcom_uart[COM1]

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern savedDevices_t  saved_devices;
extern centralStatus_t central_status;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @brief  Sends a character to serial port
 *  @param  ch character to send
 *  @retval Character sent
 */
int Uart_Send_Char(int ch)
{
  HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

/** @brief  Receives a character from serial port
 *  @param  None
 *  @retval Character received
 */
int Uart_Receive_Char(void)
{
  uint8_t ch;
  HAL_UART_Receive(&UartHandle, &ch, 1, HAL_MAX_DELAY);

  /* Echo character back to console */
  HAL_UART_Transmit(&UartHandle, &ch, 1, HAL_MAX_DELAY);

  /* And cope with Windows */
  if(ch == '\r'){
    uint8_t ret = '\n';
    HAL_UART_Transmit(&UartHandle, &ret, 1, HAL_MAX_DELAY);
  }

  return ch;
}

/** @brief  Receives a character from serial port
 *  @param  Timeout
 *  @retval Character received
 */
int Uart_Receive_Char_Timeout(int timeout)
{
  uint8_t ch;
  HAL_UART_Receive(&UartHandle, &ch, 1, timeout);

  /* And cope with Windows */
  if(ch == '\r'){
    uint8_t ret = '\n';
    HAL_UART_Transmit(&UartHandle, &ret, 1, HAL_MAX_DELAY);
  }

  return ch;
}

#if defined (__IAR_SYSTEMS_ICC__)

size_t __write(int Handle, const unsigned char * Buf, size_t Bufsize);
size_t __read(int Handle, unsigned char *Buf, size_t Bufsize);

/** @brief  IAR specific low level standard input
 *  @param  Handle IAR internal handle
 *  @param  Buf Buffer where to store characters read from stdin
 *  @param  Bufsize Number of characters to read
 *  @retval Number of characters read
 */
size_t __read(int Handle, unsigned char *Buf, size_t Bufsize)
{
  int i;

  if (Handle != 0){
    return -1;
  }

  for(i=0; i<Bufsize; i++)
    Buf[i] = Uart_Receive_Char();

  return Bufsize;
}

/** @brief  IAR specific low level standard output
 *  @param  Handle IAR internal handle
 *  @param  Buf Buffer containing characters to be written to stdout
 *  @param  Bufsize Number of characters to write
 *  @retval Number of characters read
 */
size_t __write(int Handle, const unsigned char * Buf, size_t Bufsize)
{
  int i;

  if (Handle != 1 && Handle != 2){
    return -1;
  }

  for(i=0; i< Bufsize; i++)
    Uart_Send_Char(Buf[i]);

  return Bufsize;
}

#elif defined (__CC_ARM)

/** @brief  fgetc call for standard input implementation
 *  @param  f File pointer
 *  @retval Character acquired from standard input
 */
int fgetc(FILE *f)
{
  return Uart_Receive_Char();
}

#elif defined (__GNUC__)

/** @brief  getchar call for standard input implementation
 *  @param  None
 *  @retval Character acquired from standard input
 */
int __io_getchar(void)
{
  return Uart_Receive_Char();
}

#else
#error "Toolchain not supported"
#endif

/**
 * @brief  Print available key options
 * @param  None
 * @retval None
 */
void Main_Menu(void)
{
  if ((central_status != SELECT_ANOTHER_CHARACTERISTIC) &&
      (central_status != RECEIVE_NOTIFICATIONS)) {
    printf("\n * *********************** MENU *********************** *\r\n");
    printf(" *                                                      *\r\n");

    switch (central_status)
    {
    case INIT_STATUS:
      printf(" * [S/s]   Scan the network                             *\n");
      break;
    case SELECT_DEVICE:
      printf(" * [S/s]   Scan the network                             *\n");
      printf(" * [0 - %d] Connect to a device                          *\n",
             saved_devices.dev_num-1);
      break;
    case SELECT_CHARACTERISTIC:
      printf(" * [x.y.z] Update the characteristic properties         *\n");
      printf(" *         - x = service index                          *\n");
      printf(" *         - y = characteristic index                   *\n");
      printf(" *         - z = characteristic property index          *\n");
      printf(" *         [D/d] Disable notifications                  *\n");
      printf(" * [P/p]   Print device info                            *\n");
      printf(" * [C/c]   Close connection                             *\n");
      break;
    default:
      break;
    }

    printf(" *                                                      *\r\n");
    printf(" * *********************** ---- *********************** *\r\n");
  }

  if (central_status != RECEIVE_NOTIFICATIONS) {
    printf("\r\n Type your choice..... ");
  }
  fflush(stdout);
}

/**
 * @brief  Get a string from console
 * @param  The string buffer
 * @retval The string lenght
 */
uint8_t Get_Value(uint8_t* console_ch)
{
  uint8_t i = 0;

  while (i < MAX_STRING_LENGTH) {
    console_ch[i] = Uart_Receive_Char();
    if (console_ch[i] == 0x0D) { /* hex for carriage return */
      break;
    }
    i++;
  }
  return i;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
