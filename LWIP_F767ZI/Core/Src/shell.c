/*
 * shell.c
 *
 *  Created on: 23 Jun 2021
 *      Author: adityasehgal
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#include "cmsis_os.h"

#include "shell.h"
#include "rx.h"

typedef bool (*shell_func)(int argc, char **argv);

struct shell_command {
	char name[16];
	shell_func funcptr;
};
static bool shell_exit(int argc, char **argv);
static bool shellHelp(int argc, char **argv);

// Must be sorted in ascending order.
const struct shell_command commands[] = {
//  {"actuator", shell_actuator},
//  {"adc", shell_adc},
//  {"apaled", shell_apaled},
//  {"button", shell_button},
//  {"cksum", shell_cksum},
//  {"del", shell_del},
//  {"dir", shell_dir},
		{ "exit", shell_exit },
//  {"fan", shell_fan},
//  {"format", shell_format},
//  {"health", shell_health},
		{ "help", shellHelp },
//  {"power", shell_power},
//  {"reboot", shell_reboot},
//  {"rgbled", shell_rgbled},
//  {"shutdown", shell_shutdown},
//  {"sonar", shell_sonar},
//  {"switch", shell_switch},
//  {"type", shell_type},
//  {"wait", shell_wait}
		};
static bool shell_exit(int argc, char **argv) {
	// Exit the shell interpreter.
	return false;
}
static bool shellHelp(int argc, char **argv) {
	uint32_t i;

	// Loop over each shell command.
	for (i = 0; i < sizeof(commands) / sizeof(struct shell_command); ++i) {
		telnetPrintf("%s\n", commands[i].name);
	}

	return true;
}

// Parse out the next non-space word from a string.
// str    Pointer to pointer to the string
// word   Pointer to pointer of next word.
// Returns 0:Failed, 1:Successful
static int shellParse(char **str, char **word) {
	// Skip leading spaces.
	while (**str && isspace(**(uint8_t** )str))
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

// Attempt to execute the shell command.
static bool shellCommand(char *cmdline) {
	int i;
	char *argv[8];
	int argc = 0;
	bool rv = true;
	struct shell_command *command;

	// Parse the command and any arguments into an array.
	for (i = 0; i < (sizeof(argv) / sizeof(char*)); ++i) {
		shellParse(&cmdline, &argv[i]);
		if (*argv[i] != 0)
			++argc;
	}

	// Search for a matching command.
	command = (struct shell_command*) bsearch(argv[0], commands,
			sizeof(commands) / sizeof(struct shell_command),
			sizeof(struct shell_command),
			(int (*)(const void*, const void*)) strcasecmp);

	// Call the command if found.
	if (command)
		rv = (command->funcptr)(argc, argv);
	else
		telnetPrintf("Unknown command: %s\n", argv[0]);

	return rv;
}

// Runs the shell by printing the prompt and processing each shell command typed in.
void shellProcess() {
	char cmdbuf[64];
	telnetPrintf("WAZZUUUPPPP\n");
//  char uptime_str[32];

// Display version information.
//  uptime_get_string(uptime_str, sizeof(uptime_str));
//  telnetPrintf("%s\n", (char *) USER_FLASH_INFO_STRING_ADDRESS);
//  telnetPrintf("%s\n", get_hardware_version());
//  telnetPrintf("System uptime: %s\n", uptime_str);
//
//  if (health_get_watchdog_reset())
//  {
//    telnetPrintf("\n*** BOARD WAS RESET BY WATCHDOG ***\n");
//  }

// Display the "sysinfo" kept on the file system with unique information for each robot.
//	shellOuputFile("sysinfo");

// Send a prompt.
	telnetPuts("> ");
	telnetFlush();

	// Get a string.
	while (telnetGets(cmdbuf, sizeof(cmdbuf), 0, true)) {
		// Process the line as a command.
		if (!shellCommand(cmdbuf))
			break;

		// Send a prompt.
		telnetPuts("> ");
		telnetFlush();
	}
}
