/*
 * shell.c
 *
 *  Created on: 24 Apr 2021
 *      Author: adityasehgal
 */

#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "cmsis_os.h"
#include "lwip/sys.h"
#include "shell.h"
#include "telnet.h"

typedef bool (*shell_func)(int argc, char **argv);

struct shell_command {
	char name[16];
	shell_func funcptr;
};

#if HAVE_ACTUATORS
static bool shell_actuator(int argc, char **argv);
#endif

static bool shell_led(int argc, char **argv);
static bool shell_exit(int argc, char **argv);
static bool shell_help(int argc, char **argv);

// Must be sorted in ascending order.
const struct shell_command commands[] = //
		{ //
		{ "exit", shell_exit }, //
				{ "help", shell_help }, //
				{ "led", shell_led }, //
		};

static bool shell_led(int argc, char **argv) {
	bool help = false;

	// Should we set a value?
	if (argc > 1) {

		if (!strcasecmp(argv[1], "print")) {
			telnet_printf("Led states:\n");
		} else if (!strcasecmp(argv[1], "1")) {
			if (!strcasecmp(argv[2], "1")) {
				HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
			} else if (!strcasecmp(argv[2], "0")) {
				HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
			} else if (!strcasecmp(argv[2], "toggle")) {
				HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
			} else {
				telnet_printf("Incorrect LED State\n");
			}

		} else if (!strcasecmp(argv[1], "2")) {
			if (!strcasecmp(argv[2], "1")) {
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
			} else if (!strcasecmp(argv[2], "0")) {
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
			} else if (!strcasecmp(argv[2], "toggle")) {
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
			} else {
				telnet_printf("Incorrect LED State\n");
			}

		} else if (!strcasecmp(argv[1], "3")) {
			if (!strcasecmp(argv[2], "1")) {
				HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
			} else if (!strcasecmp(argv[2], "0")) {
				HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
			} else if (!strcasecmp(argv[2], "toggle")) {
				HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
			} else {
				telnet_printf("Incorrect LED State\n");
			}
		} else if (!strcasecmp(argv[1], "all")) {
			if (!strcasecmp(argv[2], "1")) {
				HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
			} else if (!strcasecmp(argv[2], "0")) {
				HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
			} else if (!strcasecmp(argv[2], "toggle")) {
				HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
				HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
			} else {
				telnet_printf("Incorrect LED State\n");
			}
		} else if (!strcasecmp(argv[1], "dance")) {
			telnet_printf("    LEDs will dance for 2s\n");
			HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
			osDelay(25);
			for (int i = 0; i < 20; i++) {
				HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
				HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
				osDelay(100);
			}
		} else {
			telnet_printf("Incorrect LED\n");
		}
	} else {
		help = true;
	}

	if (help) {
		telnet_printf("LED commands:\n");
		telnet_printf("  led print\n");
		telnet_printf("  led <num> <0 or 1 or toggle>\n");
		telnet_printf("  led clear\n");
		telnet_printf("  led toggle\n");
		telnet_printf("  led dance\n");
	}

	return true;
}

static bool shell_exit(int argc, char **argv) {
	// Exit the shell interpreter.
	return false;
}

static bool shell_help(int argc, char **argv) {
	uint32_t i;

	// Loop over each shell command.
	for (i = 0; i < sizeof(commands) / sizeof(struct shell_command); ++i) {
		telnet_printf("%s\n", commands[i].name);
	}

	return true;
}

// Parse out the next non-space word from a string.
// str    Pointer to pointer to the string
// word   Pointer to pointer of next word.
// Returns 0:Failed, 1:Successful
static int shell_parse(char **str, char **word) {
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
static bool shell_command(char *cmdline) {
	int i;
	char *argv[8];
	int argc = 0;
	bool rv = true;
	struct shell_command *command;

	// Parse the command and any arguments into an array.
	for (i = 0; i < (sizeof(argv) / sizeof(char*)); ++i) {
		shell_parse(&cmdline, &argv[i]);
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
		telnet_printf("Unknown command: %s\n", argv[0]);

	return rv;
}

// Runs the shell by printing the prompt and processing each shell command typed in.
void shell_process() {
	char cmdbuf[64];
//	char uptime_str[32];

// Display version information.
	telnet_printf("Hello\n");
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
	telnet_puts("> ");
	telnet_flush();

	// Get a string.
	while (telnet_gets(cmdbuf, sizeof(cmdbuf), 0, true)) {
		// Process the line as a command.
		if (!shell_command(cmdbuf))
			break;

		// Send a prompt.
		telnet_puts("> ");
		telnet_flush();
	}
}
