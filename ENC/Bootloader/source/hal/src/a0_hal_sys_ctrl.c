/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_sys_ctrl.c
 * @brief   SYS_CTRL HAL module. Include external spi controller, external sram
 *          and sdram interface, pll, clk, codec, efuse, usb phy, wdt, etc.
 * @version V1.0.5
 * @date    2023/12/01
 * @author  huangzheng
 ******************************************************************************
 */

/**
 * 版本日志：
 * V1.0.4, 2022/07/13,
 *      1. 系统锁定寄存器涉及寄存器的说明更新
 *      2. 增加USB的VBUS禁止接口，用于软件控制设备断开和主机的连接
 * V1.0.3, 2022/04/07,
 *      1. 按照最终版本芯片对应修改
 *      2. 整理头文件中依赖关系，使用jat_typedef.h作为数据类型的头文件，并把include放到extern "C" 之外。
 *      3. 函数命名按照大驼峰方式命名，头文件增加原函数名的define，为了兼容原代码，后续希望渐渐过渡到统一风格书写规范
 *      4. 将一些配置选项移到a0_hal_conf.h，统一进行配置
 * V1.0.2, 2021/10/20,
 *      1. 头文件中增加pll0配置寄存器的读写接口
 * V1.0.1, 2021/10/11,
 *      1. 头文件中的注释部分调整，使用Doxygen
 *      2. 添加其他一些系统控制寄存器的操作宏
 * V1.0.0, 2021/05/14,
 *      1. 头文件中相关寄存器操作的宏
 *      2. 增加一些实际应用会用到的HAL接口函数
 */

/* Includes ------------------------------------------------------------------*/
#include "a0_hal_conf.h"
/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup SYS_CTRL
 * @{
 */
#include "a0_hal_sys_ctrl.h"
#include "a0_hal_def.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @defgroup Sys_Ctrl_Exported_Functions Sys Ctrl Exported Functions
 * @{
 */

#ifdef HAL_SYS_WDT_SOFT_RST_EN
void HalSysWdtSoftwareRst(void)
{
    /* wdt interrupt to excute a software reset */
    if (HAL_SYS_WDT_MIS_GET() != 0)
    {
        HAL_SYS_WDT_UNLOCK();
        HAL_SYS_WDT_INT_CLR();
        NVIC_SystemReset();
    }
    return;
}
#else
void HalSysWdtSoftwareRst(void)
{
    return;
}
#endif

/*
 * WatchDog use pclk to count, so input parameter cycle = SystemPclk * time (in seconds)
 * e.g. config watchdog count in 500ms, cycle = SystemPclk * 500 / 1000;
 * Default config watchdog as hardware reset, NMI_Handler is not necessary
 */
void HalSysWdtInit(uint32_t cycle)
{
    /* unlock wtd reg */
    HAL_SYS_WDT_UNLOCK();
    /* config watchdog count value */
    HAL_SYS_WDT_LOAD_SET(cycle);
#if (HAL_SYS_WDT_SOFT_RST != 0)
    /* wtd irq enable (nmi irq, soft rst by user, add code to function NMI_Handler ) */
    HAL_SYS_WDT_INT_EN();
#else
    /* wtd rst enable (rst by hard, function NMI_Handler is not necessary ) */
    HAL_SYS_WDT_RST_EN();
#endif
    /* lock wtd reg */
    HAL_SYS_WDT_LOCK();
    return;
}

void HalSysWdtReload(uint32_t cycle)
{
    /* unlock wtd reg */
    HAL_SYS_WDT_UNLOCK();
    /* config watchdog count value */
    HAL_SYS_WDT_LOAD_SET(cycle);
    /* lock wtd reg */
    HAL_SYS_WDT_LOCK();
    return;
}

/**
 * @brief  record system cycle by DWT->CYCNT, need to use 5 words memory
 * @param  pRecordMem pointer to 5 words memory
 * @return None
 * @details 
 * memory[5] = {
 *      current start cycle, 
 *      cycle between 2 HalSysCycleRecordStart called, 
 *      cycle between 2 HalSysCycleRecordStart called max recorded, 
 *      cycle between HalSysCycleRecordStart and HalSysCycleRecordEnd called, 
 *      cycle between HalSysCycleRecordStart and HalSysCycleRecordEnd called max recorded }
 */
__RAM_CODE void HalSysCycleRecordStart(uint32_t *pRecordMem)
{
    uint32_t dTimCnt = DWT->CYCCNT;
    pRecordMem[1] = dTimCnt - pRecordMem[0];
    pRecordMem[0] = dTimCnt;
    if (pRecordMem[1] > pRecordMem[2])
    {
        pRecordMem[2] = pRecordMem[1];
    }
    return;
}

__RAM_CODE void HalSysCycleRecordEnd(uint32_t *pRecordMem)
{
    pRecordMem[3] = DWT->CYCCNT - pRecordMem[0];
    if (pRecordMem[3] > pRecordMem[4])
    {
        pRecordMem[4] = pRecordMem[3];
    }
    return;
}

#ifdef EFUSE_WRITE_EN
/* 该函数不使用优化，供测试使用，如需烧写eFuse，使用离线烧写工具进行烧写 */
__RAM_CODE uint32_t HalSysEfuseWrite(uint32_t efuse0, uint32_t efuse1)
{
    uint32_t eFuseResult = (HAL_SYS_EFUSE_RESULT0_RD() && HAL_SYS_EFUSE_RESULT1_RD());
    efuse1 &= 0x00FFFFFF;
    if (eFuseResult != EFUSE_CHECK_ERROR)
    {
        uint32_t delay;
        HAL_SYS_EFUSE_PROGRAM_BIT_STOP();
        for (uint8_t i = 0; i < 32; i++)
        {
            if (efuse0 & (1 << i))
            {
                delay = 5;
                HAL_SYS_EFUSE_PROGRAM_BIT_SET(i);
                HAL_SYS_EFUSE_PROGRAM_BIT_START();
                //HAL_GPIO_BIT_SET(GPIO0, BIT0);
                while(delay--);
                HAL_SYS_EFUSE_PROGRAM_BIT_STOP();
                //HAL_GPIO_BIT_CLR(GPIO0, BIT0);
            }
        }
        for (uint8_t i = 32; i < 56; i++)
        {
            if (efuse1 & (1 << (i & 0x1F)))
            {
                delay = 5;
                HAL_SYS_EFUSE_PROGRAM_BIT_SET(i);
                HAL_SYS_EFUSE_PROGRAM_BIT_START();
                while(delay--);
                HAL_SYS_EFUSE_PROGRAM_BIT_STOP();
            }
        }
    }
    return eFuseResult;
}
#endif

/**
 * @}
 */

/**
 * @}
 */
/**
 * @}
 */
