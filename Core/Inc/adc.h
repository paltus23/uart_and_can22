#ifndef INC_ADC_H_
#define INC_ADC_H_

#include "FreeRTOS.h"
#include "common.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "uart.h"

void MX_ADC1_Init(void);
void vAdcTask( void *pvParametrs );
extern TaskHandle_t adcTaskHandle;
extern SemaphoreHandle_t xAdcSemaphore;

#define ADC_PERIOD 10



#endif /* INC_ADC_H_ */
