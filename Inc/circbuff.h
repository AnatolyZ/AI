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

typedef enum {
	NO_ERR,
	MEM_ERR,
	EMPTY_ERR,
	FULL_ERR,
	SIZE_ERR
} cb_err;

typedef struct{
	uint8_t *buf;
	uint32_t size;
	uint32_t read_pos;
	uint32_t write_pos;
} circbuff;

cb_err CB_Init(circbuff *cb, uint32_t buf_size);
uint32_t CB_Data_Length(circbuff *cb);
cb_err CB_Write(circbuff *cb, uint8_t item);
cb_err CB_Read(circbuff *cb, uint8_t * item);


#endif /* CIRCBUFF_H_ */
