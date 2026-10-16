/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_dma.c
 * @brief   DMA HAL module.
 * @version V1.0.3
 * @date    2022/04/07
 * @author  huangzheng
 ******************************************************************************
 */

/**
 * 版本日志：
 * V1.0.3, 2022/04/07
 *      1. 函数命名按照大驼峰方式命名，头文件增加原函数名的define，为了兼容原代码，后续希望渐渐过渡到统一风格书写规范
 *      2. 整理头文件中依赖关系，使用jat_typedef.h作为数据类型的头文件，并把include放到extern "C" 之外
 *      3. 将一些配置选项移到a0_hal_conf.h，统一进行配置
 * V1.0.2, 2021/10/11,
 *      1. 修改IRQ响应函数
 *      2. 新增TC和EA的回调函数(__WEAK)，用户可在drv或app层重写该函数
 *      3. 整理.h文件中的宏及注释说明，适配Doxygen
 * V1.0.1, 2021/05/27,
 *      1. 添加通用DMA初始化函数，以及DMA中断响应等函数接口
 *      2. 头文件中寄存器操作宏增加，注释补全
 * V1.0.0, 2021/04/30,
 *      1. 头文件包含寄存器操作宏
 */
/* Includes ------------------------------------------------------------------*/
#include "a0_hal_conf.h"
/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup DMA
 * @{
 */

#ifdef HAL_DMA_MODULE_ENABLED
#include "a0_hal_dma.h"
#include "a0_hal_def.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @defgroup DMA_Exported_Functions DMA Exported Functions
 * @{
 */
__RAM_CODE __WEAK void HalDmaTcCallback(DMA_CTRL_TypeDef *DMACtrl, uint32_t tcIsr)
{
    return;
}

__RAM_CODE __WEAK void HalDmaEaCallback(DMA_CTRL_TypeDef *DMACtrl, uint32_t eaIsr)
{
    /* dma error or abort, halted */
    __breakpoint(0);
    return;
}

__RAM_CODE void HalDmaIrqHandler(DMA_CTRL_TypeDef *DMACtrl)
{
    if (HAL_DMA_GROUP_INT_GET(DMACtrl) != 0)
    {
        uint32_t tcIsr = HAL_DMA_GROUP_TC_INT_GET(DMACtrl);
        if (tcIsr != 0)
        {
            HAL_DMA_GROUP_TC_INT_CLR(DMACtrl, tcIsr);
            HalDmaTcCallback(DMACtrl, tcIsr);
        }
        else
        {
            uint32_t eaIsr = HAL_DMA_GROUP_EA_INT_GET(DMACtrl);
            if (eaIsr != 0)
            {
                HAL_DMA_GROUP_EA_INT_CLR(DMACtrl,eaIsr);
                HalDmaEaCallback(DMACtrl, eaIsr);
            }
        }
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
