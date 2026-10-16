/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_i2s.c
 * @brief   I2S HAL module.
 * @version V1.0.4
 * @date    2022/04/07
 * @author  huangzheng
 ******************************************************************************
 */

/**
 * 版本日志：
 * V1.0.4, 2022/04/07
 *      1. 函数命名按照大驼峰方式命名，头文件增加原函数名的define，为了兼容原代码，后续希望渐渐过渡到统一风格书写规范
 *      2. 整理头文件中依赖关系，使用jat_typedef.h作为数据类型的头文件，并把include放到extern "C" 之外
 *      3. 将一些配置选项移到a0_hal_conf.h，统一进行配置
 * V1.0.3, 2021/10/22,
 *      1. 系统控制寄存器的修改，增加先解锁(HAL_SYS_REG_UNLOCK)，改完锁定的操作(HAL_SYS_REG_LOCK)，用于保护系统控制寄存器不被随意改动
 * V1.0.2, 2021/10/11,
 *      1. 头文件中的注释部分调整，适应Doxygen
 * V1.0.1, 2021/05/27,
 *      1. 添加通用I2S初始化函数，默认屏蔽I2S中断，使用DMA，DMA配置用户代码自行配置
 * V1.0.0, 2021/04/21,
 *      1. 不配置IO复用，需要上层应用控制IO复用
 *      2. 头文件包含寄存器操作宏
 */
/* Includes -----------------------------------------------------------------*/
#include "a0_hal_conf.h"
/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup I2S
 * @{
 */

#ifdef HAL_I2S_MODULE_ENABLED
#include "a0_hal_i2s.h"
#include "a0_hal_sys_ctrl.h"
#include "a0_hal_def.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
static const SYS_PLL_CFG1_TypeDef i2sPll2CfgTb[2] =
{
    {
        /* pll1 out 192M, pll2 out 45.1584M */
        .b = {
            .pll1En = 1,
            .pll1Range = 3,
            .pll1M = 1,
            .pll1N = 16,
            .pll2En = 1,
            .pll2M = 25,
            .pll2N = 147,
        },
    },
    {
        /* pll1 out 192M, pll2 out 49.152M */
        .b = {
            .pll1En = 1,
            .pll1Range = 3,
            .pll1M = 1,
            .pll1N = 16,
            .pll2En = 1,
            .pll2M = 20,
            .pll2N = 128,
        },
    },
};

/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Private function prototypes ----------------------------------------------*/
/* Exported functions -------------------------------------------------------*/
/**
 * @defgroup I2S_Exported_Functions I2S Exported Functions
 * @{
 */
void HalI2sInit(I2S_InitTypeDef *pInit)
{
    NVIC_DisableIRQ(I2S_IRQn);
    HAL_I2S_CH_CTRL0_SET(pInit->ccr0.w);
    HAL_I2S_CH_CTRL1_SET(pInit->ccr1.w);
    /* fill zero data to out fifo, 16 for 7 fifo */
    for (uint8_t i = 0; i < 16; i++)
    {
        for (uint8_t j = 0; j < 7; j++)
        {
            HAL_I2S_FIFO_WR_L(j, 0);
            HAL_I2S_FIFO_WR_R(j, 0);
        }
    }
    HAL_I2S_CTRL_SET(pInit->cr.w);
    return;
}

/**
 * @brief  config the i2s clk source, if i2s is selected from pll2 out, pll2 clk will be configed, and enable pll2, otherwise, pll2 will be powerdown
 * @param  adj: pll2 fine adjust, -1, N of pll2 -1 (pll2 out freq decrease), +1, N of pll2 +1 (pll2 out freq increase)
 * @return NONE
 */
void HalI2sClkFineAdjust(int32_t adj)
{
    SYS_PLL_CFG1_TypeDef    pllCfg1;
    pllCfg1.w = HAL_SYS_PLL_CODEC_GET();
#if 1
    pllCfg1.b.pll2N = (uint32_t)((int32_t)pllCfg1.b.pll2N + adj);
#else
    if (adj > 0)
    {
        if (pllCfg1.b.pll2N == 128)
        {
            pllCfg1.b.pll2N = 109;
            pllCfg1.b.pll2M = 17;
            DBG_PRINT("pll adj = 1\n");
        }
        else
        {
            pllCfg1.b.pll2N = 128;
            pllCfg1.b.pll2M = 20;
            DBG_PRINT("pll adj = 0\n");
        }
    }
    else if (adj < 0)
    {
        if (pllCfg1.b.pll2N == 128)
        {
            pllCfg1.b.pll2N = 147;
            pllCfg1.b.pll2M = 23;
            DBG_PRINT("pll adj = -1\n");
        }
        else
        {
            pllCfg1.b.pll2N = 128;
            pllCfg1.b.pll2M = 20;
            DBG_PRINT("pll adj = 0\n");
        }
    }
#endif
    HAL_SYS_REG_UNLOCK();
    HAL_SYS_PLL_CODEC_SET(pllCfg1.w);
    HAL_SYS_REG_LOCK();
    return;
}

/**
 * @brief  config the i2s clk source, if i2s is selected from pll2 out, pll2 clk will be configed, and enable pll2, otherwise, pll2 will be powerdown
 * @param  i2sClkSrc: i2s clk source select[0,1], 0 is from pll2 out, 1 is from SystemClk/4
 * @param  pll2ClkSrc: pll2 clk source select[0,1], 0, is from pll0 out, 1 is from pll1 out
 * @param  pll2ClkMode: pll2 clk mode select[0,1], 0, pll2 out is 45.1584M (for 44.1K sample rate), 1, pll2 out is 49.152M (for 48k sample rate)
 * @return NONE
 */
void HalI2sClkSelect(uint8_t i2sClkSrc, uint8_t pll2ClkSrc, uint8_t pll2ClkMode)
{
    SYS_CLK_TypeDef clkCfg;
    clkCfg.w = HAL_SYS_CLK_GET();
    if (i2sClkSrc == 0)
    {
        /* config pll1 and pll2 for codec */
        SYS_PLL_CFG1_TypeDef    pllCfg1;
        pllCfg1.w = i2sPll2CfgTb[pll2ClkMode].w;
        /* i2s from pll2 out */
        clkCfg.b.clkI2SSel = 0;
        if (pll2ClkSrc == 0)
        {
            /* pll2 from pll0 out */
            clkCfg.b.clkPll2Sel = 0;
            /* power down pll1 */
            pllCfg1.b.pll1En = 0;
        }
        else
        {
            /* pll2 from pll1 out */
            clkCfg.b.clkPll2Sel = 1;
        }
        HAL_SYS_REG_UNLOCK();
        HAL_SYS_PLL_CODEC_SET(pllCfg1.w);
        HAL_SYS_CLK_SET(clkCfg.w);
        HAL_SYS_REG_LOCK();
    }
    else
    {
        /* i2s from SystemClk/4 */
        clkCfg.b.clkI2SSel = 1;
        HAL_SYS_REG_UNLOCK();
        HAL_SYS_CLK_SET(clkCfg.w);
        /* power down pll1 and pll2 */
        SYS_PLL_CFG1_TypeDef    pllCfg1;
        pllCfg1.w = HAL_SYS_PLL_CODEC_GET();
        pllCfg1.b.pll1En = 0;
        pllCfg1.b.pll2En = 0;
        HAL_SYS_PLL_CODEC_SET(pllCfg1.w);
        HAL_SYS_REG_LOCK();
    }
    return;
}
/**
 * @}
 */

#endif
/**
 * @}
 */
/**
 * @}
 */
