
/**
  ******************************************************************************
  * @file    drv_key_adc.c
  * @brief   ADC key module.
  * @version V1.0.0
  * @date    2021/09/10
  * @author  ziyingli
  ******************************************************************************
  */
#ifndef __DRV_KEY_ADC_C__
#define __DRV_KEY_ADC_C__

#include "conf_drv_en.h"
#ifdef DRV_KEY_ADC_ENABLED

#include "a0_hal_def.h"
#include "drv_key_adc.h"

/* Exported constants --------------------------------------------------------*/   
#define FILTER_N     (1)  /*滤波次数*/
uint16_t adcValRead[21];               //记录21通道ADC值
volatile uint8_t adcBusyFlag;
uint16_t filter_buf[FILTER_N+1];

/* Exported types ------------------------------------------------------------*/   
 //ADC初始化

/* Exported functions --------------------------------------------------------*/    
void ADC_IRQHandler(void)
{
    adcBusyFlag=0;
    return;
}

//ADC检测及处理
void DrvAdcCore(void)
{  
	static uint32_t adcTimeCnt;
    uint32_t newAdcTimeCnt = HAL_TickGet();
    if((newAdcTimeCnt-adcTimeCnt)>=1000)  //循环读取ADC值，20ms采集一次
    {
		adcTimeCnt = HAL_TickGet();
        while(adcBusyFlag!=0);
        adcBusyFlag=1;
        for (uint8_t i = 0; i < 21; i++)
        {
           // if ((ADC->MCHSR & (1 << i))!=0)
           if ((HAL_ADC_MULTI_CH_SELECT_GET() & (1U << i)) != 0)
            {          
                 adcValRead[i]=HAL_ADC_CH_DATA_GET(i);  //读取ADC值
            }
        }
        HAL_ADC_START();
    }
    return;
}

//处理检测到的adc数据
void DrvKeyAdcProcess(KEY_ADC_PARA_TypeDef *pPara)
{
    uint16_t curVal = adcValRead[pPara->adcCh];
    uint8_t newState = pPara->state;
    for (uint8_t i = 0; i < pPara->keyNum; i++)
    {
		uint16_t *pCampareAdcVal = &pPara->pKeyRangeTb[i*2]; //获取到设置的范围值大小比较		
		if ( (curVal >= *(pCampareAdcVal)) &&  (curVal <= *(pCampareAdcVal + 1)))
        {
            newState = i;
            break;
        }
    }
    if (newState != pPara->state)
    {
        pPara->state = newState;
        pPara->pCb[newState]();  //ADC按键响应函数
    }    
}

//ADC滤波  
uint16_t DrvAdcProcess(ADC_PARA_TypeDef *pPara)
{
	static uint32_t adcTimeCntFilter;
    uint32_t tempData;
    uint16_t filter_sum=0;
    for( uint8_t count = 0; count < FILTER_N; count++)
    {
        filter_buf[count]=adcValRead[pPara->adcCh];
    }
    tempData = HAL_TickGet();
    if((tempData - adcTimeCntFilter) > 50)
    { 
		adcTimeCntFilter = HAL_TickGet();
        for( uint8_t i = 0; i <FILTER_N; i++)
        {
            {
                filter_buf[i+1] = filter_buf[i]; // 所有数据左移，低位仍掉  
                
                filter_sum += filter_buf[i];                    
            }                    
        }               
    }  
    return (uint16_t)(filter_sum/FILTER_N); 
}

#endif

#endif
