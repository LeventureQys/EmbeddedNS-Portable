/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_timer.c
 * @brief   TIMER HAL module.
 * @version V1.0.3
 * @date    2022/04/07
 * @author  huangzheng
 ******************************************************************************
 */

/**
 * 版本日志：
 * V1.0.3, 2022/04/07, by huangzheng
 *      1. 函数命名按照大驼峰方式命名，头文件增加原函数名的define，为了兼容原代码，后续希望渐渐过渡到统一风格书写规范
 *      2. 整理头文件中依赖关系，使用jat_typedef.h作为数据类型的头文件，并把include放到extern "C" 之外
 *      3. 将一些配置选项移到a0_hal_conf.h，统一进行配置
 * V1.0.2, 2021/10/11,
 *      1. 头文件中的注释部分调整，适应Doxygen
 * V1.0.1, 2021/05/19, by huangzheng
 *      1. 头文件中添加寄存器操作宏及注释
 *      2. 增加接口函数
 * V1.0.0,
 *      1. 初版
 */
/* Includes -----------------------------------------------------------------*/
#include "a0_hal_conf.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */

#ifdef HAL_TIMER_MODULE_ENABLED
#include "a0_hal_timer.h"
#include "a0_hal_def.h"
/* Private typedef ----------------------------------------------------------*/
typedef struct {
    FUNCP       funcTimerIrq[4];
    uint32_t    timerState[4];
} HAL_TIM_VarTypeDef;
/* Private define -----------------------------------------------------------*/
static void HalTimerNullCallback(void);
/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
static HAL_TIM_VarTypeDef  halTimerVar = {
    .funcTimerIrq = {
        HalTimerNullCallback,
        HalTimerNullCallback,
        HalTimerNullCallback,
        HalTimerNullCallback,
    },
};
/* Private function prototypes ----------------------------------------------*/
static void HalTimerNullCallback(void)
{
    return;
}

/* Exported functions -------------------------------------------------------*/
/**
 * @addtogroup TIMER
 * @{
 */
/**
 * @defgroup TIMER_Exported_Functions TIMER Exported Functions
 * @{
 */
__RAM_CODE void HalTimerIrqHandler(uint32_t timerIdx)
{
    halTimerVar.funcTimerIrq[timerIdx]();
    return;
}

//return    >= 0,   state value, 0, timer idle, 1, timer in use
//          -1,     timerIdx out of range
int32_t HalTimerStateGet(uint32_t timerIdx)
{
    if (timerIdx < 4)
    {
        return (halTimerVar.timerState[timerIdx]);
    }
    else
    {
        return -1;
    }
}

//time max 44s at 96M pclk
//return    0,  ok
//          -1, timerIdx out of range
//          -2, cb is null
int32_t HalTimerInit(TIM_InitTypeDef *pInit)
{
    if (pInit->timerIdx < 4)
    {
        TIM_CR_TypeDef ctrlReg = {
            .b = {
                .intEn = 1,
                .enable = 1,
            },
        };
        TIM_TypeDef *TIMx = TIMN(pInit->timerIdx);
        IRQn_Type TIMx_IRQn = (IRQn_Type)(TIMER0_IRQn + pInit->timerIdx);
        HAL_TIM_DISABLE(TIMx);
        HAL_TIM_INT_CLR(TIMx);
        NVIC_DisableIRQ(TIMx_IRQn);
        if (pInit->cb != NULL)
        {
            halTimerVar.funcTimerIrq[pInit->timerIdx] = pInit->cb;
            halTimerVar.timerState[pInit->timerIdx] = 1;
            uint32_t reloadValue = (uint32_t)(((float_t)SystemPclk / 1000.0f) * pInit->time + 0.5f) - 2;    /* 和SysTick计数进行对比测试，发现需要-2才准，具体在芯片上主频上去之后再测试 */
            HAL_TIM_RELOAD_SET(TIMx, reloadValue);
            HAL_TIM_COUNT_SET(TIMx, reloadValue);
            NVIC_EnableIRQ(TIMx_IRQn);
            HAL_TIM_CTRL_SET(TIMx, ctrlReg.w);
            return 0;
        }
        else
        {
            return -2;
        }
    }
    else
    {
        return -1;
    }
}

//return    0,  ok
//          -1, timerIdx out of range
int32_t HalTimerDeinit(uint32_t timerIdx)
{
    if (timerIdx < 4)
    {
        TIM_TypeDef *TIMx = TIMN(timerIdx);
        IRQn_Type TIMx_IRQn = (IRQn_Type)(TIMER0_IRQn + timerIdx);
        NVIC_DisableIRQ(TIMx_IRQn);
        HAL_TIM_DISABLE(TIMx);
        halTimerVar.funcTimerIrq[timerIdx] = HalTimerNullCallback;
        halTimerVar.timerState[timerIdx] = 0;
        return 0;
    }
    else
    {
        return -1;
    }
}

/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup DUAL_TIMER
 * @{
 */
/**
 * @defgroup DUAL_TIMER_Exported_Functions DUAL TIMER Exported Functions
 * @{
 */

/**
 * @}
 */
/**
 * @}
 */

#endif

/**
 * @}
 */
