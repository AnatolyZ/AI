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
extern xQueueHandle cleaner_queue;

profibus_MPI_t hprot;

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

void SendAckMsg(void) {
	uint8_t * temp;
	temp = (uint8_t*) pvPortMalloc(1);
	*temp = 0xE5;
	TRANS_ON();
	HAL_UART_Transmit_DMA(&huart5, temp, 1);
}
void SendTokenMsg(uint8_t to, uint8_t from) {
	uint8_t * temp;
	static int cnt = 100;
	temp = (uint8_t*) pvPortMalloc(3);
	if (temp == NULL) {
		Error_Handler();
	}
	temp[0] = 0xDC;
	temp[1] = to;
	temp[2] = from;
	cnt--;
	if (cnt == 0) {
		HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_10);
		cnt = 10;
	}
	TRANS_ON();
	HAL_UART_Transmit_DMA(&huart5, temp, 3);
}

void SendNoDataMsg(uint8_t to, uint8_t from, uint8_t fc) {
	uint8_t * temp;
	temp = (uint8_t*) pvPortMalloc(6);
	if (temp == NULL) {
		Error_Handler();
	}
	temp[0] = 0x10;
	temp[1] = to;
	temp[2] = from;
	temp[3] = fc;
	temp[4] = to + from + fc;
	temp[5] = 0x16;
	TRANS_ON();
	HAL_UART_Transmit_DMA(&huart5, temp, 6);
}

void SendRequestMsg(uint8_t to, uint8_t from, uint8_t* data, uint8_t data_len) {
	static uint8_t req_num = 0;
	uint8_t *msg_ptr;
	uint8_t *tmp_ptr;
	uint8_t msg_len = data_len + 14;
	msg_ptr = (uint8_t*) pvPortMalloc(msg_len);
	if (msg_ptr == NULL) {
		LogText(SUB_SYS_MEMORY, LOG_LEV_ERR,
				"Request buffer allocation error.");
		Error_Handler();
	}
	tmp_ptr = msg_ptr;
	*tmp_ptr++ = 0x68;
	*tmp_ptr++ = data_len + 8;
	*tmp_ptr++ = data_len + 8;
	*tmp_ptr++ = 0x68;
	*tmp_ptr++ = to | 0x80;
	*tmp_ptr++ = from | 0x80;
	*tmp_ptr++ = 0x6D;
	*tmp_ptr++ = 0x00;
	*tmp_ptr++ = 0x1F;
	*tmp_ptr++ = 0xE0;
	*tmp_ptr++ = 0x04;
	*tmp_ptr++ = req_num++;
	memcpy(tmp_ptr, data, data_len);
	tmp_ptr += data_len;
	*tmp_ptr++ = CalculateFCS(msg_ptr + 4, data_len + 8);
	*tmp_ptr = 0x16;
	TRANS_ON();
	HAL_UART_Transmit_DMA(&huart5, msg_ptr, msg_len);
	/*
	 osDelay(4);
	 uint8_t* DC;
	 DC = (uint8_t*) pvPortMalloc(3);
	 DC[0] = 0xDC;
	 DC[1] = 0x02;
	 DC[2] = 0x01;
	 TRANS_ON();
	 HAL_UART_Transmit_DMA(&huart5, DC, 3);
	 */
	hprot.is_connected = 1U;
	hprot.have_data_to_send = 0U;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &huart5) {
		CB_Write(&inbuf_UART, received_byte);
		HAL_UART_Receive_IT(&huart5, &received_byte, 1);
		__HAL_TIM_SET_COUNTER(&htim8, 0x00U);
		HAL_TIM_Base_Start_IT(&htim8);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	static portBASE_TYPE xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	if (huart == &huart5) {
		TRANS_OFF();
		xQueueSendFromISR(cleaner_queue, &(huart5.pTxBuffPtr),
				&xHigherPriorityTaskWoken);
		if (xHigherPriorityTaskWoken == pdTRUE) {
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	} else if (huart == hlog.interface) {
		xQueueSendFromISR(cleaner_queue, &(hlog.interface->pTxBuffPtr),
				&xHigherPriorityTaskWoken);
		if (xHigherPriorityTaskWoken == pdTRUE) {
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
}

void StartProcessTask(void const * argument) {
	uint8_t len;
	static uint8_t cmd_data_buf[SIZE_OF_CMD_BUF];

	error_t err = NO_ERR;
	err = CB_Init(&inbuf_UART, UART_BUFF_SIZE);
	if (err != NO_ERR) {
		LogText(SUB_SYS_MEMORY, LOG_LEV_ERR,
				"Circular buffer allocation error.\r\n");
	}
	HAL_UART_Receive_IT(&huart5, &received_byte, 1);
	for (;;) {
		xQueueReceive(frames_queue, &len, portMAX_DELAY);
		cmd_data_buf[SIZE_OF_CMD_BUF - 1] = len;
		uint pos = 0;
		while (len) {
			uint8_t ch;
			CB_Read(&inbuf_UART, &ch);
			--len;
			cmd_data_buf[pos++] = ch;
		}
		CommandParser(cmd_data_buf);
	}
}

