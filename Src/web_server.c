#include "web_server.h"

volatile int reboot_flag = 0;

static const unsigned char PAGE_HEADER[] = {
		//"HTTP/1.1 200 OK"
		0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x30, 0x20, 0x32, 0x30, 0x30,
		0x20, 0x4f, 0x4b, 0x0d, 0x0a,
		//zero
		//0x00,
		//"Server: lwIP/1.3.1 (http://savannah.nongnu.org/projects/lwip)"
		0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x3a, 0x20, 0x6c, 0x77, 0x49, 0x50,
		0x2f, 0x31, 0x2e, 0x33, 0x2e, 0x31, 0x20, 0x28, 0x68, 0x74, 0x74, 0x70,
		0x3a, 0x2f, 0x2f, 0x73, 0x61, 0x76, 0x61, 0x6e, 0x6e, 0x61, 0x68, 0x2e,
		0x6e, 0x6f, 0x6e, 0x67, 0x6e, 0x75, 0x2e, 0x6f, 0x72, 0x67, 0x2f, 0x70,
		0x72, 0x6f, 0x6a, 0x65, 0x63, 0x74, 0x73, 0x2f, 0x6c, 0x77, 0x69, 0x70,
		0x29, 0x0d, 0x0a,
		//zero
		//0x00,
		0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x74, 0x79, 0x70, 0x65,
		0x3a, 0x20, 0x74, 0x65, 0x78, 0x74, 0x2f, 0x68, 0x74, 0x6d, 0x6c, 0x0d,
		0x0a, 0x0d, 0x0a,
		//zero
		0x00 };
//*

static portCHAR PAGE_BODY[768];
extern struct netif gnetif;
extern UART_HandleTypeDef huart5;

void Web_thread(void *arg) {
	err_t err, recv_err;
	struct netbuf *inbuf;
	struct netconn *newconn;
	struct netconn *arg_conn;
	arg_conn = (struct netconn *) arg;
	u16_t buflen;
	char* buf;
	struct fs_file file;
	FlashToJSON(&hjsondata, &hflash);

	osDelay(1);
	for (;;) {
		err = netconn_accept(arg_conn, &newconn);
		if (err == ERR_OK) {
			printf("Accept.\n");
			recv_err = netconn_recv(newconn, &inbuf);
			if (recv_err == ERR_OK) {
				if (netconn_err(newconn) == ERR_OK) {
					netbuf_data(inbuf, (void**) &buf, &buflen);
					if ((buflen >= 5) && (strncmp(buf, "GET /", 5) == 0)) {
						buf += 5;
						if ((strncmp((char const *) buf, " ", 1) == 0)
								|| (strncmp((char const *) buf, "index.shtml",
										11) == 0)) {
							fs_open(&file, "/index.shtml");
							netconn_write(newconn,
									(const unsigned char* )(file.data),
									(size_t )file.len, NETCONN_NOCOPY);
							fs_close(&file);
						} else if (strncmp((char const *) buf, "img/logo.png",
								12) == 0) {
							fs_open(&file, "/img/logo.png");
							netconn_write(newconn,
									(const unsigned char* )(file.data),
									(size_t )file.len, NETCONN_NOCOPY);
							fs_close(&file);
						} else if (strncmp((char const *) buf, "img/icon.png",
								12) == 0) {
							fs_open(&file, "/img/icon.png");
							netconn_write(newconn,
									(const unsigned char* )(file.data),
									(size_t )file.len, NETCONN_NOCOPY);
							fs_close(&file);
						} else if (strncmp((char const *) buf, "style.css", 9)
								== 0) {
							fs_open(&file, "/style.css");
							netconn_write(newconn,
									(const unsigned char* )(file.data),
									(size_t )file.len, NETCONN_NOCOPY);
							fs_close(&file);
						} else if (strncmp((char const *) buf, "AI.data?=", 9)
								== 0) {
							uint8_t *json;
							json = (uint8_t*) pvPortMalloc(250);
							if (json != NULL) {
								GetJSONData(json);
								sprintf(PAGE_BODY, "%s%s", PAGE_HEADER, json);
								netconn_write(newconn, PAGE_BODY,
										strlen((char* )PAGE_BODY),
										NETCONN_COPY);
								vPortFree(json);
							}
						} else if (strncmp((char const *) buf, "json=", 5)
								== 0) {
							uint8_t *json_str;
							json_str = (uint8_t*) pvPortMalloc(250);
							if (json_str != NULL) {
								DecodeURL((uint8_t*) buf + 5, json_str);
								ParseJSON(&hjsondata, json_str);
								JSONToFlash(&hjsondata, &hflash);
								SaveFash();
								netconn_write(newconn, PAGE_HEADER,
										strlen((char* )PAGE_HEADER),
										NETCONN_COPY);
								vPortFree(json_str);
							}
						} else if (strncmp((char const *) buf, "cmd.reboot=1",
								12) == 0) {
							netconn_write(newconn, PAGE_HEADER,
									strlen((char* )PAGE_HEADER), NETCONN_COPY);
							LogText(SUB_SYS_WEB, LOG_LEV_INFO, "Reboot");
							reboot_flag = 1;
						} else {
							fs_open(&file, "/404.html");
							netconn_write(newconn,
									(const unsigned char* )(file.data),
									(size_t )file.len, NETCONN_NOCOPY);
							fs_close(&file);
							osDelay(1);
						}
					}
				}
			}
			netconn_close(newconn);
			netbuf_delete(inbuf);
			netconn_delete(newconn);
		}
	}
}
