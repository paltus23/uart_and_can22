/*
 * can.h
 *
 *  Created on: Jun 11, 2021
 *      Author: maksim
 */

#ifndef SRC_CAN_H_
#define SRC_CAN_H_

#include "common.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
extern TaskHandle_t canTaskHandle;

void MX_CAN_Init(void);
void vCanTask( void *pvParametrs );

void CAN1_RX0_IRQHandler(void);
extern QueueHandle_t can_tx_queue;

#endif /* SRC_CAN_H_ */
