
#ifndef __DEMO_ADC_C_
#define __DEMO_ADC_C_

#include "a0_hal_def.h"
#include "demo_adc.h"
#include "Timer.h"

#define ADC_PORT				(GPIO0)
#define ADC_PIN					(0)
#define ADC_CH_USE	    		(13)

ADC_PARA_TypeDef  ADCch[1]=
{
	{
		.adcCh=ADC_CH_USE,
	}    
};

uint16_t filterValue;  

ADC_InitTypeDef adcInitVar = 
{
	.chSel = ((1 << ADC_CH_USE)),
	.adcClk = 16000000,
	.delayCycle = 15,
};

void DemoAdcInit(void)
{
	//AD复用配置
	HAL_GPIO_AF_SET(ADC_PORT,ADC_PIN,GPIO_AF2_AIN13);
    //使能中断
    NVIC_EnableIRQ(ADC_IRQn); 
	
    HalAdcInit(&adcInitVar);
}

void DemoAdcProcess(void)
{
    DrvAdcCore();
	filterValue = DrvAdcProcess(&ADCch[0]);
}

void DemoAdc(void)
{	
	DemoAdcInit();
	while(1)
	{		
		DemoAdcProcess();
	}	
}

#endif
