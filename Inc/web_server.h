/*
 * web_server.h
 *
 *  Created on: 10 дек. 2018 г.
 *      Author: AZharkov
 */

#ifndef WEB_SERVER_H_
#define WEB_SERVER_H_


#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "string.h"
#include "lwip/apps/fs.h"
#include "lwip.h"


void web_server_thread(void *arg);



#endif /* WEB_SERVER_H_ */
