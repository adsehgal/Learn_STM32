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

// Telnet states
#define TELSTATE_NORMAL 0
#define TELSTATE_IAC    1
#define TELSTATE_OPT    2
#define TELSTATE_SB     3
#define TELSTATE_OPTDAT 4
#define TELSTATE_SE     5

// Telnet special characters
#define TELSC_SE    0xF0	//	240   // End of subnegotiation parameters
#define TELSC_NOP   0xF1	//	241   // No operation
#define TELSC_MARK  0xF2	//	242   // Data mark
#define TELSC_BRK   0xF3	//	243   // Break
#define TELSC_IP    0xF4	//	244   // Interrupt process
#define TELSC_AO    0xF5	//	245   // Abort output
#define TELSC_AYT   0xF6	//	246   // Are you there
#define TELSC_EC    0xF7	//	247   // Erase character
#define TELSC_EL    0xF8	//	248   // Erase line
#define TELSC_GA    0xF9	//	249   // Go ahead
#define TELSC_SB    0xFA	//	250   // Start of subnegotiation parameters
#define TELSC_WILL  0xFB	//	251   // Will option code
#define TELSC_WONT  0xFC	//	252   // Won't option code
#define TELSC_DO    0xFD	//	253   // Do option code
#define TELSC_DONT  0xFE	//	254   // Don't option code
#define TELSC_IAC   0xFF	//	255   // Interpret as command

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

//Telnet Terminal Type
#define TELTERM_UNKNOWN 0
#define TELTERM_CONSOLE 1
#define TELTERM_VT100   2

#define TELNET_BUF_SIZE 512
#define TELNET_SHELL_CMD_SIZE 64
#define TELNET_OPT_BUF_SIZE 256
#define TELNET_NUM_PREV_CMDS 10

/**
 * contains terminal setup like type, cols, rows...
 */
typedef struct Terminal {
	int type;
	int cols;
	int rows;
} terminal_t;

/**
 * contains buffer for data recvd/sent on the shell
 */
typedef struct Telnetbuf {
	unsigned char data[TELNET_BUF_SIZE];
	unsigned char *start;
	unsigned char *end;
} telnetbuf_t;

/**
 * contains IAC... options and commands recvd on the shell
 */
typedef struct Telnetopt {
	int code;
	unsigned char data[TELNET_OPT_BUF_SIZE];
	int len;
} telnetopt_t;

typedef struct TelnetPrevCmds {
	char cmds[TELNET_NUM_PREV_CMDS][TELNET_SHELL_CMD_SIZE];
	uint8_t pos;	//tracks number of stored commands
	uint8_t selected; //tracks which cmd was displayed last time up arrow was detected
} telnetprev_t;

/**
 * contains terminal socket, state, options, terminal type and data buffers
 */
typedef struct Terminalstate {
	int socket;
	int state;
	telnetopt_t opt;
	terminal_t term;
	telnetbuf_t buffIn;
	telnetbuf_t buffOut;
	telnetprev_t prev;
} terminalstate_t;

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
 * @brief:
 * @param: void
 * @return: void
 */
void shellPutInputString(void);

/**
 * @brief: flushes the remote terminal
 * @param: void
 * @return: void
 */
uint32_t shellFlush(void);

/**
 * @brief: get a char from the remote terminal
 * @param: void
 * @return: uint8_t - recvd char or 0 on EOF
 */
uint8_t shellGetc(void);

/**
 * @brief: get a string from the remote terminal
 * @param: buff - pointer at which to store the string
 * @param: len - max size of string to recv
 * @param: echo - echo back recvd string to remote?
 * @return: uint8_t - 0 on error, 1 otherwise
 */
uint8_t shellGets(char *buffRecv, uint32_t len, uint8_t echo);

#endif /* INC_SHELL_STDIO_H_ */
