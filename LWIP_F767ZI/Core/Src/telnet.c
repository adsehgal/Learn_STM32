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
#include "shell_stdio.h"

/**
 * @brief: get string from shell and process it
 * @param: void
 * @return: void
 */
static void telnetProcess(void) {

	shellFlush();

	char cmdbuf[TELNET_SHELL_CMD_SIZE] = { 0 };

	shellPrintf("Hello %d\n", 23);
	shellFlush();

	shellPuts("> ");
	shellFlush();

	while (shellGets(cmdbuf, sizeof(cmdbuf), 1)) {
		printf("Recv: %s\n", cmdbuf);
		shellPrintf("Recv: %s\n", cmdbuf);
		if (!strcasecmp(cmdbuf, "exit")) {
			break;
		}
		shellPuts("> ");
		shellFlush();
		memset(cmdbuf, 0, sizeof(cmdbuf));
	}

}

/**
 * @brief: run the shell itself
 * @param: socket: file descriptor of the socket being used
 * @return: void
 */
static void telnetShell(int socket) {

	shellInit(socket);

	// run the shell process to recv and process commands
	telnetProcess();

	// check and close socket
	shellClose();

}

/**
 * @brief: main thread for the telnet server
 * @param: arg - not used; only here to comply with rtos reqs
 * @return: void
 */
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

	// Binding newly created socket to given IP
	if ((bind(sockfd, (struct sockaddr* )&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		return;
	}

	// Put the socket into listen mode
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
