
#include "Timer.h"
#include "a0_hal_timer.h"

//定义定时器变量
static volatile uint32_t timerMsCount;
static volatile uint32_t timerSynthCount;

/*
确认a0_it.c中是否定义了下列Timer的IRQ响应函数
TIMER0_IRQHandler
TIMER1_IRQHandler
TIMER2_IRQHandler
TIMER3_IRQHandler
TIMER_DUAL_IRQHandler

void TIMER0_IRQHandler(void)
{
    hal_timer_irq_handler(0);
    HAL_TIM_INT_CLR(TIM0);
    return;
}
*/
uint8_t timer_300ms_flag = 0;
//定义定时器响应函数
__RAM_CODE void timer_irq_synth(void)
{
    timerSynthCount++;
    return;
}
__RAM_CODE void timer_irq_1ms(void)
{
    timerMsCount++;
	if (timerMsCount%300 == 0)
	{
		timer_300ms_flag = 1;
	}
    return;
}

//Timer0作为毫秒定时器，提供相关接口函数
__RAM_CODE uint32_t TimeMsGet(void)
{
    return (timerMsCount);
}
__RAM_CODE uint32_t GetMs(uint32_t lastTimeMs)
{
	return (timerMsCount - lastTimeMs);
}
__RAM_CODE void Timer_Delay(uint32_t ms)
{
    uint32_t cur = GetMs(0);
    while(1)
    {
        if (GetMs(cur) >= ms)
        {
            break;
        }
    }
    return;
}
void ClrMs(void)
{
	timerMsCount = 0;
}
//Timer1提供的函数接口
uint32_t KernTimerCounterGet(void)
{
    return (timerSynthCount);
}
void KernTimerCounterClr(void)
{
    timerSynthCount = 0;
    return;
}

//定时器配置信息初始化
const TIM_InitTypeDef timer0Init = {
    .cb = timer_irq_1ms,
    .timerIdx = 0,
    .time = 1.0f,               //1ms
};

const TIM_InitTypeDef timer1Init = {
    .cb = timer_irq_synth,
    .timerIdx = 1,
    .time = 16.0f,              //16ms, 512samples @ 32KFS
};


//定时器初始化
void TimerInitial(void)
{
    if (hal_timer_init((TIM_InitTypeDef *)&timer0Init) != 0)
    {
        __breakpoint(0);
    }
    if (hal_timer_init((TIM_InitTypeDef *)&timer1Init) != 0)
    {
        __breakpoint(0);
    }
    
    //Dual timer0 initial
//    DTIM_CR_TypeDef dTimCR;
//    dTimCR.b.timerSize = 1; //32-bit counter
//    dTimCR.b.timerPre = 0;  //div256 from pclk, 2.67us@192M, 3.3us@192M*0.8, 3.56us@192M*0.75, 4us@192M*0.67
//    dTimCR.b.enable = 1;    //enable
//    HAL_DTIM_CTRL_SET(DTIM0, dTimCR.w);

}





