/*
 * circbuff.h
 *
 *  Created on: 20 дек. 2018 г.
 *      Author: AZharkov
 */

#ifndef CIRCBUFF_H_
#define CIRCBUFF_H_

#include "stm32f407xx.h"
#include "FreeRTOS.h"
#include "error_type.h"


typedef struct{
	uint8_t *buf;
	uint32_t size;
	uint32_t read_pos;
	uint32_t write_pos;
} circbuff;

error_t CB_Init(circbuff *cb, uint32_t buf_size);
uint32_t CB_Data_Length(circbuff *cb);
error_t CB_Write(circbuff *cb, uint8_t item);
error_t CB_Read(circbuff *cb, uint8_t * item);


#endif /* CIRCBUFF_H_ */
