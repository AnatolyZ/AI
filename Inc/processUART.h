/*
 * processUART.h
 *
 *  Created on: 18 дек. 2018 г.
 *      Author: AZharkov
 */

#ifndef PROCESSUART_H_
#define PROCESSUART_H_

#include "FreeRTOS.h"
#include "stm32f4xx_hal.h"
#include "queue.h"
#include "semphr.h"
#include "tim.h"
#include "usart.h"
#include "circbuff.h"
#include "protocol.h"

#define UART_BUFF_SIZE 256
#define SIZE_OF_CMD_BUF 256


void StartProcessTask(void const * argument);
void CommandProcess(void);

#endif /* PROCESSUART_H_ */
