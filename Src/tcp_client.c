/*
 * tcp_server.c
 *
 *  Created on: 11 џэт. 2019 у.
 *      Author: AZharkov
 */

#include "tcp_client.h"

extern osTimerId myTimer01Handle;

void Serv_thread(void *arg) {
	err_t err, recv_err;
	struct netbuf *inbuf;
	struct netconn *newconn;
	struct netconn *arg_conn;
	arg_conn = (struct netconn *) arg;
	u16_t buflen;
	char* buf;
	const uint8_t data_COTP[] = { 0x03, 0x00, 0x00, 0x16, 0x11, 0xD0, 0x00,
			0x01, 0x00, 0x01, 0x00, 0xC1, 0x02, 0x01, 0x00, 0xC2, 0x02, 0x01,
			0x02, 0xC0, 0x01, 0x0A };

	const uint8_t data_S7_setup[] = { 0x03, 0x00, 0x00, 0x1B, 0x02, 0xF0, 0x80,
			0x32, 0x03, 0x00, 0x00, 0x04, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
			0x00, 0xF0, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0xF0 };

	const uint8_t data_S7_var[] = { 0x03, 0x00, 0x00, 0x1A, 0x02, 0xF0, 0x80,
			0x32, 0x03, 0x00, 0x00, 0x05, 0x00, 0x00, 0x02, 0x00, 0x05, 0x00,
			0x00, 0x04, 0x01, 0xFF, 0x04, 0x00, 0x08, 0x08 };

	osDelay(1);
	for (;;) {
		err = netconn_accept(arg_conn, &newconn);
		if (err == ERR_OK) {
			printf("Accept.\n");
			recv_err = netconn_recv(newconn, &inbuf);
			if (recv_err == ERR_OK) {
				if (netconn_err(newconn) == ERR_OK) {
					do {
						netbuf_data(inbuf, (void**) &buf, &buflen);
						if ((buf[0] == 0x03) && (buf[1] == 0x00)
								&& (buf[2] == 0x00) && (buf[3] == 0x16)) {
							//osTimerStart(myTimer01Handle, 5);
							netconn_write(newconn,
									(const unsigned char* )(data_COTP),
									sizeof(data_COTP), NETCONN_NOCOPY);
							while (recv_err == ERR_OK) {
								netconn_recv(newconn, &inbuf);
								netbuf_data(inbuf, (void**) &buf, &buflen);
								if ((buf[0] == 0x03) && (buf[1] == 0x00)
										&& (buf[2] == 0x00)
										&& (buf[3] == 0x19)) {
									netconn_write(newconn,
											(const unsigned char* )(data_S7_setup),
											sizeof(data_S7_setup),
											NETCONN_NOCOPY);
								} else if ((buf[0] == 0x03) && (buf[1] == 0x00)
										&& (buf[2] == 0x00) && (buf[3] == 0x16)) {
									//osTimerStart(myTimer01Handle, 5);
									netconn_write(newconn,
											(const unsigned char* )(data_COTP),
											sizeof(data_COTP), NETCONN_NOCOPY);
								} else {
									netconn_write(newconn,
											(const unsigned char* )(data_S7_var),
											sizeof(data_S7_var),
											NETCONN_NOCOPY);
								}
								netbuf_delete(inbuf);
							}
						}
					} while (netbuf_next(inbuf) >= 0);
					netbuf_delete(inbuf);
				}
			} else {
				LogText(SUB_SYS_TCP, LOG_LEV_ERR, "Receive_error.");
				netbuf_delete(inbuf);
			}
			netconn_close(newconn);
			netconn_delete(newconn);
		}
	}
}



