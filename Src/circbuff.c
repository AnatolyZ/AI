/*
 * circbuff.c
 *
 *  Created on: 20 дек. 2018 г.
 *      Author: AZharkov
 */

#include "circbuff.h"

/*Checks if argument is power of 2 --------------------------------------- */

static uint8_t is_exp_of_2(uint32_t n)
{
    return ( n & (n - 1) ) == 0;
}

/* Circular buffer initiation function ----------------------------------- */

cb_err CB_Init(circbuff *cb, uint32_t buf_size) {
	if (!is_exp_of_2(buf_size)) return SIZE_ERR;
						/* return error if buffer size is not power of 2 */
	cb->size = buf_size;
	cb->read_pos = 0;
	cb->write_pos = 0;
	cb->buf = (uint8_t*) pvPortMalloc(buf_size * sizeof(uint8_t));
	if (cb->buf == NULL) {
		return MEM_ERR;
	}
	return NO_ERR;
}

/* Returns length of data in buffer ---------------------------------------*/

uint32_t CB_Data_Length(circbuff *cb) {
	return ((cb->write_pos - cb->read_pos) & (cb->size - 1));
}

/* Write new element to buffer --------------------------------------------*/

cb_err CB_Write(circbuff *cb, uint8_t item){
	if (CB_Data_Length(cb) >= (cb->size - 1)){
		return FULL_ERR;
	}
	cb->buf[cb->write_pos] = item;
	cb->write_pos = (cb->write_pos + 1) & (cb->size - 1);
	return NO_ERR;
}

/* Read element from buffer -----------------------------------------------*/

cb_err CB_Read(circbuff *cb, uint8_t * item){
	if (CB_Data_Length(cb) == 0){
		return EMPTY_ERR;
	}
	*item = cb->buf[cb->read_pos];
	cb->read_pos = (cb->read_pos + 1) & ( cb->size - 1);
	return NO_ERR;
}
