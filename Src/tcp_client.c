/*
 * tcp_server.c
 *
 *  Created on: 11 џэт. 2019 у.
 *      Author: AZharkov
 */

#include "tcp_client.h"

xQueueHandle tcp_client_queue;

void Client_thread(void *arg) {
	err_t accept_err, err;
	struct netbuf *inbuf;
	struct netconn *newconn;
	struct netconn *arg_conn;
	arg_conn = (struct netconn *) arg;
	u16_t buflen;
	char *buf;

	uint8_t data_COTP[] = { 0x03, 0x00, 0x00, 0x16, 0x11, 0xD0, 0x00, 0x01,
			0x00, 0x01, 0x00, 0xC1, 0x02, 0x01, 0x00, 0xC2, 0x02, 0x01, 0x02,
			0xC0, 0x01, 0x0A };

	osDelay(1);
	for (;;) {
		accept_err = netconn_accept(arg_conn, &newconn);
		if (accept_err == ERR_OK) {
			err = netconn_recv(newconn, &inbuf);
			while (err == ERR_OK) {
				do {
					netbuf_data(inbuf, (void**) &buf, &buflen);
					if ((buf[0] == 0x03) && (buf[1] == 0x00) && (buf[2] == 0x00)
							&& (buf[3] == 0x16) && (buf[21] == hprot.master_address)) {
						netconn_write(newconn,
								(const unsigned char* )(data_COTP),
								sizeof(data_COTP), NETCONN_COPY);
						if (hprot.conn_stat == CONN_CLOSED) {
							hprot.conn_stat = CONN_AGAIN;
						}
						hprot.req_num = 0;
					} else if (buf[7] == 0x32) {
						parcel_t parc;
						parc.len = buflen - 7;
						parc.data = pvPortMalloc(parc.len);
						memcpy(parc.data, buf + 7, parc.len);
						xQueueSend(tcp_client_queue, &parc, 0);
						xQueueReceive(protocol_queue, &parc, portMAX_DELAY);
						uint8_t * answer;
						answer = (uint8_t*) pvPortMalloc(parc.len + 7);
						answer[0] = 0x03;
						answer[1] = 0x00;
						answer[2] = 0x00;
						answer[3] = parc.len + 7;
						answer[4] = 0x02;
						answer[5] = 0xF0;
						answer[6] = 0x80;
						memcpy(answer + 7, parc.data, parc.len);
						vPortFree(parc.data);
						netconn_write(newconn, (const unsigned char* )(answer),
								parc.len + 7, NETCONN_COPY);
						vPortFree(answer);
					}
				} while (netbuf_next(inbuf) >= 0);
				netbuf_delete(inbuf);
				err = netconn_recv(newconn, &inbuf);
			} /* while (err == ERR_OK) */
			if (err == ERR_CLSD) {
				if (hprot.conn_stat == CONN_OK) {
					hprot.conn_stat = CONN_CLOSE;
				}
			}
		}
		netconn_close(newconn);
		netconn_delete(newconn);
	}
}

