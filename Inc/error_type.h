/*
 * error_type.h
 *
 *  Created on: 25 дек. 2018 г.
 *      Author: AZharkov
 */

#ifndef ERROR_TYPE_H_
#define ERROR_TYPE_H_

typedef enum {
	NO_ERR,
	MEM_ERR,
	EMPTY_ERR,
	FULL_ERR,
	SIZE_ERR,
	UNKNOWN_SD_ERR,
	FRAME_ERR,
	FCS_ERR
} error_t;

#endif /* ERROR_TYPE_H_ */
