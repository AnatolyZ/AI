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
/*
 {\"addr_IP\":\"192.168.4.255\",\"mask\":\"255.255.255.000\",\"gate\":\"192.168.4.1\",\"port\":102,\"speed\":187500,
 \"addr_own\":1,\"ver\":\"0.0.0\",\"sn\":0,\"addr_MAC\":\"AA-BB-CC-DD-EE-FF\"}
 */
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
	/*

	 uint8_t ver[12];
	 } json_data_t;

	 uint16_t ver[3];

	 } flash_data_t;
	 */
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
