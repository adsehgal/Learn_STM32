/*
 * shell.c
 *
 *  Created on: Jun 9, 2021
 *      Author: adityasehgal
 */

#include "shell.h"


// Runs the shell by printing the prompt and processing each shell command typed in.
void shellProcess(void) {
	char cmdbuf[64];
//	char uptime_str[32];

// Display version information.
	telnetPrintf("Hello\n");
//	uptime_get_string(uptime_str, sizeof(uptime_str));
//	telnet_printf("%s\n", (char*) USER_FLASH_INFO_STRING_ADDRESS);
//	telnet_printf("%s\n", get_hardware_version());
//	telnet_printf("System uptime: %s\n", uptime_str);

// Display the "sysinfo" kept on the file system with unique information for each robot.
//	shell_ouput_file("sysinfo");

//	if (health_get_watchdog_reset()) {
//		telnet_printf("\n*** BOARD WAS RESET BY WATCHDOG ***\n");
//	}
	// Send a prompt.
	telnetPuts("> ");
	telnet_flush();

	// Get a string.
	while (telnet_gets(cmdbuf, sizeof(cmdbuf), 0, true)) {
		// Process the line as a command.
		if (!shell_command(cmdbuf))
			break;

		// Send a prompt.
		telnetPuts("> ");
		telnet_flush();
	}
}
