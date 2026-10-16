/*****************************************************************************/
/* System Name 	: A0   													     */
/* File Name 	: Main.c 													 */
/* Version 		: 1.0.0 													 */
/* Histroy 		: 															 */
/* Date 		: 2022/03/18												 */
/* 				Initial version												 */
/* 				by Huangzheng 												 */
/*****************************************************************************/
#include "main.h"
#include "Timer.h"
#include "Initial.h"
#include "UserFunction.h"
#include <stddef.h>
#include <stdbool.h>
#include "agent.h"

__RAM_CODE int main()
{
    SCB->VTOR = ((uint32_t)&__Vectors & 0xFFFFFF80);

    /* 硬件相关初始化操作 */
    TargetResetInit();
    
    OsTick.Init();
	Crc32.Init();
//	Gpio.Init();
//	Spi.Init();
	Hid.Init();
    
    Dfu.Init();
	ParamTbl.Init();
	Com.Init();
	Ui.Init();
    
    /* 用户软件初始化操作 */
    UserFunctionInit();
    
//  Dsp.Init();
//	Mem.Init();
//	Ui.Init();
//	Com.Init();
//	Dsp.Program->Unmute();
    
	while(1)
	{
		Com.Task();
		Dfu.Task();
        UserFunctionProcess();						//其他用户程序处理
	}
}

// 引导判断，可以是硬件方式，也可以是软件方式
uint32 BootCheck(void)
{
	uint8 jump = *((uint8*)(FLASH_DFU_SB_ADDR + 1));
	
#ifndef __DEBUG
	if (jump == BL_JUMP_TO_APP)
	{
		return 0x00008000;		// MainApp
	}
	else
	{
		return 0x00000000;		// Bootloader
	}
#endif
}

