#include "web_server.h"

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

static void form_data_parser(char * in_buf);
static uint read_param(char * out_buf, const char * const in_buf, const uint max_len);

static uint read_param(char * out_buf, const char * const  in_buf, const uint max_len){
	const char* tmp_p =  in_buf;
	uint len = 0;
	while (*tmp_p != ' ' && *tmp_p != '&' && len <= max_len) {
		*out_buf++ = *tmp_p;
		len++;
		tmp_p++;
	}
	return len;
}



static void form_data_parser(char * in_buf) {
	char par_str[16];
	uint par_len;
	uint baudrate = 0;
	while (*in_buf != ' ') {
		if (*in_buf == '&') {
			in_buf++;
		}
		int param_num = atoi(in_buf);
		if (param_num < 9){
			in_buf += 2;
		} else {
			in_buf += 3;
		}
		switch(param_num){
		case 1:                                        //IP-address
			par_len = read_param(par_str,in_buf,15);
			par_str[par_len] = '\0';
			ip4_addr_t new_ip;
			ipaddr_aton(par_str,&new_ip);
			EE_WriteVariable(IP_02_01_ADDR,(uint16_t)(new_ip.addr & 0x0000FFFF));
			EE_WriteVariable(IP_04_03_ADDR,(uint16_t)((new_ip.addr >> 16) & 0x0000FFFF));
			netif_set_ipaddr(&gnetif,&new_ip);
			in_buf += par_len;
			break;
		case 2:                                       //Baudrate
			par_len = read_param(par_str,in_buf,15);
			par_str[par_len] = '\0';
			baudrate = atoi(par_str);
			HAL_UART_DeInit(&huart5);
			huart5.Init.BaudRate = baudrate;
			EE_WriteVariable(BR_LS_ADDR,(uint16_t)(baudrate & 0x0000FFFF));
			EE_WriteVariable(BR_MS_ADDR,(uint16_t)((baudrate >> 16) & 0x0000FFFF));
			if (HAL_UART_Init(&huart5) != HAL_OK)
			{
			   Error_Handler();
			}
			in_buf += par_len;
			break;
		}
	}
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
						} else if (strncmp((char const *) buf, "AI.shtml?led=1",
								14) == 0) {
							HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_9);
						} else if (strncmp((char const *) buf, "AI.shtml?IP=",
								12) == 0) {
							sprintf(PAGE_BODY, "%s%s", PAGE_HEADER,
									ip4addr_ntoa(&gnetif.ip_addr));
							netconn_write(newconn, PAGE_BODY,
									strlen((char* )PAGE_BODY), NETCONN_COPY);
						} else if (strncmp((char const *) buf, "AI.shtml?BR=",
								12) == 0) {

							sprintf(PAGE_BODY, "%s%u", PAGE_HEADER,(uint)huart5.Init.BaudRate);
							netconn_write(newconn, PAGE_BODY,
									strlen((char* )PAGE_BODY), NETCONN_COPY);
						} else if (*buf == '?') {
							form_data_parser(++buf);
							fs_open(&file, "/index.shtml");
							netconn_write(newconn,
									(const unsigned char* )(file.data),
									(size_t )file.len, NETCONN_NOCOPY);
							fs_close(&file);
						} else {
							fs_open(&file, "/404.html");
							netconn_write(newconn,
									(const unsigned char* )(file.data),
									(size_t )file.len, NETCONN_NOCOPY);
							fs_close(&file);
							printf("File not found\n");
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
