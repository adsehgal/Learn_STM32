/*
 * telnet.h
 *
 *  Created on: Oct 15, 2021
 *      Author: root
 */

#ifndef INC_TELNET_H_
#define INC_TELNET_H_

// Telnet states
#define TELSTATE_NORMAL 0
#define TELSTATE_IAC    1
#define TELSTATE_OPT    2
#define TELSTATE_SB     3
#define TELSTATE_OPTDAT 4
#define TELSTATE_SE     5

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

//Telnet Terminal Type
#define TELTERM_UNKNOWN 0
#define TELTERM_CONSOLE 1
#define TELTERM_VT100   2

#define TELNET_BUF_SIZE 512
#define TELNET_SHELL_CMD_SIZE 64

typedef struct Terminal {
	int type;
	int cols;
	int lines;
} terminal_t;

typedef struct Telnetbuf {
	unsigned char data[TELNET_BUF_SIZE];
	unsigned char *start;
	unsigned char *end;
} telnetbuf_t;

typedef struct Terminalstate {
	int socket;
	int state;
	int code;
	unsigned char optdata[256];
	int optionLen;
	terminal_t terminal;
	telnetbuf_t buffIn;
	telnetbuf_t buffOut;
} terminalstate_t;

void telnetInit(void);

#endif /* INC_TELNET_H_ */
