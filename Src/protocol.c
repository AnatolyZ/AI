/*
 * protocol.c
 *
 *  Created on: 25 дек. 2018 г.
 *      Author: AZharkov
 */

#include "protocol.h"

void ProtocolSettingsInit(profibus_MPI_t* hp) {
	hp->own_address = hflash.own_addr;
	hp->speed = hflash.speed;
	hp->token_possession = 0U;
	hp->have_data_to_send = 0U;
	hp->is_connected = 0U;
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
	if (hprot.have_data_to_send == 0U) {
		SendTokenMsg(tel->SA, hprot.own_address);
		hprot.token_possession = 0U;
	} else {
		hprot.token_possession = 1U;
		SendRequestMsg(tel->SA, hprot.own_address, hprot.data_ptr,
				hprot.data_len);
		hprot.have_data_to_send = 0U;
	}

	return NO_ERR;
}

static inline error_t NoDataCmdProcessing(telegram_t * tel) {
	if (tel->FC == 0x49) {
		SendNoDataMsg(tel->SA, tel->DA, 0x20);
	}
	return NO_ERR;
}

static inline error_t VarDataCmdProcessing(telegram_t * tel) {

	if (tel->UK1 == 0xD0) {
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
		if (htel.UK1 == 0xB0) {
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
		return TokenCmdProcessing(&htel);
		break;
	default:
		return UNKNOWN_SD_ERR;
	}
	return NO_ERR;
}
