/*
 * rx.c
 *
 *  Created on: 23 Jun 2021
 *      Author: adityasehgal
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "cmsis_os.h"

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

#include "rx.h"
#include "shell.h"

//packets to send - 1 packet per thread/task
static termstate_t telnetPacket;
static termstate_t *ts = &telnetPacket;

static uint32_t TELNET_TIMEOUT = 60 * 1000 * 1000;

// OS tasks:
osThreadId telnetTaskHandle;

/**************************************************************************
 **************************************************************************
 **************************************************************************
 **************************************************************************
 **************************************************************************/

/**
 * Static function Definitions
 */

static void telnetSendopt(struct termstate *ts, int code, int option) {
	unsigned char buf[3];
	buf[0] = TELNET_IAC;
	buf[1] = (unsigned char) code;
	buf[2] = (unsigned char) option;
	send(ts->sock, buf, 3, 0);
}

static void telnetParseopt(struct termstate *ts, int code, int option) {
	switch (option) {
	case TELOPT_ECHO:
		break;
	case TELOPT_SUPPRESS_GO_AHEAD:
		if (code == TELNET_WILL || code == TELNET_WONT)
			telnetSendopt(ts, TELNET_DO, option);
		else
			telnetSendopt(ts, TELNET_WILL, option);
		break;
	case TELOPT_TERMINAL_TYPE:
	case TELOPT_NAWS:
	case TELOPT_TERMINAL_SPEED:
		telnetSendopt(ts, TELNET_DO, option);
		break;
	default:
		if ((code == TELNET_WILL) || (code == TELNET_WONT))
			telnetSendopt(ts, TELNET_DONT, option);
		else
			telnetSendopt(ts, TELNET_WONT, option);
		break;
	}
}

static void telnetParseoptdata(struct termstate *ts, int option,
		unsigned char *data, int len) {
	switch (option) {
	case TELOPT_NAWS:
		if (len == 4) {
			int cols = ntohs(*(unsigned short* ) data);
			int lines = ntohs(*(unsigned short* ) (data + 2));
			if (cols)
				ts->term.cols = cols;
			if (lines)
				ts->term.lines = lines;
		}
		break;
	case TELOPT_TERMINAL_SPEED:
		break;
	case TELOPT_TERMINAL_TYPE:
		break;
	}
}

static void telnetProcess(struct termstate *ts) {
	unsigned char *p = ts->bi.start;
	unsigned char *q = p;
	while (p < ts->bi.end) {
		int c = *p++;
		switch (ts->state) {
		case STATE_NORMAL:
			if (c == TELNET_IAC)
				ts->state = STATE_IAC;
			else
				*q++ = c;
			break;
		case STATE_IAC:
			switch (c) {
			case TELNET_IAC:
				*q++ = c;
				ts->state = STATE_NORMAL;
				break;
			case TELNET_WILL:
			case TELNET_WONT:
			case TELNET_DO:
			case TELNET_DONT:
				ts->code = c;
				ts->state = STATE_OPT;
				break;
			case TELNET_SB:
				ts->state = STATE_SB;
				break;
			default:
				ts->state = STATE_NORMAL;
			}
			break;
		case STATE_OPT:
			telnetParseopt(ts, ts->code, c);
			ts->state = STATE_NORMAL;
			break;
		case STATE_SB:
			ts->code = c;
			ts->optlen = 0;
			ts->state = STATE_OPTDAT;
			break;
		case STATE_OPTDAT:
			if (c == TELNET_IAC)
				ts->state = STATE_SE;
			else if (ts->optlen < sizeof(ts->optdata))
				ts->optdata[ts->optlen++] = c;
			break;
		case STATE_SE:
			if (c == TELNET_SE)
				telnetParseoptdata(ts, ts->code, ts->optdata, ts->optlen);
			ts->state = STATE_NORMAL;
			break;
		}
	}
	ts->bi.end = q;
}

static int telnetWrite(char *str, int size, bool flush) {
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

static void telnetShell(int s) {
	int off;

// TCP_NODELAY is used to disable the Nagle buffering algorithm. It
// should only be set for applications such as telnet that send frequent
// small bursts of information without getting an immediate response,
// where timely delivery of data is required.
	off = 0;
	setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char* ) &off, sizeof(off));

	// Initialize terminal state.
	memset(ts, 0, sizeof(termstate_t));
	ts->sock = s;
	ts->state = STATE_NORMAL;
	ts->term.type = TERM_VT100;
	ts->term.cols = 80;
	ts->term.lines = 25;

	// Send initial option that we'll echo on this side.
	telnetSendopt(ts, TELNET_WILL, TELOPT_ECHO);

	// Reset the output buffer.
	ts->bo.start = ts->bo.data;

	// Run the shell.
	shellProcess();

	// Close the socket.
	if (ts->sock >= 0) {
		close(ts->sock);
		ts->sock = -1;
	}
}

static void telnetThread(void const *argument) {

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

	if (bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0) {
		printf("TELNET: cannot bind socket");
		return;
	}

	// Listen for incoming connections (TCP listen backlog = 1).
	listen(sock, 1);

	size = sizeof(remotehost);

	for (;;) {
		newconn = accept(sock, (struct sockaddr* )&remotehost,
				(socklen_t* )&size);
		if (newconn > 0)
			telnetShell(newconn);
	}

}

/**
 * Static function definitions END
 */

/**************************************************************************
 **************************************************************************
 **************************************************************************
 **************************************************************************
 **************************************************************************/

/**
 * Public function definitions
 */
void telnetInit(void) {
	osThreadDef(telnetTask, telnetThread, osPriorityNormal, 0, 1024);
	telnetTaskHandle = osThreadCreate(osThread(telnetTask), NULL);
}


void telnetPutc(char c) {
	if (c == '\n')
		telnetPutc('\r');
	if (ts->sock > 0)
		telnetWrite(&c, 1, (c == '\n') ? true : false);
}

void telnetPuts(char *str) {
	// Send each character in the string.
	while (*str)
		telnetPutc(*(str++));
}

void telnetPrintf(const char *fmt, ...) {
	va_list arp;
	char buffer[128];

	va_start(arp, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, arp);
	va_end(arp);

	telnetPuts(buffer);
}

// Flush the telnet output buffers.
int telnetFlush(void) {
	int i;
	int n;
	int len;
	fd_set data_write;
	struct timeval tv;
	unsigned char *ptr;

	len = n = 0;

	// Is there data to flush?
	if (ts->bo.start > ts->bo.data) {
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

	return n < 0 ? n : len;
}

// Return true if there is input waiting.
bool telnetHasInput(void) {
	int i;
	int n;
	fd_set fdsr;
	struct timeval tv_timeout;

	// Do we need to fill the input buffer with new data?
	if (ts->bi.start == ts->bi.end) {
		FD_ZERO(&fdsr);
		FD_SET(ts->sock, &fdsr);
		tv_timeout.tv_sec = 0;
		tv_timeout.tv_usec = 1000;

		// Wait up to 1 millisecond for input activity on the socket.
		i = select(FD_SETSIZE, &fdsr, NULL, NULL, &tv_timeout);

		// Was an error returned?
		if (i < 0)
			return true;

		// Do we have data to read?
		if ((i > 0) && FD_ISSET(ts->sock, &fdsr)) {
			// Read data from user.
			n = recv(ts->sock, ts->bi.data, sizeof(ts->bi.data), 0);

			// Was a error returned?
			if (n < 0)
				return true;

			// End of stream?
			if (n == 0)
				return true;

			// Set the input buffer.
			ts->bi.start = ts->bi.data;
			ts->bi.end = ts->bi.data + n;

			// Process input for telnet options.
			telnetProcess(ts);
		}

		telnetFlush();
	}

	// Return true if there is data to read.
	return (ts->bi.start == ts->bi.end) ? false : true;
}

// Get the next character from the buffer.
int telnetGetc(void) {
	int i;
	int n;
	int ch;
	fd_set fdsr;
	struct timeval tv_timeout;

	uint32_t systick_start;
	uint64_t systick_extended;
	uint32_t current_tick;

	systick_start = osKernelSysTick();
	systick_extended = 0;

	// Do we need to fill the input buffer with new data?
	while (ts->bi.start == ts->bi.end) {
		FD_ZERO(&fdsr);
		FD_SET(ts->sock, &fdsr);
		tv_timeout.tv_sec = 1;
		tv_timeout.tv_usec = 0;

		// Wait for input activity on the socket.
		i = select(FD_SETSIZE, &fdsr, NULL, NULL, &tv_timeout);

		// Was an error returned?
		if (i < 0)
			break;

		current_tick = osKernelSysTick();
		systick_extended += current_tick - systick_start;
		systick_start = current_tick;

		if (systick_extended
				> osKernelSysTickMicroSec(((uint64_t )TELNET_TIMEOUT))) {
			return -1;
		}

		// Do we have data to read?
		if ((i > 0) && FD_ISSET(ts->sock, &fdsr)) {

			// Read data from user.
			n = recv(ts->sock, ts->bi.data, sizeof(ts->bi.data), 0);

			// Was a error returned?
			if (n < 0)
				break;

			// End of stream?
			if (n == 0)
				break;

			// Set the input buffer.
			ts->bi.start = ts->bi.data;
			ts->bi.end = ts->bi.data + n;

			// Process input for telnet options.
			telnetProcess(ts);
		}

		telnetFlush();
	}

	// If no data then either error or end of stream.
	if (ts->bi.start == ts->bi.end)
		return -1;

	// Get the next character in the buffer.
	ch = (unsigned char) *(ts->bi.start++);

	return ch;
}

bool telnetGets(char *buff, int len, int tocase, bool echo) {
	int c;
	int i;

	i = 0;
	for (;;) {
		// Get a char from the incoming stream.
		c = telnetGetc();

		// End of stream?
		if (c < 0)
			return false;


		// Convert to upper/lower case?
		if (tocase > 0)
			c = toupper(c);
		if (tocase < 0)
			c = tolower(c);

		// End of line?
		if (c == '\r')
			break;

		// Back space?
		if (((c == '\b') && i) || ((c == 0x7f) && i)) {
			i--;
			if (echo) {
				telnetPutc(c);
				telnetPutc(' ');
				telnetPutc(c);
			}
		}

		// Visible chars.
		if ((c >= ' ') && (c < 0x7f) && (i < (len - 1))) {
			buff[i++] = c;
			if (echo)
				telnetPutc(c);
		}

		telnetFlush();
	}

	// Null terminate.
	buff[i] = 0;
	if (echo) {
		telnetPuts("\r\n");
		telnetFlush();
	}

	return true;
}
