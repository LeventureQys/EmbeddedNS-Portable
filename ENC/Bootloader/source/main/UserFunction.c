/****************************************************************************/
/* System Name 	: AP3852													*/
/* File Name 	: UserFunction.c 											*/
/* Version 		: 1.0.0 													*/
/* Histroy 		: 															*/
/* Date 		: 2019/07/10												*/
/* 				Initial version												*/
/* 				by huangzheng 												*/
/****************************************************************************/

#include "Main.h"
#include "Initial.h"
#include "UserFunction.h"

#include "app_dma.h"
#include "app_i2s.h"
//#include "app_user_alg.h"
//#include "app_alg_dsp.h"
#include "drv_alg.h"
#include "drv_uac10.h"

#include "a0_hal_sys_ctrl.h"
#include "a0_hal_gpio.h"
#include "conf_drv_en.h"
#include "conf_demo_or_test.h"
#include "Timer.h"
#include "user_memery.h"
#define IO_ENTER_BOOT_EN                                (0)             // 1, 使用通过IO电平变化进入更新模式
#define IO_ENTER_BOOT_MEM_ADDR                          (0x20007FFC)    // 需要和升级时使用的xml中信息匹配
#define IO_ENTER_BOOT_MEM_VAL                           (0x12345678)    // 需要和升级时使用的xml中信息匹配
#if (IO_ENTER_BOOT_EN != 0)
#define IO_ENTER_BOOT_PORT                              (GPIO0)         // port
#define IO_ENTER_BOOT_PIN                               (18)            // pin
#define IO_ENTER_BOOT_PUPD_MODE                         (GPIO_PULLUP)   // GPIO_PULLUP，内部上拉低电平进入更新模式，GPIO_PULLDOWN，内部下拉高电平进入更新模式
#endif
#define SYSTEM_TICK_SECOND_PRINT_EN                     (1)             //1，使能主循环打印按秒计时（SWO需要使能），   0，不使能

#if (SYSTEM_TICK_SECOND_PRINT_EN != 0)
static uint32_t curTickMs, curTickS;
extern uint32_t g_dmaCycleCnt[5];
#endif

uint32_t g_testPll0Recfg = 0;

void UserEnterUpdateMode(void)
{
    // 确保RAM没有因为省功耗而关闭
    SYS_CLK_TypeDef clkCfg;
    clkCfg.w = HAL_SYS_CLK_GET();
    HAL_SYS_REG_UNLOCK();
    clkCfg.b.ramEn00 = 1;
    clkCfg.b.ramEn01 = 1;
    clkCfg.b.ramEn02 = 1;
    clkCfg.b.ramEn1 = 1;
    HAL_SYS_CLK_SET(clkCfg.w);
    U32REG(IO_ENTER_BOOT_MEM_ADDR) = IO_ENTER_BOOT_MEM_VAL;
    HAL_TickDelay(10);
    NVIC_SystemReset();
    while(1) {;}
}

#include "drv_uac10_user_config.h"
#if(EFFECT_DEBUG == 1)
#include "alg_filter.h"
extern void DspAlgEqParaSet(ALG_PARA_FILTER *pEqPara, uint32_t segNumMax, uint32_t EqIdx);
extern __RAM_CODE void McuAlgEqParaSet(ALG_PARA_FILTER *pPara, uint32_t segNumMax, uint32_t EqIdx);
extern ALG_PARA_FILTER paraDefaultAdcEq[1][5];
extern ALG_PARA_FILTER paraDefaultSpkEq[1][7];
extern ALG_PARA_FILTER paraDefaultDacEq[1][5];
#endif
__RAM_CODE void UserFunctionHidParaCheck(void *pParaIn)
{
    uint32_t *pParaBuff = (uint32_t *)pParaIn;
    if (pParaBuff[0] == 0)
    {
        // 长度为0，无需处理
        return;
    }
    uint32_t address = pParaBuff[1];
    if (address >= 0x8000)
    {
        return;
    }
    
    if (address == 0x3FFF)
    {
        UserEnterUpdateMode();
    }
    switch (address)
    {
        // 根据实际情况添加处理代码
        
#if(EFFECT_DEBUG == 1)
        //===========JinAudioEffect===========
        case (0x00000001)://GAIN
        {

        }
        break;
        case (0x0000000E)://ANS
        {

        }
        break;
        
        case (0x00000010)://MIC EQ
        {
//            pParaBuff[2];//0
//            pParaBuff[3];//段落数
//            pParaBuff[4];//增益
//            pParaBuff[5];//频率
//            pParaBuff[6];//q
//            pParaBuff[7];//类型
            if(pParaBuff[3] < 5)//0-4段 DSP EQ
            {
                paraDefaultAdcEq[0][pParaBuff[3]].gain = *((float *)pParaBuff+4);
                paraDefaultAdcEq[0][pParaBuff[3]].fc = *((float *)pParaBuff+5);
                paraDefaultAdcEq[0][pParaBuff[3]].q = *((float *)pParaBuff+6);
                paraDefaultAdcEq[0][pParaBuff[3]].type = pParaBuff[7];
                DspAlgEqParaSet((ALG_PARA_FILTER *)&paraDefaultAdcEq[0][0], 5, 0);
            }
            else//5-11段 //SPK EQ
            {
                //SPK EQ
                paraDefaultSpkEq[0][pParaBuff[3]-5].gain = *((float *)pParaBuff+4);
                paraDefaultSpkEq[0][pParaBuff[3]-5].fc = *((float *)pParaBuff+5);
                paraDefaultSpkEq[0][pParaBuff[3]-5].q = *((float *)pParaBuff+6);
                paraDefaultSpkEq[0][pParaBuff[3]-5].type = pParaBuff[7];
                McuAlgEqParaSet((ALG_PARA_FILTER *)&paraDefaultSpkEq[0][0], 7, 0);
            }
        }
        break;
        case (0x00000011)://OutPut L EQ
        {
//           pParaBuff[2];//1
//           pParaBuff[3];//段落数
//           (float32_t)pParaBuff[4];//增益
//           (float32_t)pParaBuff[5];//频率
//           (float32_t)pParaBuff[6];//q
//           pParaBuff[7];//类型
                paraDefaultDacEq[0][pParaBuff[3]].gain = *((float *)pParaBuff+4);
                paraDefaultDacEq[0][pParaBuff[3]].fc = *((float *)pParaBuff+5);
                paraDefaultDacEq[0][pParaBuff[3]].q = *((float *)pParaBuff+6);
                paraDefaultDacEq[0][pParaBuff[3]].type = pParaBuff[7];
                McuAlgEqParaSet((ALG_PARA_FILTER *)&paraDefaultDacEq[0][0], 7, 0);
        }
        break;
        case (0x00000012)://OutPut R EQ
        {
//           pParaBuff[2];//2
//           pParaBuff[3];//段落数
//           (float32_t)pParaBuff[4];//增益
//           (float32_t)pParaBuff[5];//频率
//           (float32_t)pParaBuff[6];//q
//           pParaBuff[7];//类型
        }
        break;
        //===========JinAudioEffect===========
#endif        
        default:
            break;
    }
    pParaBuff[0] = 0;
    return;
}


uint8_t testDspRunMode = 255;

extern uint8_t AppAlgDspRunModeSwitch(uint8_t mode);
extern uint8_t timer_300ms_flag;


__RAM_CODE void UserFunctionProcess(void)
{
    /* USB主循环处理 */
    DrvUac10Process();

    return;
}

void UserFunctionInit(void)
{
    /* <0>.定时器初始化 */

    /* <1>. 加载Flash中的配置参数，进行各项配置，方便后续的算法及USB初始化 */
    DrvUac10Init0();
    
    /* <2> 算法初始化 */

    /* <3> DMA初始化 */

    /* <4> I2S包括Codec的初始化 */

    /* <5> USB初始化配置 */
    DrvUsbHidUserParaHandlerCallbackInstall(UserFunctionHidParaCheck);
    DrvUac10Init();

#if (SYSTEM_TICK_SECOND_PRINT_EN != 0)
    curTickMs = HAL_TickGet();
    curTickS = curTickMs / 1000;
#endif

#if (IO_ENTER_BOOT_EN != 0)
    HAL_GPIO_PUPD_SET(IO_ENTER_BOOT_PORT, IO_ENTER_BOOT_PIN, IO_ENTER_BOOT_PUPD_MODE);
#endif
	

    return;
}
