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
static inline void AddMaster(profibus_MPI_t *prot, uint8_t new_master);
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
	hp->bus_masters = 0x00U;
	AddMaster(hp, hp->own_address);
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
	hprot.token_possession = 1U;
	if ((hprot.conn_stat == CONN_CLOSE)
			&& (hprot.confirm_status != CONF_NEED08)) {
		SendClosemMsg(hprot.master_address, hprot.own_address);
	} else if (hprot.confirm_status == CONF_NEED07) {
		SendConfirmMsg(hprot.master_address, hprot.own_address, 0x07, 0x5C);
	} else if (hprot.confirm_status == CONF_NEED08) {
		SendConfirmMsg(hprot.master_address, hprot.own_address, 0x08, 0x5C);
	} else if (hprot.confirm_status == CONF_NEED07_AGAIN) {
		SendConfirmMsg(hprot.master_address, hprot.own_address, 0x07, 0x7C);
	} else {
		if (xQueuePeek(tcp_client_queue,&parc,0) != pdPASS) {

			SendTokenMsg(GetNextMaster(&hprot), hprot.own_address);
			hprot.token_possession = 0U;
		} else {
			if (hprot.conn_stat == CONN_OK) {
				parc.data = NULL;
				xQueueReceive(tcp_client_queue, &parc, 0);
				SendRequestMsg(hprot.master_address, hprot.own_address,
						parc.data, parc.len);
				if (parc.data != NULL) {
					vPortFree(parc.data);
				}
			} else if (hprot.conn_stat == CONN_NO) {
				SendConnectMsg(hprot.master_address, hprot.own_address, 0x6D);
			} else if (hprot.conn_stat == CONN_AGAIN) {
				SendConnectMsg(hprot.master_address, hprot.own_address, 0x5D);
			} else {
				SendTokenMsg(GetNextMaster(&hprot), hprot.own_address);
				hprot.token_possession = 0U;
			}
		}
	}
	return NO_ERR;
}

static inline error_t NoDataCmdProcessing(telegram_t * tel) {
	if (tel->FC == 0x49) {
		SendNoDataMsg(tel->SA, tel->DA, 0x20);
#ifdef WATCH_DOG_ON
		if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
			Error_Handler();
		}
#endif /* #ifdef WATCH_DOG_ON */
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
	} else if (tel->UK1 == 0xC0) {
		/* No ACK */
	} else if ((tel->FC == 0x7C) || (tel->FC == 0x5C)) {
		parcel_t parc;
		parc.len = tel->LE - 7;
		parc.data = pvPortMalloc(parc.len);
		memcpy(parc.data, tel->PDU, parc.len);
		xQueueSend(protocol_queue, &parc, 0);
		hprot.confirm_status = CONF_NEED08;
		SendAckMsg();
	} else {
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
	static uint8_t skip_first = 1;
	static telegram_t htel;
	uint8_t size = buf[SIZE_OF_CMD_BUF - 1];
	htel.SD = *buf++;
	switch (htel.SD) {
	case 0xDC: /* Token  */
		/* Format: |SD4|DA|SA|*/
		if (size != 3) {
			return SIZE_ERR;
		}
		htel.DA = *buf++;
		htel.SA = *buf;

		AddMaster(&hprot, htel.DA);
		/*
		 * fixed in v0.6.1
		AddMaster(&hprot, htel.SA);
		*/
		if (htel.DA != hprot.own_address) {
			return NO_ERR;
		}
		return TokenCmdProcessing(&htel);
		break;
	case 0x10: /* No data */
		/* Format: |SD1|DA|SA|FC|FCS|ED| */
		if (CalculateFCS(buf, 3) != *(buf + 3)) {
			return FCS_ERR;
		}
		htel.DA = *buf++;
		htel.SA = *buf++;
		htel.FC = *buf++;
		if (htel.FC == 0x20){
			AddMaster(&hprot, htel.SA);
		}
		if (htel.DA != hprot.own_address) {
			return NO_ERR;
		}
		htel.FCS = *buf++;
		htel.ED = *buf;
		if (skip_first) {
			skip_first = 0;
			return NO_ERR;
		} else {
			return NoDataCmdProcessing(&htel);
		}
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
		if (hprot.token_possession == 1U) {
			SendTokenMsg(GetNextMaster(&hprot), hprot.own_address);
			hprot.token_possession = 0U;
		}
		return NO_ERR;
		break;
	default:
		return UNKNOWN_SD_ERR;
	}
	return NO_ERR;
}

static inline void AddMaster(profibus_MPI_t *prot, uint8_t new_master) {
	prot->bus_masters |= (0x01U << new_master);
}

uint8_t GetNextMaster(profibus_MPI_t *prot) {
	uint8_t pos = prot->own_address;

	for (int i = 0; i < 32; i++) {
		pos++;
		if (pos == 32) {
			pos = 0;
		}
		if (((0x01U << pos) & prot->bus_masters) != 0x00) {
			return pos;
		}
	}
	return prot->own_address;
}

uint8_t CheckMaster(profibus_MPI_t *prot, uint8_t master) {
	if (master == prot->own_address) {
		return 0U;
	}
	if (((0x01U << master) & prot->bus_masters) != 0x00) {
		return 1U;
	} else {
		return 0U;
	}
}
