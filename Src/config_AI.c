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
MPI_ADDR_ADDR,
PORT_ADDR,
MASK_02_01_ADDR,
MASK_04_03_ADDR,
GATE_02_01_ADDR,
GATE_04_03_ADDR,
SN_MS_ADDR,
SN_LS_ADDR,
MAC_02_01_ADDR,
MAC_04_03_ADDR,
MAC_06_05_ADDR };

void FlashInit() {

	uint16_t mem_key;
	EE_ReadVariable(MEM_KEY_ADDR, &mem_key); /* Memory key reading */
	if ((mem_key != MEMORY_KEY) || HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3) == GPIO_PIN_SET) { /* First launching of device */
		/* Default FLASH initialization --------------------- */
		EE_WriteVariable(MEM_KEY_ADDR, MEMORY_KEY);
		EE_WriteVariable(IP_02_01_ADDR, DEFAULT_IP_02_01);
		EE_WriteVariable(IP_04_03_ADDR, DEFAULT_IP_04_03);
		EE_WriteVariable(BR_MS_ADDR, DEFAULT_BR_MS);
		EE_WriteVariable(BR_LS_ADDR, DEFAULT_BR_LS);
		EE_WriteVariable(MPI_ADDR_ADDR, DEFAULT_MPI_ADDR);
		EE_WriteVariable(PORT_ADDR, DEFAULT_PORT);
		EE_WriteVariable(MASK_02_01_ADDR, DEFAULT_MASK_02_01);
		EE_WriteVariable(MASK_04_03_ADDR, DEFAULT_MASK_04_03);
		EE_WriteVariable(GATE_02_01_ADDR, DEFAULT_GATE_02_01);
		EE_WriteVariable(GATE_04_03_ADDR, DEFAULT_GATE_04_03);
		if (mem_key != MEMORY_KEY){
			EE_WriteVariable(SN_MS_ADDR, DEFAULT_SN_MS);
			EE_WriteVariable(SN_LS_ADDR, DEFAULT_SN_LS);
		}
		EE_WriteVariable(MAC_02_01_ADDR, DEFAULT_MAC_02_01);
		EE_WriteVariable(MAC_04_03_ADDR, DEFAULT_MAC_04_03);
		EE_WriteVariable(MAC_06_05_ADDR, DEFAULT_MAC_06_05);
		EE_WriteVariable(WEB_PORT_ADDR, DEFAULT_WEB_PORT);
		/* ---------------------------------------------- */
		for (int i = 0; i < 20; i++) {
			volatile register int j = 500000;
			while (j) {
				j--;
			}
			HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_10);
			j = 500000;
			while (j) {
				j--;
			}
			HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_9);
		}
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
	hflash.speed = tmp_u.ui32;
	/* IP-address */
	EE_ReadVariable(IP_02_01_ADDR, &tmp_u.ui16[0]);
	EE_ReadVariable(IP_04_03_ADDR, &tmp_u.ui16[1]);
	hflash.IP_addr[0] = tmp_u.ui8[0];
	hflash.IP_addr[1] = tmp_u.ui8[1];
	hflash.IP_addr[2] = tmp_u.ui8[2];
	hflash.IP_addr[3] = tmp_u.ui8[3];
	/* MPI/Profibus address */
	EE_ReadVariable(MPI_ADDR_ADDR, &hflash.own_addr);
	/* Subnet mask */
	EE_ReadVariable(MASK_02_01_ADDR, &tmp_u.ui16[0]);
	EE_ReadVariable(MASK_04_03_ADDR, &tmp_u.ui16[1]);
	hflash.mask[0] = tmp_u.ui8[0];
	hflash.mask[1] = tmp_u.ui8[1];
	hflash.mask[2] = tmp_u.ui8[2];
	hflash.mask[3] = tmp_u.ui8[3];
	/* Gateway */
	EE_ReadVariable(GATE_02_01_ADDR, &tmp_u.ui16[0]);
	EE_ReadVariable(GATE_04_03_ADDR, &tmp_u.ui16[1]);
	hflash.gate[0] = tmp_u.ui8[0];
	hflash.gate[1] = tmp_u.ui8[1];
	hflash.gate[2] = tmp_u.ui8[2];
	hflash.gate[3] = tmp_u.ui8[3];
	/* Port number */
	EE_ReadVariable(PORT_ADDR, &hflash.port);
	/* Web port number */
	EE_ReadVariable(WEB_PORT_ADDR, &hflash.web_port);
	/* Serial number */
	EE_ReadVariable(SN_MS_ADDR, &tmp_u.ui16[1]);
	EE_ReadVariable(SN_LS_ADDR, &tmp_u.ui16[0]);
	hflash.serial_num = tmp_u.ui32;
	/* MAC address */
	EE_ReadVariable(MAC_02_01_ADDR, &tmp_u.ui16[0]);
	EE_ReadVariable(MAC_04_03_ADDR, &tmp_u.ui16[1]);
	hflash.mac_addr[0] = tmp_u.ui8[0];
	hflash.mac_addr[1] = tmp_u.ui8[1];
	hflash.mac_addr[2] = tmp_u.ui8[2];
	hflash.mac_addr[3] = tmp_u.ui8[3];
	EE_ReadVariable(MAC_06_05_ADDR, &tmp_u.ui16[0]);
	hflash.mac_addr[4] = tmp_u.ui8[0];
	hflash.mac_addr[5] = tmp_u.ui8[1];

	hflash.ver[0] = VER1;
	hflash.ver[1] = VER2;
	hflash.ver[2] = VER3;
	/* ------------------------------------------------------- */
}

void SaveFash(void) {
	union {
		uint32_t ui32;
		uint16_t ui16[2];
		uint8_t ui8[4];
	} tmp_u;

	/*baud rate */

	tmp_u.ui32 = hflash.speed ;
	EE_WriteVariable(BR_MS_ADDR, tmp_u.ui16[1]);
	EE_WriteVariable(BR_LS_ADDR, tmp_u.ui16[0]);
	EE_WriteVariable(BR_MS_ADDR, tmp_u.ui16[1]);
	EE_WriteVariable(BR_LS_ADDR, tmp_u.ui16[0]);
	/* IP-address */
	tmp_u.ui8[0] = hflash.IP_addr[0];
	tmp_u.ui8[1] = hflash.IP_addr[1];
	tmp_u.ui8[2] = hflash.IP_addr[2];
	tmp_u.ui8[3] = hflash.IP_addr[3];
	EE_WriteVariable(IP_02_01_ADDR, tmp_u.ui16[0]);
	EE_WriteVariable(IP_04_03_ADDR, tmp_u.ui16[1]);
	EE_WriteVariable(IP_02_01_ADDR, tmp_u.ui16[0]);
	EE_WriteVariable(IP_04_03_ADDR, tmp_u.ui16[1]);
	/* MPI/Profibus address */
	EE_WriteVariable(MPI_ADDR_ADDR, hflash.own_addr);
	EE_WriteVariable(MPI_ADDR_ADDR, hflash.own_addr);
	/* Subnet mask */
	tmp_u.ui8[0] = hflash.mask[0];
	tmp_u.ui8[1] = hflash.mask[1];
	tmp_u.ui8[2] = hflash.mask[2];
	tmp_u.ui8[3] = hflash.mask[3];
	EE_WriteVariable(MASK_02_01_ADDR, tmp_u.ui16[0]);
	EE_WriteVariable(MASK_04_03_ADDR, tmp_u.ui16[1]);
	EE_WriteVariable(MASK_02_01_ADDR, tmp_u.ui16[0]);
	EE_WriteVariable(MASK_04_03_ADDR, tmp_u.ui16[1]);
	/* Gateway */
	tmp_u.ui8[0] = hflash.gate[0];
	tmp_u.ui8[1] = hflash.gate[1];
	tmp_u.ui8[2] = hflash.gate[2];
	tmp_u.ui8[3] = hflash.gate[3];
	EE_WriteVariable(GATE_02_01_ADDR, tmp_u.ui16[0]);
	EE_WriteVariable(GATE_04_03_ADDR, tmp_u.ui16[1]);
	EE_WriteVariable(GATE_02_01_ADDR, tmp_u.ui16[0]);
	EE_WriteVariable(GATE_04_03_ADDR, tmp_u.ui16[1]);
	/* Port number */
	EE_WriteVariable(PORT_ADDR, hflash.port);
	EE_WriteVariable(PORT_ADDR, hflash.port);
	/* Web-port number */
	EE_WriteVariable(WEB_PORT_ADDR, hflash.web_port);
	EE_WriteVariable(WEB_PORT_ADDR, hflash.web_port);
	/* Serial number */
	tmp_u.ui32 = hflash.serial_num;
	EE_WriteVariable(SN_MS_ADDR, tmp_u.ui16[1]);
	EE_WriteVariable(SN_LS_ADDR, tmp_u.ui16[0]);
	EE_WriteVariable(SN_MS_ADDR, tmp_u.ui16[1]);
	EE_WriteVariable(SN_LS_ADDR, tmp_u.ui16[0]);
	/* MAC address */
	tmp_u.ui8[0] = hflash.mac_addr[0];
	tmp_u.ui8[1] = hflash.mac_addr[1];
	tmp_u.ui8[2] = hflash.mac_addr[2];
	tmp_u.ui8[3] = hflash.mac_addr[3];
	EE_WriteVariable(MAC_02_01_ADDR, tmp_u.ui16[0]);
	EE_WriteVariable(MAC_04_03_ADDR, tmp_u.ui16[1]);
	EE_WriteVariable(MAC_02_01_ADDR, tmp_u.ui16[0]);
	EE_WriteVariable(MAC_04_03_ADDR, tmp_u.ui16[1]);
	tmp_u.ui8[0] = hflash.mac_addr[4];
	tmp_u.ui8[1] = hflash.mac_addr[5];
	EE_WriteVariable(MAC_06_05_ADDR, tmp_u.ui16[0]);
	EE_WriteVariable(MAC_06_05_ADDR, tmp_u.ui16[0]);
}
