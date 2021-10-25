/*
 * telnet.h
 *
 *  Created on: Oct 15, 2021
 *      Author: root
 */

#ifndef INC_TELNET_H_
#define INC_TELNET_H_

typedef uint8_t (*telFunc)(int argc, char **argv);

typedef struct TelCmd {
	char name[16];
	telFunc funcptr;
} telCmd_t;

/**
 * @brief: initialize the telnet thread
 * @param: void
 * @param: void
 */
void telnetInit(void);

#endif /* INC_TELNET_H_ */
