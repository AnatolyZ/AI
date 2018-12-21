/*
 * log.h
 *
 *  Created on: 21 дек. 2018 г.
 *      Author: AZharkov
 */

#ifndef LOG_H_
#define LOG_H_

typedef enum{
	SUB_SYS_MEMORY,
	SUB_SYS_WEB,
	SUB_SYS_RS458,
	SUB_SYS_TCP,
	NUM_OF_SUB_SYS
} log_sub_sys;

typedef enum{
	LOG_LEV_OFF,
	LOG_LEV_ERR,
	LOG_LEV_WARN,
	LOG_LEV_INFO,
	NUM_LOG_LEV
} log_level;

typedef struct{
	UART_HandleTypeDef *interface;
	uint8_t levels [NUM_OF_SUB_SYS];
} log_handler;

void LogInit(UART_HandleTypeDef * log_if);
void LogText(log_sub_sys sys, log_level level, char *msg);
void LogNum(log_sub_sys sys, int number);
void LogTextNum(log_sub_sys sys, log_level level, char *msg, int number);
void LogSetOutputLevel(log_sub_sys sys, log_level level);
void LogGlobalOn();
void LogGlobalOff();

#endif /* LOG_H_ */
