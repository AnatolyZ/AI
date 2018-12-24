/*
 * log.c
 *
 *  Created on: 21 дек. 2018 г.
 *      Author: AZharkov
 */

#include "log.h"

log_handler hlog;

static char* txt_lev[NUM_LOG_LEV - 1] = { "OFF", "ERR", "WAR", "INF", };

static char* txt_sys[NUM_OF_SUB_SYS] = { "MEM", "WEB", "458", "TCP", };

void LogInit(UART_HandleTypeDef * log_if) {
	hlog.interface = log_if;
	for (register int i = 0; i < NUM_OF_SUB_SYS; i++) {
		hlog.levels[i] = LOG_LEV_ALL;
	}
}

void LogText(log_sub_sys sys, log_level level, char *msg) {
	if (level <= hlog.levels[sys]
			&& hlog.interface->gState == HAL_UART_STATE_READY) {
		uint len = 0;
		char * ptr = msg;
		while (*ptr++) {
			len++;
		}
		ptr = (char*) pvPortMalloc((size_t) (len + 10));
		if (ptr != NULL) {
			strcpy(ptr, txt_sys[sys]);
			strcpy(ptr + 3, "_");
			strcpy(ptr + 4, txt_lev[level]);
			strcpy(ptr + 7, ":");
			strcpy(ptr + 8, msg);
			HAL_UART_Transmit_DMA(hlog.interface, (uint8_t*) ptr, len + 8);
			vPortFree(ptr);
		}
	}
}

void LogNum(log_sub_sys sys, log_level level, int number) {
	if (level <= hlog.levels[sys]
			&& hlog.interface->gState == HAL_UART_STATE_READY) {
		uint len = 0;
		char * ptr;
		ptr = (char*) pvPortMalloc(30);
		if (ptr != NULL) {
			strcpy(ptr, txt_sys[sys]);
			strcpy(ptr + 3, "_");
			strcpy(ptr + 4, txt_lev[level]);
			strcpy(ptr + 7, ":");
			itoa(number, ptr + 8, 10);
			while (*ptr++) {
				len++;
			}
			ptr -= len + 1;
			strcpy(ptr + len, "\r\n");
			HAL_UART_Transmit_DMA(hlog.interface, (uint8_t*) ptr, len + 2);
			vPortFree(ptr);
		}
	}
}

void LogTextNum(log_sub_sys sys, log_level level, char *msg, int number) {
	if (level <= hlog.levels[sys]
			&& hlog.interface->gState == HAL_UART_STATE_READY) {
		uint len = 0;
		uint msg_len = 0;
		char * ptr = msg;
		while (*ptr++) {
			msg_len++;
		}
		ptr -= msg_len + 1;
		ptr = (char*) pvPortMalloc(msg_len + 30);
		if (ptr != NULL) {
			strcpy(ptr, txt_sys[sys]);
			strcpy(ptr + 3, "_");
			strcpy(ptr + 4, txt_lev[level]);
			strcpy(ptr + 7, ":");
			strcpy(ptr + 8, msg);
			itoa(number, ptr + 8 + msg_len, 10);
			while (*ptr++) {
				len++;
			}
			ptr -= len + 1;
			strcpy(ptr + len, "\r\n");
			HAL_UART_Transmit_DMA(hlog.interface, (uint8_t*) ptr, len + 2);
			vPortFree(ptr);
		}
	}
}

void LogSetOutputLevel(log_sub_sys sys, log_level level) {
	hlog.levels[sys] = level;
}

void LogGlobalOn() {
	for (register int i = 0; i < NUM_OF_SUB_SYS; i++) {
		hlog.levels[i] = LOG_LEV_ALL;
	}
}

void LogGlobalOff() {
	for (register int i = 0; i < NUM_OF_SUB_SYS; i++) {
		hlog.levels[i] = LOG_LEV_OFF;
	}
}
