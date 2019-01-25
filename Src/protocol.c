/*
 * protocol.c
 *
 *  Created on: 25 дек. 2018 г.
 *      Author: AZharkov
 */
/* Includes */
#include "protocol.h"
/* -------- */

/* Global variables */
xQueueHandle protocol_queue;
/* ---------------- */

/* Function prototypes */
static inline error_t TokenCmdProcessing(telegram_t * tel);
static inline error_t NoDataCmdProcessing(telegram_t * tel);
static inline error_t VarDataCmdProcessing(telegram_t * tel);
static inline error_t FixDataCmdProcessing(telegram_t * tel);
/* ------------------- */

/* ---------- FUNCTIONS ------------ */
/* -------||--||--||--||--||-------- */
/* -------\/--\/--\/--\/--\/-------- */

void ProtocolSettingsInit(profibus_MPI_t* hp) {
	hp->own_address = hflash.own_addr;
	hp->speed = hflash.speed;
	hp->token_possession = 0U;
	hp->confirm_status = CONF_OK;
	hp->conn_stat = CONN_NO;
	hp->wait_for_answer = 0U;
	hp->data_ptr = NULL;
	hp->data_len = 0U;
}

uint8_t CalculateFCS(uint8_t * buf, uint8_t len) {
	uint8_t result = 0;
	while (len--) {
		result += *buf++;
	}
	return result;
}

static inline error_t TokenCmdProcessing(telegram_t * tel) {
	parcel_t parc;
	if ((hprot.conn_stat == CONN_CLOSE) && (hprot.confirm_status != CONF_NEED08)) {
		SendClosemMsg(tel->SA, hprot.own_address);
	} else if (hprot.confirm_status == CONF_NEED07) {
		SendConfirmMsg(tel->SA, hprot.own_address, 0x07, 0x5C);
	} else if (hprot.confirm_status == CONF_NEED08) {
		SendConfirmMsg(tel->SA, hprot.own_address, 0x08, 0x5C);
	} else if (hprot.confirm_status == CONF_NEED07_AGAIN) {
		SendConfirmMsg(tel->SA, hprot.own_address, 0x07, 0x7C);
	} else {
		if (xQueuePeek(tcp_client_queue,&parc,0) != pdPASS) {
			SendTokenMsg(tel->SA, hprot.own_address);
			hprot.token_possession = 0U;
		} else {
			hprot.token_possession = 1U;
			if (hprot.conn_stat == CONN_OK) {
				parc.data = NULL;
				xQueueReceive(tcp_client_queue, &parc, 0);
				SendRequestMsg(tel->SA, hprot.own_address, parc.data, parc.len);
				if (parc.data != NULL) {
					vPortFree(parc.data);
				}
			} else if (hprot.conn_stat == CONN_NO) {
				SendConnectMsg(tel->SA, hprot.own_address, 0x6D);
			} else if (hprot.conn_stat == CONN_AGAIN) {
				SendConnectMsg(tel->SA, hprot.own_address, 0x5D);
			} else {
				SendTokenMsg(tel->SA, hprot.own_address);
				hprot.token_possession = 0U;
			}
		}
	}
	return NO_ERR;
}

static inline error_t NoDataCmdProcessing(telegram_t * tel) {
	if (tel->FC == 0x49) {
		SendNoDataMsg(tel->SA, tel->DA, 0x20);
		hprot.master_address = tel->SA;

		if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
		  {
		    Error_Handler();
		  }

	}
	return NO_ERR;
}

static inline error_t VarDataCmdProcessing(telegram_t * tel) {

	if (tel->UK1 == 0xD0) {
		if (tel->FC == 0x6C) {
			hprot.confirm_status = CONF_NEED07;
			hprot.master_SAP = tel->SSAP;
			SendAckMsg();
		}
		if (tel->FC == 0x5C) {
			hprot.confirm_status = CONF_NEED07_AGAIN;
			hprot.master_SAP = tel->SSAP;
			SendAckMsg();
		}
	} else if (tel->UK1 == 0x05) {
		hprot.confirm_status = CONF_OK;
		hprot.conn_stat = CONN_OK;
		SendAckMsg();
	} else if (tel->UK1 == 0xB0) {
		SendAckMsg();
	} else if (tel->UK1 == 0xC0){
		/* No ACK */
	}
	else if ((tel->FC == 0x7C) || (tel->FC == 0x5C)) {
		parcel_t parc;
		parc.len = tel->LE - 7;
		parc.data = pvPortMalloc(parc.len);
		memcpy(parc.data, tel->PDU, parc.len);
		xQueueSend(protocol_queue, &parc, 0);
		hprot.confirm_status = CONF_NEED08;
		SendAckMsg();
	}  else {
		SendAckMsg();
	}


	if (tel->PDU != NULL) {
		vPortFree(tel->PDU);
		tel->PDU = NULL;
	}
	return NO_ERR;
}

static inline error_t FixDataCmdProcessing(telegram_t * tel) {

	return NO_ERR;
}

error_t CommandParser(uint8_t *buf) {
	static telegram_t htel;
	htel.SD = *buf++;
	switch (htel.SD) {
	case 0xDC: /* Token  */
		/* Format: |SD4|DA|SA|*/
		htel.DA = *buf++;
		if (htel.DA != hprot.own_address) {
			return NO_ERR;
		}
		htel.SA = *buf;
		return TokenCmdProcessing(&htel);
		break;
	case 0x10: /* No data */
		/* Format: |SD1|DA|SA|FC|FCS|ED| */
		if (CalculateFCS(buf, 3) != *(buf + 3)) {
			return FCS_ERR;
		}
		htel.DA = *buf++;
		if (htel.DA != hprot.own_address) {
			return NO_ERR;
		}
		htel.SA = *buf++;
		htel.FC = *buf++;
		htel.FCS = *buf++;
		htel.ED = *buf;
		return NoDataCmdProcessing(&htel);
		break;
	case 0x68:
		/* Variable length data */
		/* Format by wiki: |SD2|LE|LEr|SD2|DA|SA|FC|DSAP|SSAP|PDU|FCS|ED| */
		/* Real MPI frame: |SD2|LE|LEr|SD2|DA|SA|FC|DSAP|SSAP|F1/[B0|0x]|RN|PDU|FCS|ED| */
		htel.LE = *buf++;
		htel.LEr = *buf++;
		buf++;
		if (htel.LE != htel.LEr) {
			return FRAME_ERR;
		}
		if (CalculateFCS(buf, htel.LE) != *(buf + htel.LE)) {
			return FCS_ERR;
		}
		htel.DA = *buf++;
		if ((htel.DA & 0x7F) != hprot.own_address) { /* Broadcast message */
			return NO_ERR;
		}
		htel.SA = *buf++;
		htel.FC = *buf++;
		htel.DSAP = *buf++;
		htel.SSAP = *buf++;
		htel.UK1 = *buf++;

		if (htel.UK1 == 0xC0) {
			htel.FCS = *buf++;
			htel.ED = *buf;
		} else if (htel.UK1 == 0xB0) {
			htel.UK2 = *buf++;
			htel.RN = *buf++;
			htel.FCS = *buf++;
			htel.ED = *buf;
		} else if (htel.UK1 == 0xD0) {
			htel.UK2 = *buf++;
			htel.RN = *buf++;
			uint pdu_size = htel.LE - 8;
			htel.PDU = (uint8_t*) pvPortMalloc(pdu_size);
			memcpy(htel.PDU, buf, pdu_size);
			buf += pdu_size;
			htel.FCS = *buf++;
			htel.ED = *buf;
		} else if (htel.UK1 == 0x05) {
			htel.UK2 = *buf++;
			htel.FCS = *buf++;
			htel.ED = *buf;
		} else {
			htel.RN = *buf++;
			uint pdu_size = htel.LE - 7;
			htel.PDU = (uint8_t*) pvPortMalloc(pdu_size);
			memcpy(htel.PDU, buf, pdu_size);
			buf += pdu_size;
			htel.FCS = *buf++;
			htel.ED = *buf;
		}
		return VarDataCmdProcessing(&htel);
		break;
	case 0xA2:
		/* Fixed length data */
		break;
	case 0xE5:
		/* Acknowledgment */
		SendTokenMsg(htel.SA, hprot.own_address);
		hprot.token_possession = 0U;
		return NO_ERR;
		break;
	default:
		return UNKNOWN_SD_ERR;
	}
	return NO_ERR;
}
