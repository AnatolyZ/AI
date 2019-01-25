/*
 * json.h
 *
 *  Created on: 22 џэт. 2019 у.
 *      Author: AZharkov
 */

#ifndef JSON_H_
#define JSON_H_

/* Macro definitions */
#define MAXNUMBER_OF_TOKENS 32
#define MAX_TOKEN_LENGTH 25
/* ----------------- */

/* Includes */
#include "stm32f4xx_hal.h"
#include "string.h"
#include "stdlib.h"
#include "config_IA.h"
#include "jsmn.h"
#include "lwip.h"
/* -------- */

/* Type definitions */
typedef struct json_data {
	uint8_t ip_addr[16];
	uint8_t mask[16];
	uint8_t gate[16];
	uint16_t port;
	uint32_t speed;
	uint8_t own_addr;
	uint8_t ver[12];
	uint32_t serial_num;
	uint8_t mac_addr[18];
} json_data_t;
/* ------------------- */

/* Global variables for export */
extern json_data_t hjsondata;
/* --------------------------- */

/* Function prototypes */
uint GetJSONData(uint8_t *pstr);
void FlashToJSON(json_data_t *js,flash_data_t *fs);
void ParseJSON(json_data_t *js,uint8_t *json_str);
void DecodeURL(uint8_t *buf, uint8_t *result);
void JSONToFlash(json_data_t *js,flash_data_t *fs);
/* ------------------- */

#endif /* JSON_H_ */
