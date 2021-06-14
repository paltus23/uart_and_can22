/*
 * uart.h
 *
 *  Created on: Jun 6, 2021
 *      Author: maksim
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "common.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#define UART_TX_BUFF_SIZE 50
#define UART_CIRC_BUFF_SIZE 128
void MX_USART1_UART_Init(void);
void uart_send(char* buff, u32 len);
void vUartTXTask(void *pvParametrs);
void vUartRXTask(void *pvParametrs);

extern TaskHandle_t uartTXTaskHandle;
extern TaskHandle_t uartRXTaskHandle;
QueueHandle_t uart_tx_queue;
#endif /* INC_UART_H_ */
