#ifndef __TEST_DRV_KEY_ADC_C_
#define __TEST_DRV_KEY_ADC_C_

#include "a0_hal_def.h"
#include "test_drv_key_adc.h"
#include "Timer.h"

//adc 通道13，14adc按键回调函数
void TestDrvKeyAdcCh13Rang1Event(void);
void TestDrvKeyAdcCh13Rang2Event(void);
void TestDrvKeyAdcCh13Rang3Event(void);
void TestDrvKeyAdcCh13Rang4Event(void);
void TestDrvKeyAdcCh14Rang1Event(void);
void TestDrvKeyAdcCh14Rang2Event(void);
void TestDrvKeyAdcCh14Rang3Event(void);
void TestDrvKeyAdcCh14Rang4Event(void);

//1、adc按键配置adc pin 相关信息
#define ADC_PORT				(GPIO0) 
#define ADC_USE_PIN_NUM			(2)
#define ADC_PIN_USE1			(0)
#define ADC_PIN_USE2			(1)
#define ADC_CH_USE1	    		(13)
#define ADC_CH_USE2	    		(14)
#define ADC1_KEY_NUM    		(4)
#define ADC2_KEY_NUM    		(4)


//2、adc按键配置adc 按键电压等级范围
//adc13 按键4个等级电压范围
const uint16_t keyAdcCh13RangeTb[][2] = {
   {0,30},{50,70},{100,150},{200,0x3ff},
};

//adc14 按键4个等级电压范围
const uint16_t keyAdcCh14RangeTb[][2] = {
   {0,30},{50,70},{100,150},{200,0x3ff},
};

//3、adc按键配置adc 按键回调函数
FUNCP keyAdcCh13RangeCb[4] = {
    TestDrvKeyAdcCh13Rang1Event,
    TestDrvKeyAdcCh13Rang2Event,
    TestDrvKeyAdcCh13Rang3Event,
    TestDrvKeyAdcCh13Rang4Event,  
};
FUNCP keyAdcCh14RangeCb[4] = {
    TestDrvKeyAdcCh14Rang1Event,
    TestDrvKeyAdcCh14Rang2Event,
    TestDrvKeyAdcCh14Rang3Event, 
	TestDrvKeyAdcCh14Rang4Event, 
};

//4、调用adc按键函数
void TestDrvKeyAcd(void)
{	
	TestDrvAdcinit();	
	while(1)
	{		
		TestDrvKeyAdcProcess();		
	}
}

KEY_ADC_PARA_TypeDef  keyAdc[ADC_USE_PIN_NUM] = {
    {
        .adcCh = ADC_CH_USE1,
        .keyNum = ADC1_KEY_NUM,
        .pKeyRangeTb = (uint16_t *)keyAdcCh13RangeTb,
        .pCb = keyAdcCh13RangeCb,
    },
    {
        .adcCh = ADC_CH_USE2,
        .keyNum = ADC2_KEY_NUM,
        .pKeyRangeTb = (uint16_t *)keyAdcCh14RangeTb,
        .pCb = keyAdcCh14RangeCb,
    },
};

//ADC初始化
void TestDrvAdcinit(void)
{    	
	//AD复用IO配置
	HAL_GPIO_AF_SET(ADC_PORT,ADC_PIN_USE1,GPIO_AF2_AIN13);	
	HAL_GPIO_AF_SET(ADC_PORT,ADC_PIN_USE2,GPIO_AF2_AIN14);
	
	ADC_InitTypeDef adcInitVar = 
	{
		.chSel = ((1 << ADC_CH_USE1)|(1 << ADC_CH_USE2)),
		.adcClk = 16000000,
		.delayCycle = 15,
	};
		
    //使能中断
    NVIC_EnableIRQ(ADC_IRQn); 	
    HalAdcInit(&adcInitVar);
}


//ADC按键检测
void TestDrvKeyAdcProcess(void)
{
	DrvAdcCore();//ADC检测及处理;
    DrvKeyAdcProcess(&keyAdc[0]);//使用几个ADC通道调用几次,要匹配
    DrvKeyAdcProcess(&keyAdc[1]);
}

void TestDrvKeyAdcCh13Rang1Event(void)
{

    return;
}

void TestDrvKeyAdcCh13Rang2Event(void)
{

    return;
}

void TestDrvKeyAdcCh13Rang3Event(void)
{

    return;
}

void TestDrvKeyAdcCh13Rang4Event(void)
{

    return;
}

void TestDrvKeyAdcCh14Rang1Event(void)
{

    return;
}

void TestDrvKeyAdcCh14Rang2Event(void)
{

    return;
}

void TestDrvKeyAdcCh14Rang3Event(void)
{

    return;
}

void TestDrvKeyAdcCh14Rang4Event(void)
{

    return;
}

#endif
