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

#define TRANS_ON() 		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0,GPIO_PIN_SET)
#define TRANS_OFF() 	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0,GPIO_PIN_RESET)

void SendAckMsg(void);
void SendTokenMsg(uint8_t to,uint8_t from);
void SendNoDataMsg(uint8_t to, uint8_t from, uint8_t fc);
void SendRequestMsg(uint8_t to, uint8_t from, uint8_t* data, uint8_t data_len);
void SendConfirmMsg(uint8_t to, uint8_t from, uint8_t size, uint8_t func);
void SendConnectMsg(uint8_t to, uint8_t from);
void StartProcessTask(void const * argument);
void CommandProcess(void);

#endif /* PROCESSUART_H_ */
