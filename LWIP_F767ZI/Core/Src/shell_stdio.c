/*
 * shell_stdio.c
 *
 *  Created on: 18 Oct 2021
 *      Author: adityasehgal
 */

#include "shell_stdio.h"

/**
 * struct containing type of terminal and other relevant data
 */
terminalstate_t shell;

/**
 * timeout for the telnet shell
 */
static uint32_t SHELL_TIMEOUT = 60 * 1000 * 1000;

/**
 * @brief: send option to remote as an IAC command
 * @param: code - WILL, WONT, DO, DONT
 * @param: option - ECHO, SUPRESS.. and so on
 * @return: void
 */
static void shellSendopt(int code, int option) {
	unsigned char buf[3];
	buf[0] = TELSC_IAC;
	buf[1] = (unsigned char) code;
	buf[2] = (unsigned char) option;
	send(shell.socket, buf, 3, 0);
}

/**
 * @brief: write a string to the terminal
 * @param: str - string to write
 * @param: size - length of string
 * @param: flush - 1=flush terminal, 0 otherwise
 * @return: uint32_t - number of bytes written
 */
static uint32_t shellWrite(char *str, uint32_t size, uint8_t flush) {
	int numFd = 0;
	int numSent = 0;
	int len = 0;
	fd_set data;
	struct timeval selectTimeout;
	unsigned char *ptr = (unsigned char*) str;

	do {

		len = TELNET_BUF_SIZE
				- (int) (shell.buffOut.start - shell.buffOut.data);

		if (size < len) {
			len = size;
		}

		if (len >= 0) {
			memcpy(shell.buffOut.start, str, len);
			size -= len;
			shell.buffOut.start += len;
			str += len;
		}

		if (((int) (shell.buffOut.start - shell.buffOut.data) >= TELNET_BUF_SIZE)
				|| flush) {

			numSent = 0;
			len = numSent;
			ptr = shell.buffOut.data;

			do {
				FD_ZERO(&data);
				FD_SET(shell.socket, &data);
				selectTimeout.tv_sec = 1;
				selectTimeout.tv_usec = 0;
				numFd = select(FD_SETSIZE, NULL, &data, NULL, &selectTimeout);

				if ((numFd > 0) && (FD_ISSET(shell.socket, &data) != 0)) {
					numSent = send(shell.socket, ptr,
							(int ) (shell.buffOut.start - shell.buffOut.data)
									- len, 0);

					if (numSent >= 0) {
						ptr += numSent;
						len += numSent;
					}
				}
			} while ((numSent >= 0)
					&& (len < (int) (shell.buffOut.start - shell.buffOut.data))
					&& (numFd > 0));

			shell.buffOut.start = shell.buffOut.data;

		}

	} while (size > 0);

	return numSent < 0 ? numSent : len;

}

/**
 * @brief: parse extended options such as terminal speed...
 * @param: code - WILL, WONT, DO, DONT
 * @param: option - ECHO, SUPRESS.. and so on
 * @return: void
 */
static void shellParseOpt(int code, int option) {
	switch (option) {

	case TELOPT_TERMINAL_TYPE:
	case TELOPT_NAWS:
	case TELOPT_TERMINAL_SPEED:
		shellSendopt( TELSC_DO, option);
		break;

	case TELOPT_ECHO:
		break;

	case TELOPT_SUPPRESS_GO_AHEAD:
		if (code == TELSC_WILL || code == TELSC_WONT) {
			shellSendopt(TELSC_DO, option);
		} else {
			shellSendopt(TELSC_WILL, option);
		}
		break;

	default:
		if ((code == TELSC_WILL) || (code == TELSC_WONT)) {
			shellSendopt(TELSC_DONT, option);
		} else {
			shellSendopt(TELSC_WONT, option);
		}
		break;

	}
}

/**
 * @brief: parse option data that was recvd
 * @param: option - ECHO, SUPRESS.. and so on
 * @param: data - string containing the data such as #cols...
 * @param: len - length of data
 * @return: void
 */
static void shellParseOptData(int option, unsigned char *data, int len) {
	switch (option) {

	case TELOPT_NAWS:
		if (len == 4) {
			int cols = ntohs(*(unsigned short* ) data);
			int rows = ntohs(*(unsigned short* ) (data + 2));
			if (cols) {
				shell.term.cols = cols;
			}
			if (rows) {
				shell.term.rows = rows;
			}
		}
		break;

	case TELOPT_TERMINAL_SPEED:
		break;

	case TELOPT_TERMINAL_TYPE:
		break;
	}
}

/**
 * @brief: processes any IAC options and such
 * @param: void
 * @return: void
 */
static void shellProcessOpt(void) {
	unsigned char *p = shell.buffIn.start;
	unsigned char *q = p;

	while (p < shell.buffIn.end) {
		int c = *p++;

		switch (shell.state) {

		case TELSTATE_IAC:
			switch (c) {
			case TELSC_WILL:
			case TELSC_WONT:
			case TELSC_DO:
			case TELSC_DONT:
				shell.opt.code = c;
				shell.state = TELSTATE_OPT;
				break;

			case TELSC_IAC:
				*q++ = c;
				shell.state = TELSTATE_NORMAL;
				break;

			case TELSC_SB:
				shell.state = TELSTATE_SB;
				break;

			default:
				shell.state = TELSTATE_NORMAL;
			}
			break;

		case TELSTATE_NORMAL:
			if (c == TELSC_IAC) {
				shell.state = TELSTATE_IAC;
			} else {
				*q++ = c;
			}
			break;

		case TELSTATE_OPT:
			shellParseOpt(shell.opt.code, c);
			shell.state = TELSTATE_NORMAL;
			break;

		case TELSTATE_SB:
			shell.opt.code = c;
			shell.opt.len = 0;
			shell.state = TELSTATE_OPTDAT;
			break;

		case TELSTATE_OPTDAT:
			if (c == TELSC_IAC) {
				shell.state = TELSTATE_SE;
			} else if (shell.opt.len < sizeof(shell.opt.data)) {
				shell.opt.data[shell.opt.len++] = c;
			}
			break;

		case TELSTATE_SE:
			if (c == TELSC_SE) {
				shellParseOptData(shell.opt.code, shell.opt.data,
						shell.opt.len);
			}
			shell.state = TELSTATE_NORMAL;
			break;

		}
	}
	shell.buffIn.end = q;
}

/**
 * @brief: erases the last input char
 * @param: void
 * @return: void
 */
static void shellEraseInputChar(void) {
	shellPutc(0x08);
	shellPutc(0x20);
	shellPutc(0x08);
}

static void shellEraseInputChars(uint32_t num) {
	for (int i = 0; i < num; i++) {
		shellEraseInputChar();
	}
}

void shellInit(int socket) {

	int optLen = 0;
	setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char* )&optLen,
			sizeof(optLen));

	memset(&shell, 0, sizeof(shell));
	shell.socket = socket;
	shell.state = TELSTATE_NORMAL;
	shell.term.type = TELTERM_VT100;
	shell.term.cols = 80;
	shell.term.rows = 25;

	shellSendopt(TELSC_WILL, TELOPT_ECHO);
	shell.buffOut.start = shell.buffOut.data;
}

void shellClose(void) {
	if (shell.socket >= 0) {
		close(shell.socket);
		shell.socket = -1;
	}
}

void shellPutc(char c) {
	if (c == '\n')
		shellPutc('\r');
	if (shell.socket > 0)
		shellWrite(&c, 1, (c == '\n') ? 1 : 0);
}

void shellPuts(char *str) {
	while (*str)
		shellPutc(*(str++));
}

void shellPrintf(const char *fmt, ...) {
	va_list arp;
	char buffer[256];

	va_start(arp, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, arp);
	va_end(arp);

	shellPuts(buffer);
}

void shellPutInputString(void) {
	shellPuts("> ");
}

uint32_t shellFlush(void) {
	int err = 0;
	int numSent = 0;
	int len = 0;

	fd_set data;
	struct timeval selectTimeout;
	selectTimeout.tv_sec = 1;
	selectTimeout.tv_usec = 0;

	unsigned char *ptr = 0;

	if (shell.buffOut.start > shell.buffOut.data) {
		ptr = shell.buffOut.data;

		do {
			FD_ZERO(&data);
			FD_SET(shell.socket, &data);

			err = select(FD_SETSIZE, NULL, &data, NULL, &selectTimeout);

			if ((err > 0) && FD_ISSET(shell.socket,&data) != 0) {
				numSent = send(shell.socket, ptr,
						(int ) (shell.buffOut.start - shell.buffOut.data) - len,
						0);
				if (numSent > 0) {
					ptr += numSent;
					len += numSent;
				}
			}
		} while ((numSent >= 0)
				&& (len < (int) (shell.buffOut.start - shell.buffOut.data))
				&& (err > 0));

		shell.buffOut.start = shell.buffOut.data;
	}

	return numSent < 0 ? numSent : len;
}

uint8_t shellGetc(void) {
	int err = 0;
	int recvLen = 0;
	uint8_t ch = 0;

	fd_set fdsr;
	struct timeval selectTimeout;
	selectTimeout.tv_sec = 1;
	selectTimeout.tv_usec = 0;

	uint32_t timeOutStart = osKernelSysTick();
	uint32_t timeOutAccum = timeOutStart;

	while (shell.buffIn.start == shell.buffIn.end) {
		FD_ZERO(&fdsr);
		FD_SET(shell.socket, &fdsr);

		err = select(FD_SETSIZE, &fdsr, NULL, NULL, &selectTimeout);

		//error encountered
		if (err < 0) {
//			return 0;
			break;
		}

		timeOutAccum += osKernelSysTick();

		//check for timeout
		if (timeOutAccum - timeOutStart >= SHELL_TIMEOUT) {
			return 0;
		}

		if ((err > 0) && FD_ISSET(shell.socket, &fdsr)) {
			recvLen = read(shell.socket, shell.buffIn.data,
					sizeof(shell.buffIn.data));

			//error encountered
			if (recvLen < 0) {
				break;
			}
			//end of stream
			else if (recvLen == 0) {
				break;
			}
			//append stream
			else {

				shell.buffIn.start = shell.buffIn.data;
				shell.buffIn.end = shell.buffIn.data + recvLen;

				shellProcessOpt();

			}

		}
	}
	shellFlush();

	if (shell.buffIn.start == shell.buffIn.end) {
		return -1;
	}

	ch = *shell.buffIn.start++;
	return ch;
}

uint8_t shellGets(char *buffRecv, uint32_t len, uint8_t echo) {
	int ch = 0;
	uint32_t numRecv = 0;

	typedef enum ARROWS_ENUM {
		arrowNone = 0x00,	//
		arrowUp = 0x10,		//
		arrowDw = 0x20,		//
		arrowLt = 0x40,		//
		arrowRt = 0x80,		//

		arrowEsc = 0x1B,	//
		arrowBrkt = 0x5B,	//
		arrowA = 0x41,		//	up		( ^[A )
		arrowB = 0x42,		//	dwn		( ^[B )
		arrowC = 0x43,		//	right	( ^[C )
		arrowD = 0x44,		//	left	( ^[D )
	} arrows_t;
	arrows_t arrowRecvd = arrowNone;

	for (;;) {

		ch = shellGetc();

		//end of stream/error
		if (!ch) {
			return 0xFF;
		}

		//end of cmd
		else if (ch == '\r') {
			//add command to list of prev commands
			if (numRecv != 0 && buffRecv != NULL) {
				if (shell.prev.pos >= TELNET_NUM_PREV_CMDS) {
					shell.prev.pos = 0;
				}
				memcpy(shell.prev.cmds[shell.prev.pos], buffRecv, len);
				shell.prev.pos++;
				shell.prev.selected = 0;
			}
			break;
		}

		//process backspace and deletes
		else if (((ch == '\b') || (ch == 0x7F)) && numRecv) {
			shellEraseInputChar();
			buffRecv[--numRecv] = '\0';
		}

		//append char into string
		else if ((ch >= ' ') && (ch <= '~') && (numRecv < (len - 1))) {
			buffRecv[numRecv++] = ch;
			if (echo) {
				shellPutc(ch);
			}
		}

		// process any arrows for cmd history
		if (ch == arrowEsc) {	// esc recvd
			arrowRecvd = 1;
		}

		if (arrowRecvd) {
			// decode which char was recvd after escape
			switch (ch) {
			case arrowEsc:
				break;
			case arrowBrkt:
				arrowRecvd++;
				break;
			case arrowA:
				arrowRecvd = arrowUp;
				break;
			case arrowB:
				arrowRecvd = arrowDw;
				break;
			case arrowC:
				arrowRecvd = arrowRt;
				break;
			case arrowD:
				arrowRecvd = arrowLt;
				break;
			default:
				arrowRecvd = arrowNone;
				break;
			}
		}

		switch (arrowRecvd) {
		case arrowUp:	// process older command selection
			// erase entire input line
			shellEraseInputChars(len);
			numRecv = 0;
			if (shell.prev.selected >= TELNET_NUM_PREV_CMDS) {
				shell.prev.selected = 0;
			}
			// ignore any empty history
			while (shell.prev.cmds[shell.prev.selected] == NULL) {
				shell.prev.selected++;
				if (shell.prev.selected >= TELNET_NUM_PREV_CMDS) {
					shell.prev.selected = 0;
					break;
				}
			}
			// put selected previous command into main buffer
			strcpy(buffRecv, shell.prev.cmds[shell.prev.selected]);
			shell.prev.selected++;
			// display older command
			shellPutInputString();
			shellPuts(buffRecv);
			numRecv = strlen(buffRecv);
			break;
		case arrowDw:	// process newer command selection
			// erase entire input line
			shellEraseInputChars(len);
			numRecv = 0;
			if (shell.prev.selected < 0) {
				shell.prev.selected = 0;
			}
			// ignore any empty buffers
			while (shell.prev.cmds[shell.prev.selected] == NULL) {
				shell.prev.selected--;
				if (shell.prev.selected < 0) {
					shell.prev.selected = 0;
					break;
				}
			}
			strcpy(buffRecv, shell.prev.cmds[shell.prev.selected]);
			shell.prev.selected--;
			// display newer command
			shellPutInputString();
			shellPuts(buffRecv);
			numRecv = strlen(buffRecv);
			break;
		case arrowRt:
			// should process adding chars in the middle of the buffer
			break;
		case arrowLt:
			// should process adding chars in the middle of the buffer
			break;
		default:
			// do nothing since invalid arrow or something else was recvd
			arrowRecvd = 0;
			break;
		}

		shellFlush();
	}

// Null terminate.
	buffRecv[numRecv] = 0;
	if (echo) {
		shellPuts("\r\n");
		shellFlush();
	}

	return 1;
}
