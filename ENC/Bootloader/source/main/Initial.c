
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
    JAT1821GpioPulldownConfig();

    /* 1. 根据a0_hal_conf.h中的选项，决定是否使用SWO打印输出功能 */

    /* 2. 初始化SystemTick，并通过SWO打印，根据实际用途，可选择是否将打印去除 */
    HAL_TickInit();   

    /* 3. Flash初始化配置，根据a0_hal_spi_flash.h中flash型号即是否支持外部Flash进行初始化 */
    HalFlashInit();
    
    /* 4. IRQ优先级默认配置到最低，为了后续其他模块初始化提升优先级 */
    for (uint8_t i = 0; i <= (uint8_t)IRQn_MAX; i++)
    {
        NVIC_SetPriority((IRQn_Type)i, 7);      //默认全部最低优先级
    }
    
    /* 5. 电源管理，powerdown掉不使用的模块 */

#if (SYS_CLK_DOWN_EN != 0)
//系统时钟更改过之后，需要重新初始化SWO和SysTick    
    SystemCoreClockUpdate(clkNew);
#if ((defined ITM_ENABLE) && (defined TIM_RELEASE_EN))
    SWO_Init(BIT0, SystemCoreClock);              //脱机时需要打印输出，这里就需要初始化，仿真的时候，直接在Debug设置中打开Trace
#endif

    HAL_TickInit();
    DBG_PRINT("system tick init update\n");  
#endif

    return;
}


