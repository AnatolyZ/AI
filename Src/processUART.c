/*
 * processUART.c
 *
 *  Created on: 18 дек. 2018 г.
 *      Author: AZharkov
 */

#include "processUART.h"

uint8_t received_byte;

extern xQueueHandle RecQHandle;

inline void CommandProcess(){
	HAL_TIM_Base_Stop_IT(&htim8);
	HAL_GPIO_TogglePin(GPIOE,GPIO_PIN_9);
	//HAL_TIM_Base_Start_IT(&htim8);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	BaseType_t xHigherPriorityTaskWokenByPost = pdFALSE;
	xQueueSendFromISR(RecQHandle, (void *)&received_byte, &xHigherPriorityTaskWokenByPost);
	HAL_UART_Receive_IT(&huart5, &received_byte,1);
	__HAL_TIM_SET_COUNTER(&htim8,0x00U);
	HAL_TIM_Base_Start_IT(&htim8);
}

void StartProcessTask(void const * argument){
	uint8_t byte_from_queue;

	HAL_UART_Receive_IT(&huart5, &received_byte,1);
	for(;;){
		xQueueReceive( RecQHandle, &byte_from_queue, portMAX_DELAY);
		printf("%X ",byte_from_queue);

	}
}
