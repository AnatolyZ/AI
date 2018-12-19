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
#include "tim.h"
#include "usart.h"

void StartProcessTask(void const * argument);
void CommandProcess();

#endif /* PROCESSUART_H_ */
