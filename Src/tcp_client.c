/*
 * tcp_server.c
 *
 *  Created on: 11 џэт. 2019 у.
 *      Author: AZharkov
 */

#include "tcp_client.h"

xQueueHandle tcp_client_queue;
extern volatile int reboot_flag;

void Client_thread(void *arg) {
	err_t err;
	uint8_t conn_flag = 0;
	static uint8_t task_num = 0;
	uint8_t inst_num;
	struct netbuf *inbuf;
	struct netconn *newconn;
	newconn = (struct netconn *) arg;
	u16_t buflen;
	char *buf;
	struct ip4_addr ClientAddr;
	uint8_t data_COTP[] = { 0x03, 0x00, 0x00, 0x16, 0x11, 0xD0, 0x00, 0x01,
			0x00, 0x01, 0x00, 0xC1, 0x02, 0x01, 0x00, 0xC2, 0x02, 0x01, 0x02,
			0xC0, 0x01, 0x0A };
	uint8_t data_Connect[] = { 0x32, 0x03, 0x00, 0x00, 0x04, 0x00, 0x00, 0x08,
			0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
			0xF0 };

	ClientAddr.addr = 0;
	uint16_t ClientPort = 0;
	netconn_getaddr(newconn, &ClientAddr, &ClientPort, 0);
	task_num++;
	inst_num = task_num;

	LogText(INFO_HIDE, SUB_SYS_TCP, LOG_LEV_INFO, "\r\n");
	LogText(INFO_SHOW, SUB_SYS_TCP, LOG_LEV_INFO, "New TCP-server created on port ");
	LogNum(INFO_HIDE, SUB_SYS_TCP, LOG_LEV_INFO, ClientPort, 10);
	LogText(INFO_HIDE, SUB_SYS_TCP, LOG_LEV_INFO, ".\r\n");
	LogText(INFO_SHOW, SUB_SYS_TCP, LOG_LEV_INFO, "Number of task: ");
	LogNum(INFO_HIDE, SUB_SYS_TCP, LOG_LEV_INFO, task_num, 10);
	LogText(INFO_HIDE, SUB_SYS_TCP, LOG_LEV_INFO, ".\r\n");

	osDelay(1);

	for (;;) {
		err = netconn_recv(newconn, &inbuf);
		while (err == ERR_OK) {
			do {
				netbuf_data(inbuf, (void**) &buf, &buflen);
				if (inst_num != 1 && buf[17] == 0x04) reboot_flag = 1;
				if (buf[7] == 0x32 && inst_num == 1) {
					parcel_t parc;
					parc.len = buflen - 7;
					parc.data = pvPortMalloc(parc.len);
					memcpy(parc.data, buf + 7, parc.len);
					xQueueSend(tcp_client_queue, &parc, 0);
					xQueueReceive(protocol_queue, &parc, portMAX_DELAY);
					conn_flag = 1;
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
				} else if (buf[7] == 0x32 && buf[17] == 0xF0 && inst_num != 1 ) {
					uint8_t * answer;
					size_t data_size = sizeof(data_Connect);
					answer = (uint8_t*) pvPortMalloc(data_size + 7);
					answer[0] = 0x03;
					answer[1] = 0x00;
					answer[2] = 0x00;
					answer[3] = data_size + 7;
					answer[4] = 0x02;
					answer[5] = 0xF0;
					answer[6] = 0x80;
					memcpy(answer + 7, data_Connect, data_size);
					netconn_write(newconn, (const unsigned char* )(answer),
							data_size + 7, NETCONN_COPY);
					vPortFree(answer);
				} else if ((buf[0] == 0x03) && (buf[1] == 0x00)
						&& (buf[2] == 0x00) && (buf[3] == 0x16)
						&& (CheckMaster(&hprot, buf[21]))) {
					data_COTP[18] = buf[21];
					hprot.master_address = buf[21];
					netconn_write(newconn, (const unsigned char* )(data_COTP),
							sizeof(data_COTP), NETCONN_COPY);
					if (hprot.conn_stat == CONN_CLOSED) {
						hprot.conn_stat = CONN_AGAIN;
					}
					if (inst_num == 1) hprot.req_num = 0;
				}
			} while (netbuf_next(inbuf) >= 0);
			netbuf_delete(inbuf);
			err = netconn_recv(newconn, &inbuf);
		} /* while (err == ERR_OK) */
		if (err == ERR_CLSD) {
			if ((hprot.conn_stat == CONN_OK) && (conn_flag == 1)) {
				hprot.conn_stat = CONN_CLOSE;
				conn_flag = 0;
			}
			LogText(INFO_SHOW, SUB_SYS_TCP, LOG_LEV_INFO,
					"Connection closed by client.\r\n");
		}
		task_num--;
		netconn_close(newconn);
		LogText(INFO_SHOW, SUB_SYS_TCP, LOG_LEV_INFO,
				"Connection deleted on port: ");
		LogNum(INFO_HIDE, SUB_SYS_TCP, LOG_LEV_INFO, ClientPort, 10);
		LogText(INFO_HIDE, SUB_SYS_TCP, LOG_LEV_INFO, ".\r\n");
		netconn_delete(newconn);
		LogText(INFO_SHOW, SUB_SYS_TCP, LOG_LEV_INFO,
				"TCP-server task deleted. Task number: ");
		LogNum(INFO_HIDE, SUB_SYS_TCP, LOG_LEV_INFO, inst_num, 10);
		LogText(INFO_HIDE, SUB_SYS_TCP, LOG_LEV_INFO, ".\r\n");
		LogText(INFO_SHOW, SUB_SYS_MEMORY, LOG_LEV_INFO, "Free heap size: ");
		LogNum(INFO_HIDE, SUB_SYS_MEMORY, LOG_LEV_INFO, xPortGetFreeHeapSize(),
				10);
		LogText(INFO_HIDE, SUB_SYS_MEMORY, LOG_LEV_INFO, " bytes.\r\n");
		vTaskDelete(NULL);
	}
}

