/*
 * processUART.c
 *
 *  Created on: 18 дек. 2018 г.
 *      Author: AZharkov
 */

#include "processUART.h"
#include "log.h"

uint8_t received_byte;
circbuff inbuf_UART;

extern xQueueHandle frames_queue;

inline void CommandProcess() {
	static portBASE_TYPE xHigherPriorityTaskWoken;
	uint32_t len;
	xHigherPriorityTaskWoken = pdFALSE;
	HAL_TIM_Base_Stop_IT(&htim8);
	len = CB_Data_Length(&inbuf_UART);
	xQueueSendFromISR(frames_queue, &len, &xHigherPriorityTaskWoken);
	if (xHigherPriorityTaskWoken == pdTRUE) {
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &huart5)
	{
		CB_Write(&inbuf_UART, received_byte);
		HAL_UART_Receive_IT(&huart5, &received_byte, 1);
		__HAL_TIM_SET_COUNTER(&htim8, 0x00U);
		HAL_TIM_Base_Start_IT(&htim8);
	}

}

void StartProcessTask(void const * argument) {
	uint32_t len;
	cb_err err = NO_ERR;
	err = CB_Init(&inbuf_UART, UART_BUFF_SIZE);
	if (err != NO_ERR) {
		printf("Buffer allocation error");
	}
	HAL_UART_Receive_IT(&huart5, &received_byte, 1);
	for (;;) {
		xQueueReceive(frames_queue, &len, portMAX_DELAY);

		while (len) {
			HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_9);
			uint8_t ch;
			CB_Read(&inbuf_UART, &ch);
			--len;
		}

	}
}
