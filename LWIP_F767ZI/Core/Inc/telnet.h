/*
 * telnet.h
 *
 *  Created on: 24 Apr 2021
 *      Author: adityasehgal
 */

#ifndef INC_TELNET_H_
#define INC_TELNET_H_

#include <stdbool.h>

void telnet_init(void);
void telnet_reboot_on_exit(bool enable);
void telnet_shutdown_on_exit(bool enable);
void telnet_putc(char c);
void telnet_puts(char *str);
void telnet_printf(const char *fmt, ...);
int telnet_flush(void);
bool telnet_has_input(void);
int telnet_getc(void);
bool telnet_gets(char *buff, int len, int tocase, bool echo);

#endif /* INC_TELNET_H_ */
