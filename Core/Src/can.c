#include "can.h"
#include "buffs.h"

CAN_HandleTypeDef hcan;
CAN_TxHeaderTypeDef pHeader;
CAN_RxHeaderTypeDef pRxHeader;
u32 TxMailbox;
CAN_FilterTypeDef sFilterConfig;
u8 tx_msg[8]={45,76,22,77,99,11,22,33};
u8 rx_msg[8];

#define CAN_CIRC_BUFF_SIZE 128
u8 _can_rx_circ_buff[CAN_CIRC_BUFF_SIZE];
circ_buff_t can_rx_circ_buff = {_can_rx_circ_buff, CAN_CIRC_BUFF_SIZE, 0, 0, 0};

buff_t* buff_tx;
TaskHandle_t canTaskHandle = NULL;

void can_send(u8* buff, u32 len);

QueueHandle_t can_tx_queue;
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 8;
  hcan.Init.Mode = CAN_MODE_LOOPBACK;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_15TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  	sFilterConfig.FilterFIFOAssignment=CAN_FILTER_FIFO0; //set fifo assignment
	sFilterConfig.FilterIdHigh=0x8; //the ID that the filter looks for (switch this for the other microcontroller)
	sFilterConfig.FilterIdLow=0x8;
	sFilterConfig.FilterMaskIdHigh=0x0;
	sFilterConfig.FilterMaskIdLow=0x0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale=CAN_FILTERSCALE_16BIT; //set filter scale
	sFilterConfig.FilterActivation=ENABLE;

	HAL_CAN_ConfigFilter(&hcan, &sFilterConfig); //configure CAN filter


	HAL_CAN_Start(&hcan);
	HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
	HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);


	can_tx_queue = xQueueCreate( 5, sizeof(buff_t*));
}
char foo_str[]="qqwweerr11aassddff22zzxxccvv33ttyyuuii44ffgghhjj55vvbbnnmm66";
void vCanTask( void *pvParametrs ){

	pHeader.IDE=CAN_ID_STD;
	pHeader.RTR=CAN_RTR_DATA;

	pHeader.StdId=0x1;

	while(1){
//		vTaskDelay(500);
//		can_send(foo_str, strlen(foo_str));
		 if(xQueueReceive( can_tx_queue, &buff_tx, 10) == pdPASS ){
			 can_send(buff_tx->p_buff, buff_tx->len);
			 buff_clear((*buff_tx));
		 }

	}
}
void can_send(u8* buff, u32 len){

	u32 sended_len = 0;
	pHeader.DLC=8;
	while(len - sended_len >8 ){

		while(HAL_CAN_AddTxMessage(&hcan, &pHeader, buff+sended_len, &TxMailbox) != HAL_OK);
		sended_len +=8;
	}
	pHeader.DLC= len - sended_len;
	while(HAL_CAN_AddTxMessage(&hcan, &pHeader, buff+sended_len, &TxMailbox) != HAL_OK);

}
void CAN1_RX0_IRQHandler(void)
{
  /* USER CODE BEGIN CAN1_RX0_IRQn 0 */

  /* USER CODE END CAN1_RX0_IRQn 0 */
  HAL_CAN_IRQHandler(&hcan);
  /* USER CODE BEGIN CAN1_RX0_IRQn 1 */
  HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &pRxHeader, rx_msg);
  circ_buff_add(&can_rx_circ_buff, rx_msg, pRxHeader.DLC);

  /* USER CODE END CAN1_RX0_IRQn 1 */
}
