/*
 * shell_stdio.h
 *
 *  Created on: 18 Oct 2021
 *      Author: adityasehgal
 */

#ifndef INC_SHELL_STDIO_H_
#define INC_SHELL_STDIO_H_

#include <stdio.h>
#include <stdarg.h>

#include "cmsis_os.h"
#include "lwip.h"
#include "sockets.h"

#include "telnet.h"

/**
 * @brief: init shell struct and tell terminal for remote echo (here)
 * @param: void
 * @return: void
 */
void shellInit(int socket);

/**
 * @brief: check if socket needs to be closed and do so is needed
 * @param: void
 * @return: void
 */
void shellClose(void);

/**
 * @brief: put a char on the remote terminal
 * @param: c - char to print
 * @return: void
 */
void shellPutc(char c);

/**
 * @brief: put a string on the remote terminal
 * @param: str - pointer to string to print
 * @return: void
 */
void shellPuts(char *str);

/**
 * @brief: print a format specified string on remote terminal
 * @param: fmt - format specified string
 * @param: ... - optional argumants to complete format spec
 * @return: void
 */
void shellPrintf(const char *fmt, ...);

/**
 * @brief: flushes the remote terminal
 * @param: void
 * @return: void
 */
uint32_t shellFlush(void);

/**
 * @brief: get a char from the remote terminal
 * @param: void
 * @return: int32_t - recvd char or -ve error code
 */
int32_t shellGetc(void);

/**
 * @brief: get a string from the remote terminal
 * @param: buff - pointer at which to store the string
 * @param: len - max size of string to recv
 * @param: echo - echo back recvd string to remote?
 * @return: void
 */
uint8_t shellGets(char *buff, uint32_t len, uint8_t echo);

#endif /* INC_SHELL_STDIO_H_ */
