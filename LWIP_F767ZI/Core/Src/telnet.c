/*
 * telnet.c
 *
 *  Created on: Oct 15, 2021
 *      Author: root
 */

#include <stdio.h>
#include <stdarg.h>

#include "cmsis_os.h"
#include "lwip.h"
#include "sockets.h"

#include "telnet.h"
#include "shell_stdio.h"

static uint8_t telExit(int argc, char **argv);
static uint8_t telHelp(int argc, char **argv);
static uint8_t telUID(int argc, char **argv);
static uint8_t telTime(int argc, char **argv);
static uint8_t telDevId(int argc, char **argv);
static uint8_t telRevId(int argc, char **argv);
static uint8_t telHalVer(int argc, char **argv);

const telCmd_t commands[] = {	//
		{ "devid", telDevId },		//
				{ "exit", telExit },	//
				{ "halv", telHalVer },	//
				{ "help", telHelp },	//
				{ "revid", telRevId },	//
				{ "time", telTime },	//
				{ "uid", telUID },	//

		};

static uint8_t telExit(int argc, char **argv) {
	// Exit the shell interpreter.
	return 0;
}

static uint8_t telHelp(int argc, char **argv) {
	for (uint32_t i = 0; i < sizeof(commands) / sizeof(telCmd_t); ++i) {
		shellPrintf("%s\n", commands[i].name);
	}
	return 1;
}

static uint8_t telUID(int argc, char **argv) {
	uint32_t uid[3] = { 0 };
	uid[0] = HAL_GetUIDw0();
	uid[1] = HAL_GetUIDw1();
	uid[2] = HAL_GetUIDw2();

	shellPrintf("UID = 0x%08lX%08lX%08lX\n", uid[0], uid[1], uid[2]);

	return 1;
}

static uint8_t telTime(int argc, char **argv) {
	uint32_t time = HAL_GetTick();
	shellPrintf("Uptime = %ldms\n", time);
	return 1;
}

static uint8_t telDevId(int argc, char **argv) {
	uint32_t id = HAL_GetDEVID();
	shellPrintf("Dev ID = 0x%04X\n", id);
	return 1;
}

static uint8_t telRevId(int argc, char **argv) {
	uint32_t id = HAL_GetREVID();
	shellPrintf("Rev ID = 0x%04X\n", id);
	return 1;
}

static uint8_t telHalVer(int argc, char **argv) {
	uint32_t version = HAL_GetHalVersion();
	shellPrintf("HAL Version = 0x%04X\n", version);
	return 1;
}

static int cmdBreakup(char **str, char **word) {
	// Skip leading spaces.
	while (**str && isspace(**(uint8_t** ) str))
		(*str)++;

	// Set the word.
	*word = *str;

	// Skip non-space characters.
	while (**str && !isspace(**(uint8_t** )str))
		(*str)++;

	// Null terminate the word.
	if (**str)
		*(*str)++ = 0;

	return (*str != *word) ? 1 : 0;
}

static uint8_t cmdExec(char *str) {
	int i;
	char *argv[8];
	int argc = 0;
	uint8_t rv = 1;
	telCmd_t *command;

	// Parse the command and any arguments into an array[argv]
	for (i = 0; i < (sizeof(argv) / sizeof(char*)); ++i) {
		cmdBreakup(&str, &argv[i]);
		if (*argv[i] != 0)
			++argc;
	}

	// Search for a matching command.
	command = (telCmd_t*) bsearch(argv[0], commands,
			sizeof(commands) / sizeof(telCmd_t), sizeof(telCmd_t),
			(int (*)(const void*, const void*)) strcasecmp);

	// Call the command if found.
	if (command != 0) {
		rv = (command->funcptr)(argc, argv);
	} else if (argv[0][0] != 0) {
		shellPrintf("Unknown command: %s\n", argv[0]);
	}

	return rv;
}

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

	shellPutc('\n');
	shellPutInputString();
	shellFlush();

	uint8_t err = 0;
	while ((err = shellGets(cmdbuf, TELNET_SHELL_CMD_SIZE, 1))) {
		if (!cmdExec(cmdbuf)) {
			break;
		}

		//recvd EOF
		if (err != 0xFF) {
			shellPutc('\n');
			shellPutInputString();
		}
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
