#include "adc.h"

#define VCC 3300
#define ADC_MAX_VALUE 4095
#define V25 1430
#define V25_BITS (V25*ADC_MAX_VALUE/VCC)
#define AVG_SLOPE_NUM 43
#define AVG_SLOPE_DEN 10
#define AVG_SLOPE (AVG_SLOPE_NUM/AVG_SLOPE_DEN)

SemaphoreHandle_t xAdcSemaphore = NULL;

TaskHandle_t adcTaskHandle = NULL;

ADC_HandleTypeDef hadc1;

#define ADC_VALUES_CNT 1
u16 adc_values[ADC_VALUES_CNT*2]={0,};
u16 voltage_out;
i16 temperature_out;

char str[30];
void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE; //ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
//    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
//    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
//    Error_Handler();
  }

  ADC1->CR2 |= ADC_CR2_DMA;
  ADC1->CR2 |= ADC_CR2_ADON|ADC_CR2_EXTTRIG;
  //init dma
  __HAL_RCC_DMA1_CLK_ENABLE();

  DMA1_Channel1->CMAR = (u32)adc_values;
  DMA1_Channel1->CPAR = &ADC1->DR;

  DMA1_Channel1->CNDTR = ADC_VALUES_CNT*2;

  DMA1_Channel1->CCR |= DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0; //size 16-bit
  DMA1_Channel1->CCR |= DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_TCIE;

  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 10, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);


	ADC1->CR2 |= ADC_CR2_CAL;
	while(ADC1->CR2 & ADC_CR2_CAL);

	xAdcSemaphore = xSemaphoreCreateBinary();
}

TickType_t xLastWakeTime =0;
void vAdcTask( void *pvParametrs ){


	xLastWakeTime = xTaskGetTickCount();

	while(1){

		vTaskDelayUntil( &xLastWakeTime, ADC_PERIOD);
//		vTaskDelay( ADC_PERIOD);
		DMA1_Channel1->CCR |= DMA_CCR_EN;
		ADC1->CR2 |= ADC_CR2_SWSTART;

        ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

        {
//			voltage_out = 0;
//			temperature_out = 0;
//			for (u32 i=0; i<ADC_VALUES_CNT;i++){
//				voltage_out += adc_values[i*2];
//				temperature_out += adc_values[i*2+1];
//
//			}
//			voltage_out /= ADC_VALUES_CNT;
//			temperature_out /= ADC_VALUES_CNT;

			voltage_out = adc_values[0];
			temperature_out = adc_values[1];


			voltage_out = voltage_out*VCC/ADC_MAX_VALUE;
			temperature_out = (V25_BITS - temperature_out);
			temperature_out = temperature_out*VCC/ADC_MAX_VALUE/AVG_SLOPE+25;
			sprintf(str, "Temperature: %d C | Voltage %d.%02d V\r\n", temperature_out, voltage_out/1000, (voltage_out%1000)/10);

			char* p = str;
			xQueueSend(uart_tx_queue, &p, 10);

        }


	}
}
void DMA1_Channel1_IRQHandler(void){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if(DMA1->ISR & DMA_ISR_TCIF1){

		DMA1->IFCR = DMA_IFCR_CTCIF1;
	    vTaskNotifyGiveFromISR(adcTaskHandle,
	                         &xHigherPriorityTaskWoken);
	    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

//        xSemaphoreGiveFromISR( xAdcSemaphore, &xHigherPriorityTaskWoken );
//        DMA1->IFCR = DMA_IFCR_CTCIF1;
	}
}
