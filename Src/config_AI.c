/*
 * config_AI.c
 *
 *  Created on: 12 дек. 2018 г.
 *      Author: AZharkov
 */

#include "config_IA.h"

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

void init_FLASH_AI() {
	uint16_t mem_key;
	EE_ReadVariable(MEM_KEY_ADDR, &mem_key);
	if (mem_key != MEMORY_KEY) {
		EE_WriteVariable(MEM_KEY_ADDR, MEMORY_KEY);
		EE_WriteVariable(IP_02_01_ADDR, DEFAULT_IP_02_01);
		EE_WriteVariable(IP_04_03_ADDR, DEFAULT_IP_04_03);
		EE_WriteVariable(BR_MS_ADDR, DEFAULT_BR_MS);
		EE_WriteVariable(BR_LS_ADDR, DEFAULT_BR_LS);
		EE_WriteVariable(MPI_ADRR_ADDR, DEFAULT_MPI_ADDR);
	} else {
		union {
			uint32_t ui32;
			uint16_t ui16[2];
		} br;

		EE_ReadVariable(BR_MS_ADDR,&br.ui16[1]);
		EE_ReadVariable(BR_LS_ADDR,&br.ui16[0]);
		huart5.Init.BaudRate = br.ui32;
}
}
