#ifndef __DEMO_DUAL_TIMER_C_
#define __DEMO_DUAL_TIMER_C_


#include "demo_dual_timer.h"
#include "drv_dual_timer.h"
#include "a0_hal_gpio.h"  

static volatile uint32_t dualtimerMsCount;				/* ms timer counter */
static volatile uint32_t dualtimerSynthCount;

uint32_t delatTime;
uint32_t startTime;
void DualTimer0IrqHandle(void)  //dual_timer 0
{
	#if 0  //old version
   uint32_t endtime=HAL_TickGet();
   delatTime=endtime-startTime;
    if(delatTime!=0)
    {
        __breakpoint(0);       
    }
    __breakpoint(0); 
    return;
	#else
	dualtimerSynthCount++;
	#endif
}
__RAM_CODE void DualTimer1IrqHandle(void)  //dual_timer 1
{    
     dualtimerMsCount++;  	
     return;
}
__RAM_CODE uint32_t DualTimeMsGet(void)
{
    return (dualtimerMsCount);
}

__RAM_CODE uint32_t GetMs1(uint32_t lastTimeMs)
{
	return (dualtimerMsCount - lastTimeMs);
}

__RAM_CODE void DualTimerDelay(uint32_t ms)
{
    uint32_t cur = GetMs1(0);
    while(1)
    {
        if (GetMs1(cur) >= ms)
        {
            break;           
        }
    }
    return;
}

void ClrMs1(void)
{
	dualtimerMsCount = 0;
}

const Dual_TIM_INIT_TypeDef dtimerDefault[2]={
    {    
        .cb=DualTimer0IrqHandle, //中断回调函数
        .timerIdx=0,
        .time=8000.0f,  //8000ms进入一次中断
    },
    {
        .cb=DualTimer1IrqHandle,
        .timerIdx=1,
        .time=1.0f,
    }
};

//dual-timer初始化
void DualTimerInitial(void)
{
    if(HalDualTimerInit((Dual_TIM_INIT_TypeDef*)&dtimerDefault[0])!=0)
    {   
     __breakpoint(0);  
    }
    if(HalDualTimerInit((Dual_TIM_INIT_TypeDef*)&dtimerDefault[1])!=0)
    {   
     __breakpoint(0);  
    }
}

//dual-timer0测试
void DemoTimer0(void)
{
	DualTimerInitial();
    while(1);	
}

//dual-timer1测试
void DemoTimer1(void)
{
	DualTimerInitial();
    HAL_GPIO_AF_SET(GPIO0,31, GPIO_AF0);
	HAL_GPIO_OUT_EN(GPIO0,BIT31);
    while(1)
    {
    
        HAL_GPIO_BIT_SET(GPIO0, BIT31);            
        DualTimerDelay(1000);
        HAL_GPIO_BIT_CLR(GPIO0, BIT31);   
        DualTimerDelay(1000);   
    }
}

//中断处理
void TIMER_DUAL_IRQHandler(void)
{
	if(HAL_DTIM_INT_CHECK(DTIM0) != 0)
	{		
		DualTimer0IrqHandle();	
	}
	if(HAL_DTIM_INT_CHECK(DTIM1) != 0)
	{		
		DualTimer1IrqHandle();		
	}	
	HAL_DTIM_INT_CLR(DTIM0);//标志位清除
	HAL_DTIM_INT_CLR(DTIM1);//标志位清除
}

#endif
