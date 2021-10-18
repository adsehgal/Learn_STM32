/*
 * telnet.c
 *
 *  Created on: Oct 15, 2021
 *      Author: root
 */

#include <stdio.h>

#include "cmsis_os.h"
#include "lwip.h"
#include "sockets.h"

#include "telnet.h"

static terminalstate_t termState;
static uint32_t TELNET_TIMEOUT = 60 * 1000 * 1000;

static int32_t shellGetc(void) {
	int err = 0;
	int recvLen = 0;
	int ch = 0;

	fd_set fdsr;
	struct timeval selectTimeout;
	selectTimeout.tv_sec = 1;
	selectTimeout.tv_usec = 0;

	uint32_t timeOutStart = osKernelSysTick();
	uint32_t timeOutAccum = timeOutStart;

	while (termState.buffIn.start == termState.buffIn.end) {
		FD_ZERO(&fdsr);
		FD_SET(termState.socket, &fdsr);

		err = select(FD_SETSIZE, &fdsr, NULL, NULL, &selectTimeout);

		//error encountered
		if (err < 0) {
			return -1;
		}

		timeOutAccum += osKernelSysTick();

		//check for timeout
		if (timeOutAccum - timeOutStart >= TELNET_TIMEOUT) {
			return -1;
		}

		if (err > 0) {
			recvLen = recv(termState.socket, termState.buffIn.data,
					sizeof(termState.buffIn.data), 0);

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
				termState.buffIn.start = termState.buffIn.data;
				termState.buffIn.end = termState.buffIn.data + recvLen;
			}

		}
	}

	if (termState.buffIn.start == termState.buffIn.end) {
		return -1;
	}

	ch = *termState.buffIn.start++;
	return ch;
}

static uint8_t shellGets(char *buff, int len, uint8_t echo) {

	int ch = 0;
	uint32_t numRecv = 0;

	for (;;) {

		ch = shellGetc();

		//end of stream/error
		if (ch < 0) {
			return 0;
		}

		//end of cmd
		else if (ch == '\r') {
			break;
		}

		else if (ch == '\n') {
			break;
		} else if (((ch == '\b') || (ch == 0x7F)) && numRecv) {
			printf("BS\n");
			numRecv--;

		}

		//append char into string
		if ((ch >= ' ') && (ch <= '~') && (numRecv < (len - 1))) {
			buff[numRecv++] = ch;
		}

	}

	return 1;

}

static void shellProcess(void) {

	char cmdbuf[TELNET_SHELL_CMD_SIZE] = { 0 };

	while (shellGets(cmdbuf, sizeof(cmdbuf), 1)) {
		printf("Recv: %s\n", cmdbuf);
		memset(cmdbuf, 0, sizeof(cmdbuf));
	}
//
//	telnetPrintf("Hello %d\n", 23);
//
//	telnetPuts("> ");
//	telnetFlush();
//	while (telnetGets(cmdbuf, sizeof(cmdbuf), 0, 1)) {
//		// Process the line as a command.
//		if (!shellCommand(cmdbuf))
//			break;
//
//		// Send a prompt.
//		telnetPuts("> ");
//		telnetFlush();
//	}
}

static void telnetSendopt(terminalstate_t *ts, int code, int option) {
	unsigned char buf[3];
	buf[0] = TELNET_IAC;
	buf[1] = (unsigned char) code;
	buf[2] = (unsigned char) option;
	send(ts->socket, buf, 3, 0);
}

static void telnetShell(int socket) {

	int optLen = 0;
	setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char* ) &optLen,
			sizeof(optLen));

	memset(&termState, 0, sizeof(termState));
	termState.socket = socket;
	termState.state = TELSTATE_NORMAL;
	termState.terminal.type = TELTERM_VT100;
	termState.terminal.cols = 80;
	termState.terminal.lines = 25;

//remote echo carried by server (here)
	telnetSendopt(&termState, TELNET_WILL, TELOPT_ECHO);

	termState.buffOut.start = termState.buffOut.data;

	shellProcess();

// Close the socket.
	if (termState.socket >= 0) {
		close(termState.socket);
		termState.socket = -1;
	}
}

static void telnetThread(void *arg) {

	int sockfd = 0;
	int connfd = 0;
	int len = 0;
	struct sockaddr_in servaddr;
	struct sockaddr_in cli;

// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		return;
	}

	bzero(&servaddr, sizeof(servaddr));

// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(23);

// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (struct sockaddr* )&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		return;
	}

// Now server is ready to listen and verification
	if ((listen(sockfd, 1)) != 0) {
		printf("Listen failed...\n");
		return;
	}
	len = sizeof(cli);

	for (;;) {
		connfd = accept(sockfd, (struct sockaddr* )&cli, (socklen_t* )&len);

		if (connfd > 0) {
			telnetShell(connfd);
		}
	}

}

void telnetInit(void) {

	sys_thread_new("TELNET", telnetThread, NULL, DEFAULT_THREAD_STACKSIZE,
	DEFAULT_THREAD_PRIO);

}
