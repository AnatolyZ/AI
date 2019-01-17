/*
 * tcp_server.h
 *
 *  Created on: 11 џэт. 2019 у.
 *      Author: AZharkov
 */

#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "string.h"
#include "lwip/apps/fs.h"
#include "lwip.h"
#include "eeprom.h"
#include "protocol.h"
#include "log.h"
#include "FreeRTOS.h"

typedef struct parcel{
	uint8_t * data;
	uint16_t len;
} parcel_t;

extern xQueueHandle tcp_client_queue;
void Client_thread(void *arg);


#endif /* TCP_CLIENT_H_ */
