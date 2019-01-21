/*
 * protocol.h
 *
 *  Created on: 25 дек. 2018 г.
 *      Author: AZharkov
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

/* Includes */
#include "stm32f4xx_hal.h"
#include "error_type.h"
#include "config_IA.h"
#include "FreeRTOS.h"
#include "string.h"
#include "processUART.h"
#include "tcp_client.h"
/* -------- */

/* Type definitions */
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

typedef enum conn_status{
	CONN_NO,
	CONN_REQ_SENT,
	CONN_OK,
	CONN_AGAIN,
	CONN_CLOSE,
	CONN_CLOSED
}conn_status_t;

typedef enum conf_status{
	CONF_NEED07,
	CONF_NEED08,
	CONF_SENT,
	CONF_OK,
	CONF_NEED07_AGAIN
}conf_status_t;

typedef struct {
	uint8_t own_address; /* Device bus address */
	volatile uint8_t token_possession; /* Has or not token, if "must_answer" = 0 give token back */
	volatile conf_status_t confirm_status; /* Must send an confirmation*/
	volatile conn_status_t conn_stat; /* Connection status*/
	uint8_t wait_for_answer; /* Request was sent, waiting for answer */
	uint32_t speed; /* Baud rate */
	uint8_t* data_ptr; /* Pointer to data to send */
	volatile uint8_t data_len; /* Data length */
	volatile uint8_t master_address; /* Master address */
	volatile uint8_t master_SAP; /* Master SAP */
	uint8_t req_num;
} profibus_MPI_t;
/* ------------------- */

/* Global variables for export */
extern xQueueHandle protocol_queue;
extern profibus_MPI_t hprot;
/* --------------------------- */

/* Function prototypes */
void ProtocolSettingsInit(profibus_MPI_t* hp);
uint8_t CalculateFCS(uint8_t * buf, uint8_t len);
error_t CommandParser(uint8_t *buf);
/* ------------------- */

#endif /* PROTOCOL_H_ */
