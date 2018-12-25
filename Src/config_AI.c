/*
 * config_AI.c
 *
 *  Created on: 12 дек. 2018 г.
 *      Author: AZharkov
 */

#include "config_IA.h"

flash_data_t hflash;

extern UART_HandleTypeDef huart5;

uint16_t VirtAddVarTab[NB_OF_VAR] = {
MEM_KEY_ADDR,
IP_02_01_ADDR,
IP_04_03_ADDR,
BR_MS_ADDR,
BR_LS_ADDR,
MPI_ADRR_ADDR,
RESERVED01,
RESERVED02,
RESERVED03,
RESERVED04, };

void FlashInit() {

	uint16_t mem_key;
	EE_ReadVariable(MEM_KEY_ADDR, &mem_key); /* Memory key reading */
	if (mem_key != MEMORY_KEY) { /* First launching of device */
		/* Default FLASH initialization --------------------- */
		EE_WriteVariable(MEM_KEY_ADDR, MEMORY_KEY);
		EE_WriteVariable(IP_02_01_ADDR, DEFAULT_IP_02_01);
		EE_WriteVariable(IP_04_03_ADDR, DEFAULT_IP_04_03);
		EE_WriteVariable(BR_MS_ADDR, DEFAULT_BR_MS);
		EE_WriteVariable(BR_LS_ADDR, DEFAULT_BR_LS);
		EE_WriteVariable(MPI_ADRR_ADDR, DEFAULT_MPI_ADDR);
		/* --------------------------------------------------- */
	}
	/* Variables initialization ------------------------------ */
	union {
		uint32_t ui32;
		uint16_t ui16[2];
		uint8_t ui8[4];
	} tmp_u;

	/*baud rate */
	EE_ReadVariable(BR_MS_ADDR, &tmp_u.ui16[1]);
	EE_ReadVariable(BR_LS_ADDR, &tmp_u.ui16[0]);
	huart5.Init.BaudRate = tmp_u.ui32;
	hflash.speed = huart5.Init.BaudRate;
    /* IP-address */
	EE_ReadVariable(IP_02_01_ADDR, &tmp_u.ui16[0]);
	EE_ReadVariable(IP_04_03_ADDR, &tmp_u.ui16[1]);
	hflash.IP_addr[0] = tmp_u.ui8[0];
	hflash.IP_addr[1] = tmp_u.ui8[1];
	hflash.IP_addr[2] = tmp_u.ui8[2];
	hflash.IP_addr[3] = tmp_u.ui8[3];
	/* MPI/Profibus address */
	EE_ReadVariable(MPI_ADRR_ADDR, &hflash.own_addr);
	/* ------------------------------------------------------- */
}
