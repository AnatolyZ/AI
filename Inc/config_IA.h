/*
 * config_IA.h
 *
 *  Created on: 12 дек. 2018 г.
 *      Author: AZharkov
 */

#ifndef CONFIG_IA_H_
#define CONFIG_IA_H_

#include "eeprom.h"


typedef struct {
	uint32_t speed;
	uint16_t own_addr;
	uint8_t IP_addr[4];
} flash_data_t;

extern flash_data_t hflash;

void FlashInit();

#endif /* CONFIG_IA_H_ */
