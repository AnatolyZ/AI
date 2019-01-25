/*
 * json.c
 *
 *  Created on: 22 џэт. 2019 у.
 *      Author: AZharkov
 */

/* Includes */
#include "json.h"
/* -------- */

/* Global variables */
json_data_t hjsondata;
/* ---------------- */

/* Function prototypes */
/* ------------------- */

/* ---------- FUNCTIONS ------------ */
/* -------||--||--||--||--||-------- */
/* -------\/--\/--\/--\/--\/-------- */
uint GetJSONData(uint8_t *pstr) {
	uint size = 0;
	memcpy(pstr, "{\"addr_IP\":\"", 12);
	size += 12;
	uint8_t tmp = strlen((const char *) hjsondata.ip_addr);
	memcpy(pstr + size, hjsondata.ip_addr, tmp);
	size += tmp;
	memcpy(pstr + size, "\",\"mask\":\"", 10);
	size += 10;
	tmp = strlen((const char *) hjsondata.mask);
	memcpy(pstr + size, hjsondata.mask, tmp);
	size += tmp;
	memcpy(pstr + size, "\",\"gate\":\"", 10);
	size += 10;
	tmp = strlen((const char *) hjsondata.gate);
	memcpy(pstr + size, hjsondata.gate, tmp);
	size += tmp;
	memcpy(pstr + size, "\",\"port\":", 9);
	size += 9;
	itoa(hjsondata.port, (char *) pstr + size, 10);
	size += strlen((const char *) pstr + size);
	memcpy(pstr + size, ",\"speed\":", 9);
	size += 9;
	itoa(hjsondata.speed, ((char *) pstr + size), 10);
	size += strlen((const char *) pstr + size);
	memcpy(pstr + size, ",\"addr_own\":", 12);
	size += 12;
	itoa(hjsondata.own_addr, (char *) pstr + size, 10);
	size += strlen((const char *) pstr + size);
	memcpy(pstr + size, ",\"ver\":\"", 8);
	size += 8;
	tmp = strlen((const char *) hjsondata.ver);
	memcpy(pstr + size, (const char *) hjsondata.ver, tmp);
	size += tmp;
	memcpy(pstr + size, "\",\"sn\":", 7);
	size += 7;
	itoa(hjsondata.serial_num, (char *) (pstr + size), 10);
	size += strlen((const char *) pstr + size);
	memcpy(pstr + size, ",\"addr_MAC\":\"", 13);
	size += 13;
	tmp = strlen((const char *) hjsondata.mac_addr);
	memcpy(pstr + size, hjsondata.mac_addr, tmp);
	size += tmp;
	memcpy(pstr + size, "\"}\0", 3);
	size += 3;
	return size;
}

void FlashToJSON(json_data_t *js, flash_data_t *fs) {

	js->speed = fs->speed;
	js->port = fs->port;
	js->own_addr = fs->own_addr;
	js->serial_num = fs->serial_num;
	uint len = 0;

	for (int i = 0; i < 4; i++) {
		itoa(fs->IP_addr[i], (char*) &js->ip_addr[len], 10);
		len += strlen((char*) &js->ip_addr[len]);
		js->ip_addr[len] = '.';
		len++;
	}
	js->ip_addr[--len] = '\0';

	len = 0;
	for (int i = 0; i < 4; i++) {
		itoa(fs->gate[i], (char*) &js->gate[len], 10);
		len += strlen((char*) &js->gate[len]);
		js->gate[len] = '.';
		len++;
	}
	js->gate[--len] = '\0';

	len = 0;
	for (int i = 0; i < 4; i++) {
		itoa(fs->mask[i], (char*) &js->mask[len], 10);
		len += strlen((char*) &js->mask[len]);
		js->mask[len] = '.';
		len++;
	}
	js->mask[--len] = '\0';

	len = 0;
	for (int i = 0; i < 6; i++) {
		itoa(fs->mac_addr[i], (char*) &js->mac_addr[len], 16);
		len += strlen((char*) &js->mac_addr[len]);
		js->mac_addr[len] = '-';
		len++;
	}
	js->mac_addr[--len] = '\0';

	len = 0;
	for (int i = 0; i < 3; i++) {
		itoa(fs->ver[i], (char*) &js->ver[len], 10);
		len += strlen((char*) &js->ver[len]);
		js->ver[len] = '.';
		len++;
	}
	js->ver[--len] = '\0';
}

void DecodeURL(uint8_t *buf, uint8_t *result) {
	while (*buf != '\r') {
		if (*buf == '%') {
			if ((*(buf + 1) == '7') && (*(buf + 2) == 'B')) {
				*result = '{';
				buf += 2;
			}
			if ((*(buf + 1) == '2') && (*(buf + 2) == '2')) {
				*result = '\"';
				buf += 2;
			}
			if ((*(buf + 1) == '7') && (*(buf + 2) == 'D')) {
				*result = '}';
				*(result + 1) = '\0';
				break;
			}
		} else {
			*result = *buf;
		}
		buf++;
		result++;
	}
}

void ParseJSON(json_data_t *js, uint8_t *json_str) {
	int resultCode;
	jsmn_parser p;
	jsmntok_t tokens[MAXNUMBER_OF_TOKENS];
	jsmn_init(&p);
	resultCode = jsmn_parse(&p, (char*) json_str, strlen((char*) json_str),
			tokens, sizeof(tokens) / sizeof(tokens[0]));

	if (resultCode > 0) {
		uint8_t keyString[MAX_TOKEN_LENGTH];
		uint8_t Prev_keyString[MAX_TOKEN_LENGTH];

		for (int i = 1; i <= resultCode - 1; i++) // resultCode == 0 => whole json string
				{
			jsmntok_t key = tokens[i];
			uint16_t length = key.end - key.start;

			if (length < MAX_TOKEN_LENGTH) {
				memcpy(keyString, &json_str[key.start], length);
				keyString[length] = '\0';
				/*
				 {\"addr_IP\":\"192.168.4.255\",\"mask\":\"255.255.255.000\",\"gate\":\"192.168.4.1\",\"port\":102,\"speed\":187500,
				 \"addr_own\":1,\"ver\":\"0.0.0\",\"sn\":0,\"addr_MAC\":\"AA-BB-CC-DD-EE-FF\"}
				 */
				if (strcmp((char*) Prev_keyString, "addr_IP") == 0) {
					strcpy((char*) js->ip_addr, (char*) keyString);
				} else if (strcmp((char*) Prev_keyString, "mask") == 0) {
					strcpy((char*) js->mask, (char*) keyString);
				} else if (strcmp((char*) Prev_keyString, "gate") == 0) {
					strcpy((char*) js->gate, (char*) keyString);
				} else if (strcmp((char*) Prev_keyString, "port") == 0) {
					js->port = (uint16_t) atoi((char*) keyString);
				} else if (strcmp((char*) Prev_keyString, "speed") == 0) {
					js->speed = (uint32_t) atoi((char*) keyString);
				} else if (strcmp((char*) Prev_keyString, "addr_own") == 0) {
					js->own_addr = (uint8_t) atoi((char*) keyString);
				} else if (strcmp((char*) Prev_keyString, "sn") == 0) {
					if (js->serial_num
							== ((DEFAULT_SN_MS << 16) + DEFAULT_SN_LS)) {
						js->serial_num = (uint8_t) atoi((char*) keyString);
					}
				} else if (strcmp((char*) Prev_keyString, "addr_MAC") == 0) {
					strcpy((char*) js->mac_addr, (char*) keyString);
				}
				strcpy((char*) Prev_keyString, (char*) keyString);
			}
		}
	}

}

void JSONToFlash(json_data_t *js, flash_data_t *fs) {
	ip4_addr_t new_ip;
	union {
		uint32_t ui32;
		uint8_t ui8[4];
	} tmp_u;

	ipaddr_aton((char*)js->ip_addr, &new_ip);
	tmp_u.ui32 = new_ip.addr;
	fs->IP_addr[0] = tmp_u.ui8[0];
	fs->IP_addr[1] = tmp_u.ui8[1];
	fs->IP_addr[2] = tmp_u.ui8[2];
	fs->IP_addr[3] = tmp_u.ui8[3];

	ipaddr_aton((char*)js->gate, &new_ip);
	tmp_u.ui32 = new_ip.addr;
	fs->gate[0] = tmp_u.ui8[0];
	fs->gate[1] = tmp_u.ui8[1];
	fs->gate[2] = tmp_u.ui8[2];
	fs->gate[3] = tmp_u.ui8[3];

	ipaddr_aton((char*)js->mask, &new_ip);
	tmp_u.ui32 = new_ip.addr;
	fs->mask[0] = tmp_u.ui8[0];
	fs->mask[1] = tmp_u.ui8[1];
	fs->mask[2] = tmp_u.ui8[2];
	fs->mask[3] = tmp_u.ui8[3];
	uint8_t *ptr = js->mac_addr;
	for (int i = 0; i < 6; i++) {
		uint8_t tok[3];
		int j = 0;
		while (j < 2 && *ptr != '-') {
			tok[j++] = *ptr++;
		}
		ptr++;
		tok[j] = '\0';
		fs->mac_addr[i] = strtol((char*)tok, NULL, 16);
	}
	fs->own_addr = js->own_addr;
	fs->port = js->port;
	fs->serial_num = js->serial_num;
	fs->speed = js->speed;
}
