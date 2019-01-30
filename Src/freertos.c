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
#include "web_server.h"
#include "eeprom.h"
#include "processUART.h"
#include "log.h"
#include "tcp_client.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
xQueueHandle frames_queue;
xQueueHandle cleaner_queue;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern volatile int reboot_flag;
extern struct netif gnetif;
typedef struct struct_sock_t {
	struct netconn * conn;
} struct_sock;
struct_sock sock01;
osThreadId processTaskHandle;
osThreadId heapCleanerHandle;

volatile unsigned long ulHighFrequencyTimerTicks = 0;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartHeapCleanerTask(void const * argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void) {
	NVIC_SetPriority(TIM7_IRQn, 0);
	NVIC_EnableIRQ(TIM7_IRQn);
	MX_TIM7_Init();
	HAL_TIM_Base_Start_IT(&htim7);
}

__weak unsigned long getRunTimeCounterValue(void) {
	return ulHighFrequencyTimerTicks;
}
/* USER CODE END 1 */

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
	*ppxIdleTaskStackBuffer = &xIdleStack[0];
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	/* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

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
	osThreadDef(processTask, StartProcessTask, osPriorityRealtime, 0, 1024);
	processTaskHandle = osThreadCreate(osThread(processTask), NULL);

	osThreadDef(cleanerTask, StartHeapCleanerTask, osPriorityAboveNormal, 0,
			256);
	heapCleanerHandle = osThreadCreate(osThread(cleanerTask), NULL);
	/* USER CODE END RTOS_TIMERS */

	/* Create the thread(s) */
	/* definition and creation of defaultTask */
	osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 256);
	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

	/* USER CODE BEGIN RTOS_THREADS */

	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_QUEUES */
	frames_queue = xQueueCreate(16, sizeof(uint16_t));
	vQueueAddToRegistry(frames_queue, "frames_queue");
	cleaner_queue = xQueueCreate(16, sizeof(uint8_t*));
	vQueueAddToRegistry(cleaner_queue, "cleaner_queue");
	tcp_client_queue = xQueueCreate(8, sizeof(parcel_t));
	vQueueAddToRegistry(tcp_client_queue, "tcp_client_queue");
	protocol_queue = xQueueCreate(8, sizeof(parcel_t));
	vQueueAddToRegistry(protocol_queue, "protocol_queue");
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

	struct netconn *conn_port80;
	err_t err;
	conn_port80 = netconn_new(NETCONN_TCP);
	if (conn_port80 != NULL) {
		err = netconn_bind(conn_port80, IP_ADDR_ANY, 80);
		if (err == ERR_OK) {
			netconn_listen(conn_port80);
			sys_thread_new("web_thread", Web_thread, (void*) conn_port80,
			DEFAULT_THREAD_STACKSIZE, osPriorityNormal);
		} else {
			netconn_delete(conn_port80);
		}
	}

	struct netconn *conn_port102;
	conn_port102 = netconn_new(NETCONN_TCP);
	if (conn_port102 != NULL) {
		err = netconn_bind(conn_port102, IP_ADDR_ANY, hflash.port);
		if (err == ERR_OK) {
			netconn_listen(conn_port102);
			sys_thread_new("tcp_serv_thread", Client_thread,
					(void*) conn_port102,
					DEFAULT_THREAD_STACKSIZE, osPriorityAboveNormal);
		} else {
			netconn_delete(conn_port102);
		}
	}

	uint8_t data_conn1[] = { 0x80, 0x00, 0x02, 0x00, 0x02, 0x01, 0x00, 0x01,
			0x00 };
	uint8_t * request_data;
	request_data = (uint8_t*) pvPortMalloc(sizeof(data_conn1));
	memcpy(request_data, data_conn1, sizeof(data_conn1));
	hprot.data_len = sizeof(data_conn1);
	hprot.data_ptr = request_data;
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET);
	/* Infinite loop */
	for (;;) {

#ifdef DEBUG_ON
		printf("Free heap: %d\r\n", xPortGetMinimumEverFreeHeapSize());
#endif /* #ifdef DEBUG_ON */
		osDelay(500);
		HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_9);
		if (reboot_flag) {
			osDelay(30);
			SCB->AIRCR = 0x05FA0004; /* Software reset */
		}

	}
	/* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
//---------------------------------------------------------------
void StartHeapCleanerTask(void const * argument) {
	uint8_t * heap_ptr;
	for (;;) {
		xQueueReceive(cleaner_queue, &heap_ptr, portMAX_DELAY);
		vPortFree((uint8_t *) heap_ptr);
		heap_ptr = NULL;
	}
}
//---------------------------------------------------------------
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
