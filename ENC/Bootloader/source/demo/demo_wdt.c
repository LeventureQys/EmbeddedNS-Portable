
#ifndef __DEMO_WDT_C_
#define __DEMO_WDT_C_

#include "demo_wdt.h"

//初始化时调用即可，需要先初始化system tick（即调用HAL_TickInit()之后）
//实际使用时，可直接调用HAL层的看门狗相关函数即可，具体参考 a0_hal_sys_ctrl.c和a0_hal_sys_ctrl.h
void DemoWdtInit(void)
{
    /* 测试看门狗 */
    /* config watchdog count to 1 second */
    HalSysWdtInit(SystemPclk);
    return;
}

void DemoWdtRelead(void)
{
    /* 喂狗 */
    HalSysWdtReload(SystemPclk);
    return;
}

void DemoWdt(void)
{	
	DemoWdtInit();
	while(1)
	{		
		DemoWdtRelead();
	}	
}

#endif
