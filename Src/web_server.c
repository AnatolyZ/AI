#include "web_server.h"

static const unsigned char PAGE_HEADER_200_OK[] = {
		//"HTTP/1.1 200 OK"
		0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x30, 0x20, 0x32, 0x30, 0x30,
		0x20, 0x4f, 0x4b, 0x0d, 0x0a,
		//zero
		0x00 };
static const unsigned char PAGE_HEADER_SERVER[] = {
		//"Server: lwIP/1.3.1 (http://savannah.nongnu.org/projects/lwip)"
		0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x3a, 0x20, 0x6c, 0x77, 0x49, 0x50,
		0x2f, 0x31, 0x2e, 0x33, 0x2e, 0x31, 0x20, 0x28, 0x68, 0x74, 0x74, 0x70,
		0x3a, 0x2f, 0x2f, 0x73, 0x61, 0x76, 0x61, 0x6e, 0x6e, 0x61, 0x68, 0x2e,
		0x6e, 0x6f, 0x6e, 0x67, 0x6e, 0x75, 0x2e, 0x6f, 0x72, 0x67, 0x2f, 0x70,
		0x72, 0x6f, 0x6a, 0x65, 0x63, 0x74, 0x73, 0x2f, 0x6c, 0x77, 0x69, 0x70,
		0x29, 0x0d, 0x0a,
		//zero
		0x00 };
static const unsigned char PAGE_HEADER_CONTENT_TEXT[] = {
		//"Content-type: text/html"
		0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x74, 0x79, 0x70, 0x65,
		0x3a, 0x20, 0x74, 0x65, 0x78, 0x74, 0x2f, 0x68, 0x74, 0x6d, 0x6c, 0x0d,
		0x0a, 0x0d, 0x0a,
		//zero
		0x00 };
//*
static const unsigned char PAGE_HEADER_CONTENT_STREAM[] = {
		//"Content-Type: application/octet-stream"
		0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x54, 0x79, 0x70, 0x65,
		0x3a, 0x20, 0x61, 0x70, 0x70, 0x6c, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f,
		0x6e, 0x2f, 0x6f, 0x63, 0x74, 0x65, 0x74, 0x2d, 0x73, 0x74, 0x72, 0x65,
		0x61, 0x6d, 0x0d, 0x0a,
		//zero
		0x00 };
static const unsigned char PAGE_HEADER_LEN[] = {
		//"Content-Length: "
		0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x4c, 0x65, 0x6e, 0x67,
		0x74, 0x68, 0x3a, 0x20,
		//zero
		0x00 };
static const unsigned char PAGE_HEADER_BYTES[] = {
		//"Accept-Ranges: bytes"
		0x41, 0x63, 0x63, 0x65, 0x70, 0x74, 0x2d, 0x52, 0x61, 0x6e, 0x67, 0x65,
		0x73, 0x3a, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x0d, 0x0a, 0x0d, 0x0a,
		//zero
		0x00 };

static portCHAR PAGE_BODY[768];
extern struct netif gnetif;

static void get_query_parser(char * in_query);

static void get_query_parser(char * in_query) {

}

void web_server_thread(void *arg) {
	err_t err, recv_err;
	struct netbuf *inbuf;
	struct netconn *newconn;
	struct netconn *arg_conn;
	arg_conn = (struct netconn *) arg;
	u16_t buflen;
	char* buf;
	struct fs_file file;

	sprintf(PAGE_BODY,"%s%s%s%s",PAGE_HEADER_200_OK,PAGE_HEADER_SERVER,PAGE_HEADER_CONTENT_TEXT,ip4addr_ntoa(&gnetif.ip_addr));

	printf("Net task created.\n");
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
						printf("Connect\n");
						osDelay(1);
						if ((strncmp((char const *) buf, "GET / ", 6) == 0)
								|| (strncmp((char const *) buf,
										"GET /index.shtml", 16) == 0)) {
							fs_open(&file, "/index.shtml");
							netconn_write(newconn,
									(const unsigned char* )(file.data),
									(size_t )file.len, NETCONN_NOCOPY);
							fs_close(&file);
						} else if (strncmp((char const *) buf,
								"GET /img/logo.png", 17) == 0) {
							fs_open(&file, "/img/logo.png");
							netconn_write(newconn,
									(const unsigned char* )(file.data),
									(size_t )file.len, NETCONN_NOCOPY);
							fs_close(&file);
						} else if (strncmp((char const *) buf, "GET /style.css",
								14) == 0) {
							fs_open(&file, "/style.css");
							netconn_write(newconn,
									(const unsigned char* )(file.data),
									(size_t )file.len, NETCONN_NOCOPY);
							fs_close(&file);
						} else if (strncmp((char const *) buf,
								"GET /AI.shtml?led=1", 19) == 0) {
							HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_9);
						} else if (strncmp((char const *) buf,
								"GET /AI.shtml?IP=", 17) == 0) {
							netconn_write(newconn, PAGE_BODY, strlen((char*)PAGE_BODY), NETCONN_COPY);
						} else {
							fs_open(&file, "/404.html");
							netconn_write(newconn,
									(const unsigned char* )(file.data),
									(size_t )file.len, NETCONN_NOCOPY);
							fs_close(&file);
							printf("File not found\n");
							osDelay(1);
						}
						buf[buflen] = 0;
						printf("-> %s\n", buf);
						osDelay(1);
					}
				}
			}
			netconn_close(newconn);
			netbuf_delete(inbuf);
			netconn_delete(newconn);
		} else {
			printf("Accept failed.\n");
			osDelay(1000);
		}
	}
}
