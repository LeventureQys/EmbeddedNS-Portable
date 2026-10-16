/*****************************************************************************/
/* System Name 	: A0   													     */
/* File Name 	: Main.c 													 */
/* Version 		: 1.0.0 													 */
/* Histroy 		: 															 */
/* Date 		: 2022/03/18												 */
/* 				Initial version												 */
/* 				by Huangzheng 												 */
/*****************************************************************************/
#include "Main.h"
#include "Timer.h"
#include "Initial.h"
#include "UserFunction.h"
#include <stddef.h>
#include <stdbool.h>
#include "agent.h"

//Enc
#include "arm_math.h"
#include "enc.h"

int main()
{
    /*系统初始化*/
#if 0
    SystemCoreInit();
#else  
    SCB->VTOR = ((uint32_t)&__Vectors & 0xFFFFFF80);
//  HAL_SYS_USB_ID_SET(); //放入DrvUac10Init
//  HAL_SYS_USB_VBUS_EN();
#endif

    /* 硬件相关初始化操作 */
    TargetResetInit();
    
    OsTick.Init();
    Crc32.Init();
    Gpio.Init();
    Spi.Init();
    Hid.Init();

#if 1 //niko 所有硬件中断
    /* 用户软件初始化操作 */
    UserFunctionInit();

    Dsp.Init();
    
//    Mem.Init();
    Ui.Init();
    Com.Init();
    Dsp.Program->Unmute();
#endif

    //Enc init
    // Enc_Init(&g_EncParam, enc_level);

#if 0
    for (UINT16 i = 0; i < 960; i++)//  Niko 装载正弦波数据到enc_in
    {
//        double val;
//        val = (0x7FFF / 2) * (sin((double)(i%48) / (double)48 * 2 * 3.141592654));        
//        enc_in[i] = ((int32_t )val);//正弦
        enc_in[i] = (int32_t )i;//0-960
    }
#endif
    
//    Enc_Process(&g_EncParam, enc_in, enc_out);

#if 0 //测试算法运行情况
UINT16 j = 0xFFFF;
while(1)
{
    Enc_Process(&g_EncParam, enc_in, enc_out);
//    DELAY_MS(10);
    
    while(j--);
    j = 0xFFFF;
}
#endif
    
	while(1)
	{
//      Enc_Process(&g_EncParam, enc_in, enc_out);

        Ui .Task();
//        Mem.Task();
        Dsp.Task();
        Com.Task();
        Fwu.Task();
        UserFunctionProcess();						//其他用户程序处理
	}
}


