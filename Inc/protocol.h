/*
 * protocol.h
 *
 *  Created on: 25 ���. 2018 �.
 *      Author: AZharkov
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "stm32f4xx_hal.h"
#include "error_type.h"
#include "config_IA.h"
#include "FreeRTOS.h"
#include "string.h"
#include "processUART.h"

typedef struct {
	uint8_t SD; /*Start Delimiter*/
	uint8_t LE; /*Length of protocol data unit, (incl. DA,SA,FC,DSAP,SSAP)*/
	uint8_t LEr; /*Repetition of length of protocol data unit, (Hamming distance = 4)*/
	uint8_t DA; /*Destination Address*/
	uint8_t SA; /*Source Address*/
	uint8_t FC; /*Function Code*/
	uint8_t DSAP; /*Destination Service Access Point*/
	uint8_t SSAP; /*Source Service Access Point*/
	uint8_t UK1; /*Unknown parameter1*/
	uint8_t UK2; /*Unknown parameter2*/
	uint8_t RN; /*Request number*/
	uint8_t* PDU; /*Protocol Data Unit (protocol data)*/
	uint8_t FCS; /*Frame Checking Sequence, calculated by simply adding up the bytes within the specified length. An overflow is ignored here.*/
	uint8_t ED; /*End Delimiter (= 0x16 !)*/
} telegram_t;

typedef struct {
	uint8_t own_address; /* Device bus address */
	volatile uint8_t token_possession; /* Has or not token, if "must_answer" = 0 give token back */
	volatile uint8_t have_data_to_send; /* Must send an answer */
	uint8_t wait_for_answer; /* Request was sent, waiting for answer */
	uint32_t speed; /* Baud rate */
	uint8_t* data_ptr; /* Pointer to data to send */
	volatile uint8_t data_len; /* Data length */
} profibus_MPI_t;

extern profibus_MPI_t hprot;

void ProtocolSettingsInit(profibus_MPI_t* hp);
uint8_t CalculateFCS(uint8_t * buf, uint8_t len);
error_t CommandParser(uint8_t *buf);

#endif /* PROTOCOL_H_ */
