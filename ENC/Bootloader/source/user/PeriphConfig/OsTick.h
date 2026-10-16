/*
*****************************************Copyright (c)**********************************************
**
**
**                                  深圳摩罗志远科技有限公司
**                             Shenzhen Maono Technology Co.,Ltd
**
**                                        www.maono.cn
**
**
**	文件名	: OsTick.h
**
**	程序员	:
**
**	日　期  : 2023年1月12日
**
**	文件描述: 系统Tick配置，和程序用到的延时函数。
**
****************************************************************************************************
*/
#ifndef _SYS_TICK_H
#define _SYS_TICK_H
#include "UserRetype.h"



// User defined timer selection
#define OS_TIMER_1				((uint32)(1u<<0 ))
#define OS_TIMER_2				((uint32)(1u<<1	))
#define OS_TIMER_3				((uint32)(1u<<2	))
#define OS_TIMER_4				((uint32)(1u<<3	))
#define OS_TIMER_5				((uint32)(1u<<4	))
#define OS_TIMER_6				((uint32)(1u<<5	))
#define OS_TIMER_7				((uint32)(1u<<6	))
#define OS_TIMER_8				((uint32)(1u<<7	))
#define OS_TIMER_9				((uint32)(1u<<8	))
#define OS_TIMER_10				((uint32)(1u<<9	))
#define OS_TIMER_11				((uint32)(1u<<10))
#define OS_TIMER_12				((uint32)(1u<<11))
#define OS_TIMER_13				((uint32)(1u<<12))
#define OS_TIMER_14				((uint32)(1u<<13))
#define OS_TIMER_15				((uint32)(1u<<14))
#define OS_TIMER_16				((uint32)(1u<<15))
#define OS_TIMER_17				((uint32)(1u<<16))
#define OS_TIMER_18				((uint32)(1u<<17))
#define OS_TIMER_19				((uint32)(1u<<18))
#define OS_TIMER_20				((uint32)(1u<<19))
#define OS_TIMER_21				((uint32)(1u<<20))
#define OS_TIMER_22				((uint32)(1u<<21))
#define OS_TIMER_23				((uint32)(1u<<22))
#define OS_TIMER_24				((uint32)(1u<<23))
#define OS_TIMER_25				((uint32)(1u<<24))
#define OS_TIMER_26				((uint32)(1u<<25))
#define OS_TIMER_27				((uint32)(1u<<26))
#define OS_TIMER_28				((uint32)(1u<<27))
#define OS_TIMER_29				((uint32)(1u<<28))
#define OS_TIMER_30				((uint32)(1u<<29))
#define OS_TIMER_31				((uint32)(1u<<30))
#define OS_TIMER_32				((uint32)(1u<<31))


// Tick Flag Bits
#define TFB_SYS_TICK			OS_TIMER_1

#define TFB_DFU_SUC				OS_TIMER_2
#define TFB_CFG_AUTO_SAVE		OS_TIMER_3
#define TFB_PRESET_AUTO_SAVE	OS_TIMER_4

#define TFB_HID_RX_TO			OS_TIMER_5
#define TFB_DFU_TIME_OUT        OS_TIMER_6


typedef struct
{
	void   (*Init   )(void);
	uint32 (*TimerMs)(uint32 *cnt, uint32 bit);
	void   (*DelayMs)(uint32 nMs);
	void   (*DelayUs)(uint32 nUs);
	void   (*Delay  )(uint32 nCount);
} OS_TICK_HANDLE;

extern OS_TICK_HANDLE OsTick;

#endif




