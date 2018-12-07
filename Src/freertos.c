/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * This notice applies to any and all portions of this file
 * that are not between comment pairs USER CODE BEGIN and
 * USER CODE END. Other portions of this file, whether
 * inserted by the user or by software development tools
 * are owned by their respective copyright owners.
 *
 * Copyright (c) 2018 STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted, provided that the following conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of other
 *    contributors to this software may be used to endorse or promote products
 *    derived from this software without specific written permission.
 * 4. This software, including modifications and/or derivative works of this
 *    software, must execute solely and exclusively on microcontroller or
 *    microprocessor devices manufactured by or for STMicroelectronics.
 * 5. Redistribution and use of this software other than as permitted under
 *    this license is void and will automatically terminate your rights under
 *    this license.
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
 * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
 * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "string.h"
#include "lwip/apps/fs.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define MAIL_SIZE (uint32_t) 5
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern struct netif gnetif;
typedef struct struct_sock_t {
	struct netconn * conn;
} struct_sock;
struct_sock sock01;

/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void tcp_thread(void *arg);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* Create the thread(s) */
	/* definition and creation of defaultTask */
	osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 256);
	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument) {
	/* init code for LWIP */
	MX_LWIP_Init();

	/* USER CODE BEGIN StartDefaultTask */
	printf("lwIP init completed.\n");
	struct netconn *conn;
	err_t err;
	conn = netconn_new(NETCONN_TCP);
	if (conn != NULL) {
		sock01.conn = conn;
		err = netconn_bind(conn, IP_ADDR_ANY, 80);
		if (err == ERR_OK) {
			netconn_listen(conn);
			sys_thread_new("tcp_thread1", tcp_thread, (void*) &sock01,
			DEFAULT_THREAD_STACKSIZE / 4, osPriorityNormal);
			printf("Binding ... OK\n");
			osDelay(1);
		} else {
			netconn_delete(conn);
			printf("Binding ... Err\n");
			osDelay(1);
		}
	}

	/* Infinite loop */
	for (;;) {
/*
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_9);
		osDelay(249);
*/
		osDelay(1);
	}
	/* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
//---------------------------------------------------------------
static void tcp_thread(void *arg) {
	err_t err, recv_err;
	struct netconn *conn;
	struct netbuf *inbuf;
	struct netconn *newconn;
	struct_sock *arg_sock;
	arg_sock = (struct_sock*) arg;
	conn = arg_sock->conn;
	u16_t buflen;
	char* buf;
	struct fs_file file;
	printf("Net task created.\n");
	osDelay(1);
	for (;;) {
		err = netconn_accept(conn, &newconn);
		if (err == ERR_OK) {
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
						} else if (strncmp((char const *) buf,
								"GET /style.css", 14) == 0) {
							fs_open(&file, "/style.css");
							netconn_write(newconn,
									(const unsigned char* )(file.data),
									(size_t )file.len, NETCONN_NOCOPY);
							fs_close(&file);
						} else if (strncmp((char const *) buf,
								"GET /color.html?c=1",19) == 0) {
							HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_9);
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
			osDelay(1);
		}
	}
}
//---------------------------------------------------------------
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
