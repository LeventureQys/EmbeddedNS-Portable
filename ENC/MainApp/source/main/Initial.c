
//#include "app_user_alg.h"
#include "Initial.h"
#include "a0_hal_sys_ctrl.h"
#include "a0_hal_spi_flash.h"
#include "a0_hal_gpio.h"
#include "jat1821.h"
#include "drv_alg.h"

#if ((defined ITM_ENABLE) && (defined TIM_RELEASE_EN))
//extern function in dwt_itm.c
extern void SWO_Init(uint32_t portBits, uint32_t cpuCoreFreqHz);
#endif

#if (SYS_CLK_DOWN_EN == 4)
#include "pll_cfg_tb.c"
//返回新的系统时钟值
__RAM_CODE static uint32_t PllReConfig(void)
{
    SYS_CLK_TypeDef         clkCfg;
    HAL_SYS_REG_UNLOCK();
    clkCfg.w = HAL_SYS_CLK_GET();

    //切到OSC时钟
    clkCfg.b.clkLowSel0 = 0;
    clkCfg.b.clkSysSel = 0;
    HAL_SYS_CLK_SET(clkCfg.w);
    //更改PLL
    pllCfgDefault.pllCfg00.b.pdExt = 1;
    pllCfgDefault.pllCfg00.b.ssen = 0;
    SYS_CTRL->SYS_PLL_CFG00 = pllCfgDefault.pllCfg00.w;
    SYS_CTRL->SYS_PLL_CFG01 = pllCfgDefault.pllCfg01.w;
    SYS_CTRL->SYS_PLL_CFG02 = pllCfgDefault.pllCfg02.w;
    //延时，大于5us
    volatile uint32_t delay = 200;
    while(delay--);
    pllCfgDefault.pllCfg00.b.pdExt = 0;
#if (SYS_CLK_PLL0_CFG_SEL == CLK_190M_SSEN)
    pllCfgDefault.pllCfg00.b.ssen = 1;
#endif
    SYS_CTRL->SYS_PLL_CFG00 = pllCfgDefault.pllCfg00.w;
    //延时，大于80us
    delay = 200;
    while(delay--);
    //切到PLL0时钟
    clkCfg.b.clkSysSel = 1;
    HAL_SYS_CLK_SET(clkCfg.w);
    //重新锁定控制寄存器
    HAL_SYS_REG_LOCK();
    return pllCfgDefault.sysClk;
}
#endif

void TargetResetInit(void)
{
    /* 0. 将JAT1821没有的IO配置成下拉 */
//    JAT1821GpioPulldownConfig();
    JAT1621GpioPulldownConfig();
    /* 1. 根据a0_hal_conf.h中的选项，决定是否使用SWO打印输出功能 */
#ifdef ITM_ENABLE
    #if (SWO_PIN_SELECT == 0)
        HAL_GPIO_AF_SET(GPIO0, 0, GPIO_AF4_SWO);       //use swo for print output, P0.29/P0.0 can be use for swo
    #else
        HAL_GPIO_AF_SET(GPIO0, 29, GPIO_AF3_SWO);       //use swo for print output, P0.29/P0.0 can be use for swo
    #endif
#ifdef TIM_RELEASE_EN    
    SWO_Init(BIT0, SystemCoreClock);              //脱机时需要打印输出，这里就需要初始化，仿真的时候，直接在Debug设置中打开Trace
#endif
#endif

    /* 2. 初始化SystemTick，并通过SWO打印，根据实际用途，可选择是否将打印去除 */
    HAL_TickInit();
    DBG_PRINT("system tick init\n");    

    /* 3. Flash初始化配置，根据a0_hal_spi_flash.h中flash型号即是否支持外部Flash进行初始化 */
    HalFlashInit();
    
    /* 4. IRQ优先级默认配置到最低，为了后续其他模块初始化提升优先级 */
    for (uint8_t i = 0; i <= (uint8_t)IRQn_MAX; i++)
    {
        NVIC_SetPriority((IRQn_Type)i, 7);      //默认全部最低优先级
    }
    
    /* 5. 电源管理，powerdown掉不使用的模块 */
#if (SYS_POWER_SAVE_EN != 0)
    volatile SYS_CLK_TypeDef clkCfg;
    clkCfg.w = HAL_SYS_CLK_GET();

#if (SYS_CLK_DOWN_EN == 1)
    clkCfg.b.clkLowSel0 = 1;            //low from preLow
    clkCfg.b.clkLowSel1 = 8;            //preLow = 2/3 pll0
    uint32_t clkNew = 192000000*2/3;
#elif (SYS_CLK_DOWN_EN == 2)
    clkCfg.b.clkLowSel0 = 1;            //low from preLow
    clkCfg.b.clkLowSel1 = 9;            //preLow = 3/4 pll0
    uint32_t clkNew = 192000000*3/4;
#elif (SYS_CLK_DOWN_EN == 3)
    clkCfg.b.clkLowSel0 = 1;            //low from preLow
    clkCfg.b.clkLowSel1 = 10;           //preLow = 4/5 pll0
    uint32_t clkNew = 192000000*4/5;
#elif (SYS_CLK_DOWN_EN == 4)
    uint32_t clkNew = PllReConfig();
    clkCfg.w = HAL_SYS_CLK_GET();
#endif
    //根据实际需求关闭时钟和RAM

    clkCfg.b.clkEnSdc = 0;              //clk sdc disable
#if (ALG_DSP_CORE_EN)
    clkCfg.b.clkEnDsp = 1;              //clk dsp disable
    clkCfg.b.ramEnDsp = 1;              //ram(0x28000000~0x28007FFF) dsp disable
#else
    clkCfg.b.clkEnDsp = 0;              //clk dsp disable
    clkCfg.b.ramEnDsp = 0;              //ram(0x28000000~0x28007FFF) dsp disable
#endif
    clkCfg.b.ramEnKbd = 0;              //ram kbd disable
//    clkCfg.b.ramEn1 = 0;                //ram1(0x20000000~0x20007FFF) disable
    clkCfg.b.ramEn1 = 1;                //ram1(0x20000000~0x20007FFF) ENABLE
    //clkCfg.b.ramEn01 = 0;               //ram02(0x10008000~0x1000FFFF) disable
    clkCfg.b.ramEn01 = 1; 				//ram02(0x10008000~0x1000FFFF) ENABLE
    
    HAL_SYS_REG_UNLOCK();
    HAL_SYS_CLK_SET(clkCfg.w);
#if ((SYS_CLK_DOWN_EN == 1) || \
     (SYS_CLK_DOWN_EN == 2) || \
     (SYS_CLK_DOWN_EN == 3))
    clkCfg.b.clkSysSel = 0;
    HAL_SYS_CLK_SET(clkCfg.w);
#endif
    HAL_SYS_REG_LOCK();

#if (SYS_CLK_DOWN_EN != 0)
//系统时钟更改过之后，需要重新初始化SWO和SysTick    
    SystemCoreClockUpdate(clkNew);
#if ((defined ITM_ENABLE) && (defined TIM_RELEASE_EN))
    SWO_Init(BIT0, SystemCoreClock);              //脱机时需要打印输出，这里就需要初始化，仿真的时候，直接在Debug设置中打开Trace
#endif

    HAL_TickInit();
    DBG_PRINT("system tick init update\n");  
#endif

#endif

    return;
}


