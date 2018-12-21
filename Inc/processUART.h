/*
 * processUART.h
 *
 *  Created on: 18 ���. 2018 �.
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

#define UART_BUFF_SIZE 256

void StartProcessTask(void const * argument);
void CommandProcess();

#endif /* PROCESSUART_H_ */
