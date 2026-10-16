/**************************************************************************//**
 * @file     system_ARMCM4.c
 * @brief    CMSIS Device System Source File for
 *           ARMCM4 Device Series
 * @version  V5.00
 * @date     10. January 2018
 ******************************************************************************/
/*
 * Copyright (c) 2009-2018 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "jat_typedef.h"
#include "a0.h"
#include "a0_hal_sys_ctrl.h"
#include "a0_hal_spi_flash.h"

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define SYSTEM_CLOCK    (192000000UL)         
#define HCLK            (SYSTEM_CLOCK)
#define PCLK            (HCLK / 2)
#define TICKCLK         ((HCLK + 1)/ 3)

/*----------------------------------------------------------------------------
  Externals
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  System Core Clock Variable
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = SYSTEM_CLOCK;
uint32_t SystemHclk = HCLK;
uint32_t SystemPclk = PCLK;
uint32_t SystemTclk = TICKCLK;

/*----------------------------------------------------------------------------
  System Core Clock update function
 *----------------------------------------------------------------------------*/
__RAM_CODE void SystemCoreClockUpdate (uint32_t sysClk)
{
    SystemCoreClock = sysClk;
    SystemHclk = sysClk;
    SystemPclk = SystemHclk / 2;
    SystemTclk = (SystemHclk + 1) / 3;
}

/*----------------------------------------------------------------------------
  System initialization function
 *----------------------------------------------------------------------------*/

void SystemInit (void)
{
    //重新配置Cache范围
    HAL_SPIFC_CACHE_DISABLE();
    HAL_SPIFC_CACHE_RANGE_SET(0x00018000);                     //Cache memory range, [0, 0x18000)

    //if (HAL_SYS_SOFT_RESET_CHECK() != 0)
    {//如果是软件复位，或者看门狗复位，那么要进行系统控制寄存器解锁操作
        HAL_SYS_REG_UNLOCK();
    }

    //配置PLL0，用于系统时钟，基频192MHz, 不展频
    //FVCO=[1000M, 2000M], FOUT = FVCO/2^OD = 192M
    //---->   OD = 3
    //Fint(max) >= Fint = FREF/NR >= Fint(min), FREF/Fint(min) >= NR >= FREF/Fint(max)
    //NR = ceil(FREF/Fint(max)), Fint=[5M, 20M], FREF = Fosc = OSCin = 12M
    //---->   NR = 1, Fint = 12M
    //NF = FOUT * 2^(OD+15) / (Fint), nfx = NF[23:15]=[50,400]
    //---->   NF = 4194304 = 0x400000
    //---->   nfx = 128   ---->   CPA = nfx*2 = 256, CPAX=1
    //NS = 0x1E(默认值）不影响，按照默认值就行
    //NV = 0x002222（默认值）不影响，按照默认值就行
    //Fvco/Fint = 1536/12 = 128
    //---->   RS = 3
    //因为中心频点配置到192M，为了防止不小心配置错误导致系统时钟超范围出错，这里将Type改成向下展频
    //---->   TYPE = 0
    SYS_PLL_CFG00_TypeDef   pllCfg00 = {
        .b = {
            .nr = 1,
            .ns = 0x1E,
            .cpa = 256,
            .fir = 0,
            .mp = 0,
            .type = 0,
            .ssen = 0,
            .pdExt = 0,
            .testExt = 0,
        },
    };
    SYS_PLL_CFG01_TypeDef   pllCfg01 = {
        .b = {
            .nf = 4194304,
            .od = 3,
            .cpax = 1,
        },
    };
    SYS_PLL_CFG02_TypeDef   pllCfg02 = {
        .b = {
            .nv = 0x002222,
            .rs = 3,
        }
    };    

    SYS_CLK_TypeDef         clkCfg;
    clkCfg.w = HAL_SYS_CLK_GET();
    if ((SYS_CTRL->SYS_PLL_CFG00 != pllCfg00.w) || \
        (SYS_CTRL->SYS_PLL_CFG01 != pllCfg01.w) || \
        (SYS_CTRL->SYS_PLL_CFG02 != pllCfg02.w))
    {//PLL配置不一致，则进行配置
        //切到OSC时钟
        clkCfg.b.clkLowSel0 = 0;
        clkCfg.b.clkSysSel = 0;
        HAL_SYS_CLK_SET(clkCfg.w);
        //更改PLL
        pllCfg00.b.pdExt = 1;
        SYS_CTRL->SYS_PLL_CFG00 = pllCfg00.w;
        SYS_CTRL->SYS_PLL_CFG01 = pllCfg01.w;
        SYS_CTRL->SYS_PLL_CFG02 = pllCfg02.w;
        //延时，等PLL稳定
        uint16_t delay = 200;
        while(delay--);
        pllCfg00.b.pdExt = 0;
        SYS_CTRL->SYS_PLL_CFG00 = pllCfg00.w;
        delay = 200;
        while(delay--);
    }
    
    //切到PLL0时钟
    clkCfg.b.clkSysSel = 1;
    HAL_SYS_CLK_SET(clkCfg.w);
    //重新锁定控制寄存器
    HAL_SYS_REG_LOCK();
    //open cache
    HAL_SPIFC_CACHE_EN();

#if defined (__FPU_USED) && (__FPU_USED == 1U)
    SCB->CPACR |= ((3U << 10U*2U) |             /* set CP10 Full Access */
                 (3U << 11U*2U)  );             /* set CP11 Full Access */
#endif

#ifdef UNALIGNED_SUPPORT_DISABLE
    SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
#endif

    SCB->SHCSR |= (SCB_SHCSR_USGFAULTPENDED_Msk | SCB_SHCSR_MEMFAULTPENDED_Msk | SCB_SHCSR_BUSFAULTPENDED_Msk);
    SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;

    return;
}

/* for System tick config and use */
volatile uint32_t SystemTickCount;

__RAM_CODE void HAL_TickInit(void)
{
    SysTick->CTRL  &= (~(SysTick_CTRL_ENABLE_Msk));
    SysTick->LOAD   = ((SystemTclk + 500) / 1000 - 1);             //1ms
    SysTick->VAL    = 0;
    SysTick->CTRL  |= (SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
    return;
}

__RAM_CODE void HAL_TickClr(void)
{
    SystemTickCount = 0;
    return;
}

__RAM_CODE void HAL_TickInc(void)
{
    SystemTickCount++;
}

__RAM_CODE uint32_t HAL_TickGet(void)
{
    return SystemTickCount;
}


__RAM_CODE void HAL_TickDelay(volatile uint32_t Delay)
{
    uint32_t tickstart = HAL_TickGet();
    while(1)
    {
        if ((HAL_TickGet() - tickstart) >= Delay)
        {
            break;
        }
    }
    return;
}
