/*
 * telnet.c
 *
 *  Created on: Jun 9, 2021
 *      Author: adityasehgal
 */
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "main.h"

#include "cmsis_os2.h"

#include "ethernetif.h"
#include "lwip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

#include "shell.h"
#include "telnet.h"

//Ethernet link thread Argument
struct link_str link_arg;

#define TELNET_THREAD_PRIO    ( osPriorityNormal )
#define TELNET_THREAD_STACKSIZE 2048

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

static uint32_t TELNET_TIMEOUT = 60 * 1000 * 1000;

static struct termstate tstate;
struct termstate *ts = &tstate;



static void telnetThread(void *arg) {
	int sock, newconn, size;
	struct sockaddr_in address, remotehost;

	// Create a TCP socket.
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("TELNET: cannot create socket");
		return;
	}

	// Bind to port 23 at any interface.
	address.sin_family = AF_INET;
	address.sin_port = htons(23);
	address.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (struct sockaddr*) &address, sizeof(address)) < 0) {
		printf("TELNET: cannot bind socket");
		return;
	}

	// Listen for incoming connections (TCP listen backlog = 1).
	listen(sock, 1);

	size = sizeof(remotehost);

	for (;;) {
		newconn = accept(sock, (struct sockaddr* ) &remotehost,
				(socklen_t* ) &size);
		if (newconn > 0)
			telnetShell(newconn);
	}
}

void telnetInit(void) {
	sys_thread_new("TELNET", telnetThread, NULL, TELNET_THREAD_STACKSIZE,
	TELNET_THREAD_PRIO);

	osThreadAttr_t attributes;
	memset(&attributes, 0x0, sizeof(osThreadAttr_t));
	attributes.name = "TELNET";
	attributes.stack_size = TELNET_THREAD_STACKSIZE;
	attributes.priority = TELNET_THREAD_PRIO;
	osThreadNew(telnetThread, &link_arg, &attributes);
}

