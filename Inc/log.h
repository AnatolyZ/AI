/*
 * log.h
 *
 *  Created on: 21 дек. 2018 г.
 *      Author: AZharkov
 */

#ifndef LOG_H_
#define LOG_H_

#include "usart.h"
#include "string.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stdlib.h"


#define LOG_TIMEOUT 20U

#ifdef LOG_ON
#define LogText(inf,sys, level, msg) LogText__((inf), (sys), (level), (uint8_t*)(msg))
#define LogNum(inf, sys, level, num, base) LogNum__((inf), (sys), (level), (num), (base))
#else
#define LogText(inf, sys, level, msg)
#define LogNum(inf, sys, level, num, base)
#endif /* LOG_ON */

typedef enum {
	INFO_HIDE,
	INFO_SHOW
} info_presence;

typedef enum {
	SUB_SYS_MEMORY,
	SUB_SYS_WEB,
	SUB_SYS_RS458,
	SUB_SYS_TCP,
	SUB_SYS_LOG,
	NUM_OF_SUB_SYS
} log_sub_sys;

typedef enum {
	LOG_LEV_OFF,
	LOG_LEV_ERR,
	LOG_LEV_WARN,
	LOG_LEV_INFO,
	LOG_LEV_ALL,
	NUM_LOG_LEV
} log_level;

typedef struct {
	UART_HandleTypeDef *interface;
	uint8_t levels[NUM_OF_SUB_SYS];
	xSemaphoreHandle mutex;
} log_handler;

extern log_handler hlog;


void LogInit(UART_HandleTypeDef * log_if);
void LogText__(info_presence info,log_sub_sys sys, log_level level, uint8_t *msg);
void LogNum__(info_presence info,log_sub_sys sys, log_level level, int number, uint8_t base);
void LogSetOutputLevel(log_sub_sys sys, log_level level);
void LogGlobalOn();
void LogGlobalOff();

#endif /* LOG_H_ */
