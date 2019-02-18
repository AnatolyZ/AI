/*
 * log.c
 *
 *  Created on: 21 дек. 2018 г.
 *      Author: AZharkov
 */

#include "log.h"

log_handler hlog;

static char* txt_lev[NUM_LOG_LEV - 1] = { "OFF", "ERR", "WAR", "INF", };

static char* txt_sys[NUM_OF_SUB_SYS] = { "MEM", "WEB", "458", "TCP", "LOG" };

void LogInit(UART_HandleTypeDef * log_if) {
	hlog.interface = log_if;
	for (register int i = 0; i < NUM_OF_SUB_SYS; i++) {
		hlog.levels[i] = LOG_LEV_ALL;
	}
	hlog.mutex = xSemaphoreCreateMutex();
}

void LogText__(info_presence info, log_sub_sys sys, log_level level,
		uint8_t *msg) {
	if (level <= hlog.levels[sys]) {
		uint8_t info_buf[8];
		uint len = 0;
		uint8_t * ptr = msg;
		while (*ptr++) {
			len++;
		}
		if (info == INFO_SHOW) {
			strcpy((char*) info_buf, txt_sys[sys]);
			strcpy((char*) info_buf + 3, "_");
			strcpy((char*) info_buf + 4, txt_lev[level]);
			strcpy((char*) info_buf + 7, ":");
		}
		xSemaphoreTake(hlog.mutex, LOG_TIMEOUT*3);
		{
			if (info == INFO_SHOW) {
				HAL_UART_Transmit(hlog.interface, info_buf, 8, LOG_TIMEOUT);
			}
			HAL_UART_Transmit(hlog.interface, msg, len, LOG_TIMEOUT);
		}
		xSemaphoreGive(hlog.mutex);
	}
}

void LogNum__(info_presence info, log_sub_sys sys, log_level level, int number,
		uint8_t base) {
	if (level <= hlog.levels[sys]) {
		uint8_t info_buf[8];
		uint8_t str_num[33];
		if (info == INFO_SHOW) {
			strcpy((char*) info_buf, txt_sys[sys]);
			strcpy((char*) info_buf + 3, "_");
			strcpy((char*) info_buf + 4, txt_lev[level]);
			strcpy((char*) info_buf + 7, ":");
		}
		itoa(number, (char*) str_num, base);
		xSemaphoreTake(hlog.mutex, LOG_TIMEOUT*3);
		{
			if (info == INFO_SHOW) {
				HAL_UART_Transmit(hlog.interface, info_buf, 8, LOG_TIMEOUT);
			}
			HAL_UART_Transmit(hlog.interface, (uint8_t*) str_num, strlen((char*)str_num),LOG_TIMEOUT);
		}
		xSemaphoreGive(hlog.mutex);
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
