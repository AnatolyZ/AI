/*
 * config_IA.h
 *
 *  Created on: 12 дек. 2018 г.
 *      Author: AZharkov
 */

#ifndef CONFIG_IA_H_
#define CONFIG_IA_H_

#include "eeprom.h"

/* Firmware version: VER1.VER2.VER3 */
#define VER1 0
#define VER2 8
#define VER3 0
/* Current version 0.6.3 */

typedef struct {
	uint32_t speed;
	uint16_t own_addr;
	uint8_t IP_addr[4];
	uint8_t mask[4];
	uint8_t gate[4];
	uint16_t port;
	uint32_t serial_num;
	uint16_t ver[3];
	uint8_t mac_addr[6];
	uint16_t web_port;
} flash_data_t;


extern flash_data_t hflash;

void FlashInit();
void SaveFash(void);

#endif /* CONFIG_IA_H_ */
