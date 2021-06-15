#include <stm32f103xb.h>
#include "uart.h"
#include "can.h"
#include "buffs.h"
QueueHandle_t uart_tx_queue;
char uart_tx_buff[UART_TX_BUFF_SIZE] = {0,};
u32 uart_tx_buff_size = 0;

TaskHandle_t uartTXTaskHandle = NULL;
TaskHandle_t uartRXTaskHandle = NULL;
UART_HandleTypeDef huart1;


u8 _uart_rx_circ_buff[UART_CIRC_BUFF_SIZE];
circ_buff_t uart_rx_circ_buff = {_uart_rx_circ_buff, UART_CIRC_BUFF_SIZE, 0, 0, 0};

#define UART_TO_CAN_BUFF_CNT 2
u8 _uart_to_can_buff[UART_TO_CAN_BUFF_CNT][UART_CIRC_BUFF_SIZE];
buff_t uart_to_can_buff[UART_TO_CAN_BUFF_CNT];
u8 fl_send = 0;

void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 921600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
//----UART1 TX to DMA----
  USART1->CR3 |= USART_CR3_DMAT;
  DMA1_Channel4->CPAR = &USART1->DR;

  DMA1_Channel4->CMAR = (u32)uart_tx_buff;
  DMA1_Channel4->CCR &= (~(DMA_CCR_MSIZE_0 | DMA_CCR_MSIZE_1)); //size 8-bit
  DMA1_Channel4->CCR |= DMA_CCR_MINC |  DMA_CCR_TCIE | DMA_CCR_DIR;

  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 10, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

  uart_tx_queue = xQueueCreate( 5, sizeof(char*));
//----UART1 RX to DMA----
  USART1->CR3 |= USART_CR3_DMAR;
  DMA1_Channel5->CPAR = &USART1->DR;

  DMA1_Channel5->CMAR = (u32)uart_rx_circ_buff.p_buff;
  DMA1_Channel5->CCR &= (~(DMA_CCR_MSIZE_0 | DMA_CCR_MSIZE_1)); //size 8-bit
  DMA1_Channel5->CCR |= DMA_CCR_MINC | DMA_CCR_TCIE | DMA_CCR_CIRC;
  DMA1_Channel5->CNDTR = UART_CIRC_BUFF_SIZE;

  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 10, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);


//----init uart buffers for can
  for(u32 i =0; i < UART_TO_CAN_BUFF_CNT; i++){
	  uart_to_can_buff[i].p_buff = _uart_to_can_buff[i];
	  uart_to_can_buff[i].cap = UART_CIRC_BUFF_SIZE;
	  uart_to_can_buff[i].len = 0;

  }
}


void vUartTXTask ( void *pvParametrs ){



	while(1){

		char * msg;
		if(fl_send == 0){
		 if(xQueueReceive( uart_tx_queue, &msg, 10) == pdPASS ){
			 uart_send(msg, strlen(msg));

		 }
		}

	}
}

void uart_send(char* buff, u32 len){
	if(fl_send)
		return;
	memcpy(uart_tx_buff, buff, len);

	DMA1_Channel4->CCR &= (~DMA_CCR_EN);
	DMA1_Channel4->CNDTR = len;
	DMA1_Channel4->CCR |= DMA_CCR_TCIE;
	DMA1_Channel4->CCR |= DMA_CCR_EN;
	fl_send = 1;
}
u32 i_buff=0;
void vUartRXTask ( void *pvParametrs ){


	DMA1_Channel5->CCR |= DMA_CCR_EN;
	while(1){

		uart_rx_circ_buff.pos_write = UART_CIRC_BUFF_SIZE - DMA1_Channel5->CNDTR;

		if(uxQueueMessagesWaiting(can_tx_queue) < UART_TO_CAN_BUFF_CNT){
			i32 len = circ_buff_get(&uart_rx_circ_buff, uart_to_can_buff[i_buff].p_buff, uart_rx_circ_buff.cap);
			uart_to_can_buff[i_buff].len = len;
			if(uart_to_can_buff[i_buff].len){
				buff_t* _b = uart_to_can_buff+i_buff;
				xQueueSend(can_tx_queue,  ( void * ) &_b, 10);
				i_buff++;
				i_buff = i_buff % UART_TO_CAN_BUFF_CNT;
			}
		}

	}
}



void DMA1_Channel4_IRQHandler(void){
	if(DMA1->ISR & DMA_ISR_TCIF4){

		DMA1->IFCR = DMA_IFCR_CTCIF4;
		fl_send = 0;

	}
}
void DMA1_Channel5_IRQHandler(void){
	if(DMA1->ISR & DMA_ISR_TCIF5){

		DMA1->IFCR = DMA_IFCR_CTCIF5;
		uart_rx_circ_buff.fl_overflow = 1;

	}
}
