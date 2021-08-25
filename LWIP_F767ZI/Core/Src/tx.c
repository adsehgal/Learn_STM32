/*
 * tx.c
 *
 *  Created on: Jun 22, 2021
 *      Author: adityasehgal
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "cmsis_os.h"

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

#include "tx.h"

//packets to send - 1 packet per thread/task
static numsPacket_t numsPacket;
static devicePacket_t devicePacket;

// OS tasks:
osThreadId numsTaskHandle;
osThreadId deviceTaskHandle;

//OS timers:
static void Timer2_Callback(void const *arg); // prototype for timer callback function
static osTimerDef (Timer2, Timer2_Callback);
uint8_t intFlag = 0;

osSemaphoreId sid_Thread_Semaphore;                             // semaphore id
osSemaphoreDef(SampleSemaphore);// semaphore object

sys_sem_t testSem;

// Periodic Timer Example
static void Timer2_Callback(void const *arg) {
	intFlag = 1;
	osSemaphoreRelease (sid_Thread_Semaphore);
	sys_sem_signal(&testSem);
}

// Implement NUMS reporting thread.
static void numsThread(void const *argument) {
	int sock;
	struct sockaddr_in local;
	struct sockaddr_in remote;

	// Create a UDP datgram socket.
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("NUMS: cannot create socket\n");
		return;
	} else {
		printf("NUMS: socket created\n");
	}

	// Bind the socket to specified port.
	memset((char*) &local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(NUMS_PORT);
	if (bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
		printf("NUMS: cannot set socket port\n");
		return;
	} else {
		printf("NUMS: set socket port success\n");
	}

	// Specify the client to where packets are being sent.
	memset((char*) &remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_port = htons(NUMS_PORT);
	if ((inet_aton(NUMS_IP, &remote.sin_addr)) == 0) {
		printf("NUMS: cannot set remote address\n");
		return;
	} else {
		printf("NUMS: remote address set success\n");
	}

	int val = 0;
	printf("Sending from NUMS thread:\n\t");
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 8; j++) {
			numsPacket.frames[i][j] = val++;
			printf("%02X | ", numsPacket.frames[i][j]);
		}
	}
	printf("\n\n");

	for (;;) {
//		osDelay(200);
		sys_sem_wait(&testSem);
//		osSemaphoreWait(sid_Thread_Semaphore, 0);
//		while (intFlag == 0) {
//			osDelay(5);
//		}
		intFlag = 0;
		numsPacket.sequenceNum++;
		// Send the NUMS packet to the client.
		if (sendto(sock, &numsPacket, sizeof(numsPacket), 0,
				(struct sockaddr* ) &remote, sizeof(remote)) != -1) {
			HAL_GPIO_TogglePin(LEDG_GPIO_Port, LEDG_Pin);
		} else {
			HAL_GPIO_TogglePin(LEDR_GPIO_Port, LEDR_Pin);
		}
	}
}

void numsInit(void) {

	sid_Thread_Semaphore = osSemaphoreCreate(osSemaphore(SampleSemaphore), 1);
	if (!sid_Thread_Semaphore) {
		; // Semaphore object not created, handle failure
	}

	sys_sem_new(&testSem, 0);

//	osTimerCreate(osTimer(numTimer)/*&numTimerDef*/, osTimerPeriodic, numTimerCallback);
//	osTimerStart(numTimerID, 500);
	static osTimerId id2;                                           // timer id
	static uint32_t exec2;          // argument for the timer call back function
	osStatus status;                                   // function return status
	exec2 = 2;
	id2 = osTimerCreate(osTimer(Timer2), osTimerPeriodic, &exec2);
	if (id2 != NULL) {    // Periodic timer created
		// start timer with periodic 1000ms interval
		status = osTimerStart(id2, 500);
		if (status != osOK) {
			// Timer could not be started
		}
	}
	osThreadDef(numsTask, numsThread, osPriorityNormal, 0, 2048);
	numsTaskHandle = osThreadCreate(osThread(numsTask), NULL);

}

// Implement NUMS reporting thread.
static void deviceThread(void const *argument) {
	int sock;
	struct sockaddr_in local;
	struct sockaddr_in remote;

	// Create a UDP datgram socket.
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("DEVICE: cannot create socket\n");
		return;
	} else {
		printf("DEVICE: socket created\n");
	}

	// Bind the socket to specified port.
	memset((char*) &local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(DEVICE_PORT);
	if (bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
		printf("DEVICE: cannot set socket port\n");
		return;
	} else {
		printf("DEVICE: set socket port success\n");
	}

	// Specify the client to where packets are being sent.
	memset((char*) &remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_port = htons(DEVICE_PORT);
	if ((inet_aton(DEVICE_IP, &remote.sin_addr)) == 0) {
		printf("DEVICE: cannot set remote address\n");
		return;
	} else {
		printf("DEVICE: remote address set success\n");
	}

	devicePacket.halVersion = HAL_GetHalVersion();
	devicePacket.deviceRevID = HAL_GetREVID();
	devicePacket.deviceID = HAL_GetDEVID();
	devicePacket.deviceUID0 = HAL_GetUIDw0();
	devicePacket.deviceUID1 = HAL_GetUIDw1();
	devicePacket.deviceUID2 = HAL_GetUIDw2();
	devicePacket.halTick = HAL_GetTick();

	printf("Sending from DEVICE thread:\n");
	printf("\tHAL Version: %ld\n", devicePacket.halVersion);
	printf("\t Device Rev ID: %ld\n", devicePacket.deviceRevID);
	printf("\tDevice ID: %ld\n", devicePacket.deviceID);
	printf("\tDevice UID0: %ld\n", devicePacket.deviceUID0);
	printf("\tDevice UID1: %ld\n", devicePacket.deviceUID1);
	printf("\tDevice UID2: %ld\n", devicePacket.deviceUID2);
	printf("\tHAL Tick: %ld\n", devicePacket.halTick);
	printf("\n\n");

	for (;;) {
		osDelay(100);
		devicePacket.sequenceNum++;
		devicePacket.halVersion = HAL_GetHalVersion();
		devicePacket.deviceRevID = HAL_GetREVID();
		devicePacket.deviceID = HAL_GetDEVID();
		devicePacket.deviceUID0 = HAL_GetUIDw0();
		devicePacket.deviceUID1 = HAL_GetUIDw1();
		devicePacket.deviceUID2 = HAL_GetUIDw2();
		devicePacket.halTick = HAL_GetTick();

		// Send the NUMS packet to the client.
		if (sendto(sock, &devicePacket, sizeof(devicePacket), 0,
				(struct sockaddr* ) &remote, sizeof(remote)) != -1) {
			HAL_GPIO_TogglePin(LEDG_GPIO_Port, LEDG_Pin);
		} else {
			HAL_GPIO_TogglePin(LEDR_GPIO_Port, LEDR_Pin);
		}
	}
}
void deviceInit(void) {
	osThreadDef(deviceTask, deviceThread, osPriorityNormal, 0, 1024);
	deviceTaskHandle = osThreadCreate(osThread(deviceTask), NULL);
}
