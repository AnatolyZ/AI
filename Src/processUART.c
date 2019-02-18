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

void SendClosemMsg(uint8_t to, uint8_t from) {
	uint8_t *msg_ptr;
	uint8_t *tmp_ptr;
	uint8_t msg_len = 12;
	msg_ptr = (uint8_t*) pvPortMalloc(msg_len);
	if (msg_ptr == NULL) {
		LogText(INFO_SHOW, SUB_SYS_MEMORY, LOG_LEV_ERR,
				"Request buffer allocation error.\r\n");
		Error_Handler();
	}
	tmp_ptr = msg_ptr;
	*tmp_ptr++ = 0x68;
	*tmp_ptr++ = 0x06;
	*tmp_ptr++ = 0x06;
	*tmp_ptr++ = 0x68;
	*tmp_ptr++ = to | 0x80;
	*tmp_ptr++ = from | 0x80;
	*tmp_ptr++ = 0x5C;
	*tmp_ptr++ = hprot.master_SAP;
	*tmp_ptr++ = 0x1F;
	*tmp_ptr++ = 0x80;
	*tmp_ptr++ = CalculateFCS(msg_ptr + 4, 6);
	*tmp_ptr = 0x16;
	TRANS_ON();
	HAL_UART_Transmit_DMA(&huart5, msg_ptr, msg_len);
	hprot.conn_stat = CONN_CLOSED;
}

void SendConfirmMsg(uint8_t to, uint8_t from, uint8_t size, uint8_t func) {
	uint8_t *msg_ptr;
	uint8_t *tmp_ptr;
	uint8_t msg_len = size + 6;
	msg_ptr = (uint8_t*) pvPortMalloc(msg_len);
	if (msg_ptr == NULL) {
		LogText(INFO_SHOW, SUB_SYS_MEMORY, LOG_LEV_ERR,
				"Request buffer allocation error.\r\n");
		Error_Handler();
	}
	tmp_ptr = msg_ptr;
	*tmp_ptr++ = 0x68;
	*tmp_ptr++ = size;
	*tmp_ptr++ = size;
	*tmp_ptr++ = 0x68;
	*tmp_ptr++ = to | 0x80;
	*tmp_ptr++ = from | 0x80;
	*tmp_ptr++ = func;
	*tmp_ptr++ = hprot.master_SAP;
	*tmp_ptr++ = 0x1F;
	if (size == 0x07) {
		*tmp_ptr++ = 0x05;
		*tmp_ptr++ = 0x07;
	} else {
		*tmp_ptr++ = 0xB0;
		*tmp_ptr++ = 0x07;
		*tmp_ptr++ = hprot.req_num++;
		if (hprot.req_num == 0) {
			hprot.req_num++;
		}
	}
	*tmp_ptr++ = CalculateFCS(msg_ptr + 4, size);
	*tmp_ptr = 0x16;
	hprot.confirm_status = CONF_SENT;
	TRANS_ON();
	HAL_UART_Transmit_DMA(&huart5, msg_ptr, msg_len);
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
		cnt = 100;
	}
	TRANS_ON();
	if (HAL_UART_Transmit_DMA(&huart5, temp, 3) == HAL_BUSY) {
		TRANS_OFF();
		vPortFree(temp);
	}
	HAL_IWDG_Refresh(&hiwdg);
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
	uint8_t *msg_ptr;
	uint8_t *tmp_ptr;
	uint8_t msg_len = data_len + 13;
	msg_ptr = (uint8_t*) pvPortMalloc(msg_len);
	if (msg_ptr == NULL) {
		LogText(INFO_SHOW, SUB_SYS_MEMORY, LOG_LEV_ERR,
				"Request buffer allocation error.\r\n");
		Error_Handler();
	}
	tmp_ptr = msg_ptr;
	*tmp_ptr++ = 0x68;
	*tmp_ptr++ = data_len + 7;
	*tmp_ptr++ = data_len + 7;
	*tmp_ptr++ = 0x68;
	*tmp_ptr++ = to | 0x80;
	*tmp_ptr++ = from | 0x80;
	*tmp_ptr++ = 0x7C;
	*tmp_ptr++ = hprot.master_SAP;
	*tmp_ptr++ = 0x1F;
	*tmp_ptr++ = 0xF1;
	*tmp_ptr++ = hprot.req_num;
	memcpy(tmp_ptr, data, data_len);
	tmp_ptr += data_len;
	*tmp_ptr++ = CalculateFCS(msg_ptr + 4, data_len + 7);
	*tmp_ptr = 0x16;
	TRANS_ON();
	HAL_UART_Transmit_DMA(&huart5, msg_ptr, msg_len);
}

void SendConnectMsg(uint8_t to, uint8_t from, uint8_t fc) {
	uint8_t data[] = { 0x80, 0x00, 0x02, 0x00, 0x02, 0x01, 0x00, 0x01, 0x00 };
	uint8_t data_len = sizeof(data);
	uint8_t *msg_ptr;
	uint8_t *tmp_ptr;
	uint8_t msg_len = data_len + 14;
	msg_ptr = (uint8_t*) pvPortMalloc(msg_len);
	if (msg_ptr == NULL) {
		LogText(INFO_SHOW, SUB_SYS_MEMORY, LOG_LEV_ERR,
				"Request buffer allocation error.\r\n");
		Error_Handler();
	}
	tmp_ptr = msg_ptr;
	*tmp_ptr++ = 0x68;
	*tmp_ptr++ = data_len + 8;
	*tmp_ptr++ = data_len + 8;
	*tmp_ptr++ = 0x68;
	*tmp_ptr++ = to | 0x80;
	*tmp_ptr++ = from | 0x80;
	*tmp_ptr++ = fc;
	*tmp_ptr++ = 0x00;
	*tmp_ptr++ = 0x1F;
	*tmp_ptr++ = 0xE0;
	*tmp_ptr++ = 0x04;
	*tmp_ptr++ = hprot.req_num;
	memcpy(tmp_ptr, data, data_len);
	tmp_ptr += data_len;
	*tmp_ptr++ = CalculateFCS(msg_ptr + 4, data_len + 8);
	*tmp_ptr = 0x16;
	hprot.conn_stat = CONN_REQ_SENT;
	TRANS_ON();
	HAL_UART_Transmit_DMA(&huart5, msg_ptr, msg_len);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &huart5) {
		HAL_UART_Receive_IT(&huart5, &received_byte, 1);
		CB_Write(&inbuf_UART, received_byte);
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
	}
}

void StartProcessTask(void const * argument) {
	uint8_t len;
	static uint8_t cmd_data_buf[SIZE_OF_CMD_BUF];

	error_t err = NO_ERR;
	err = CB_Init(&inbuf_UART, UART_BUFF_SIZE);
	if (err != NO_ERR) {
		LogText(INFO_SHOW, SUB_SYS_MEMORY, LOG_LEV_ERR,
				"Circular buffer allocation error.\r\n");
	}
	HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_10);
	osDelay(7000 + hprot.own_address*100);            /* ver 0.6.3 */
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

