/**
  ******************************************************************************
  * @file    EEPROM/EEPROM_Emulation/inc/eeprom.h 
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the EEPROM 
  *          emulation firmware library.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EEPROM_H
#define __EEPROM_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported constants --------------------------------------------------------*/
/* EEPROM emulation firmware error codes */
#define EE_OK      (uint32_t)HAL_OK
#define EE_ERROR   (uint32_t)HAL_ERROR
#define EE_BUSY    (uint32_t)HAL_BUSY
#define EE_TIMEOUT (uint32_t)HAL_TIMEOUT

/* Define the size of the sectors to be used */
#define PAGE_SIZE               (uint32_t)0x4000  /* Page size = 16KByte */

/* Device voltage range supposed to be [2.7V to 3.6V], the operation will 
   be done by word  */
#define VOLTAGE_RANGE           (uint8_t)VOLTAGE_RANGE_3

/* EEPROM start address in Flash */
#define EEPROM_START_ADDRESS  ((uint32_t)0x08008000) /* EEPROM emulation start address:
                                                  from sector2 : after 16KByte of used 
                                                  Flash memory */

/* Pages 0 and 1 base and end addresses */
#define PAGE0_BASE_ADDRESS    ((uint32_t)(EEPROM_START_ADDRESS + 0x0000))
#define PAGE0_END_ADDRESS     ((uint32_t)(EEPROM_START_ADDRESS + (PAGE_SIZE - 1)))
#define PAGE0_ID               FLASH_SECTOR_2

#define PAGE1_BASE_ADDRESS    ((uint32_t)(EEPROM_START_ADDRESS + 0x4000))
#define PAGE1_END_ADDRESS     ((uint32_t)(EEPROM_START_ADDRESS + (2 * PAGE_SIZE - 1)))
#define PAGE1_ID               FLASH_SECTOR_3

/* Used Flash pages for EEPROM emulation */
#define PAGE0                 ((uint16_t)0x0000)
#define PAGE1                 ((uint16_t)0x0001) /* Page nb between PAGE0_BASE_ADDRESS & PAGE1_BASE_ADDRESS*/

/* No valid page define */
#define NO_VALID_PAGE         ((uint16_t)0x00AB)

/* Page status definitions */
#define ERASED                ((uint16_t)0xFFFF)     /* Page is empty */
#define RECEIVE_DATA          ((uint16_t)0xEEEE)     /* Page is marked to receive data */
#define VALID_PAGE            ((uint16_t)0x0000)     /* Page containing valid data */

/* Valid pages in read and write defines */
#define READ_FROM_VALID_PAGE  ((uint8_t)0x00)
#define WRITE_IN_VALID_PAGE   ((uint8_t)0x01)

/* Page full define */
#define PAGE_FULL             ((uint8_t)0x80)

/* Variables' number */
#define NB_OF_VAR             ((uint8_t)20)



/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define 		MEMORY_KEY			0xFEED   /* Key for default memory initiation */
#define			DEFAULT_IP_02_01	0xA8C0   /* 192.168.004.253 - default IP */
#define			DEFAULT_IP_04_03	0xFD04   /* 111.222.333.444 - template */
#define			DEFAULT_BR_MS		0x0002   /*------------*/
#define			DEFAULT_BR_LS		0xDC6C   /* 187500 Baud*/
#define			DEFAULT_MPI_ADDR	0x0000   /* Default address for MPI/Profibus */
#define			DEFAULT_PORT		0x0066   /* Default port number */
#define			DEFAULT_MASK_02_01	0xFFFF   /* 255.255.255.000 - default mask */
#define			DEFAULT_MASK_04_03	0x00FF   /* 111.222.333.444 - template */
#define			DEFAULT_GATE_02_01	0xA8C0   /* 192.168.004.001 - default gateway */
#define			DEFAULT_GATE_04_03	0x0104   /* 111.222.333.444 - template */
#define			DEFAULT_SN_MS		0x0000   /*------------*/
#define			DEFAULT_SN_LS		0x0000   /* Default serial number*/
#define 		DEFAULT_MAC_02_01 	0x0200
#define 		DEFAULT_MAC_04_03	0x42A2	 /* Hilscher_42:50:e4 (00:02:a2:42:50:e4) */
#define 		DEFAULT_MAC_06_05 	0xE450
#define 		DEFAULT_WEB_PORT 	0xBF67   /* Default web_port address */

/* Exported functions ------------------------------------------------------- */
uint16_t EE_Init(void);
uint16_t EE_ReadVariable(uint16_t VirtAddress, uint16_t* Data);
uint16_t EE_WriteVariable(uint16_t VirtAddress, uint16_t Data);
/* Memory map ----------------------------------------------------------------*/
#define			MEM_KEY_ADDR		0
#define 		IP_02_01_ADDR 		1
#define 		IP_04_03_ADDR		2
#define 		BR_MS_ADDR 		    3
#define 		BR_LS_ADDR 		    4
#define 		MPI_ADRR_ADDR	    5
#define 		PORT_ADRR			6
#define 		MASK_02_01_ADDR		7
#define 		MASK_04_03_ADDR		8
#define 		GATE_02_01_ADDR		9
#define 		GATE_04_03_ADDR		10
#define 		SN_MS_ADDR 		    11
#define 		SN_LS_ADDR 		    12
#define 		VER_1_ADDR 		    13
#define 		VER_2_ADDR 		    14
#define 		VER_3_ADDR 		    15
#define 		MAC_02_01_ADDR 		16
#define 		MAC_04_03_ADDR		17
#define 		MAC_06_05_ADDR 		18
#define 		WEB_PORT_ADRR		19

#endif /* __EEPROM_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
