/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_pwm.c
 * @brief   PWM HAL module.
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
 *      1. 头文件中的注释部分调整，适应Doxygen
 * V1.0.1, 2021/04/28,
 *      1. 头文件中添加寄存器操作宏
 *      2. 将原来APP部分代码整合到HAL层，方便用户使用PWM各模式进行配置
 * V1.0.0, 2021/02/19,
 *      1. 初版，PWM控制器HAL层函数接口，仅初始化写入CR/ICR寄存器，中断处理，不配置IO复用，需要上层应用控制IO复用
 */
/* Includes -----------------------------------------------------------------*/
#include "a0_hal_conf.h"
/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup PWM
 * @{
 */

#ifdef HAL_PWM_MODULE_ENABLED
#include "a0_hal_pwm.h"
#include "a0_hal_def.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Private function prototypes ----------------------------------------------*/
/* Exported functions -------------------------------------------------------*/
/**
 * @defgroup PWM_Exported_Functions PWM Exported Functions
 * @{
 */
/**
 * @brief  pwm channel base frequency config
 * @param  baseFreq: frequency in Hz
 * @param  pCfgVal: pointer of value (preDiv-1) and (cntValue-1)
 * @retval == 0, config value OK
 * @retval != 0, config value is out of range
 */
static int32_t hal_pwm_config_base_freq(uint32_t baseFreq, uint32_t *pCfgVal)
{
    uint32_t preDiv = 1;
    uint32_t cntValue, t;
    /* find a suitable value of preDiv */
    do {
        preDiv++;
        t = baseFreq * preDiv;
        cntValue = ((SystemPclk + (t >> 1)) / t);
    } while ((cntValue > 0x10000) || (cntValue < 2));
    if (preDiv <= 0x10000)
    {
        *pCfgVal++ = (preDiv - 1);
        *pCfgVal++ = (cntValue - 1);
        return 0;
    }
    else
    {
        return -1;
    }
}

/**
 * @brief  config the pwm channel pwm mode of PWM_MODE_STD, mode @ref PWM_mode
 * @param  chIdx: specifies the pwm channel index number
 * @param  pPara: pointer to the parameter of PWM mode 0, @ref PWM_StdModeTypeDef
 * @retval == 0, config OK
 * @retval == -1, config error, base frequency is out of range
 */
int32_t HalPwmConfigMode0(uint8_t chIdx, void *pPara)
{
    PWM_StdModeTypeDef *pModePara = (PWM_StdModeTypeDef *)pPara;
    uint32_t baseFreqCfgVal[2];
    if (hal_pwm_config_base_freq(pModePara->baseFreq, &baseFreqCfgVal[0]) == 0)
    {
        uint32_t cnt2High = 0;
        uint32_t cnt2Low = (uint32_t)(pModePara->dutyRatio * (float_t)baseFreqCfgVal[1]);
        PWM_CCR_TypeDef pwmCCR = {
            .preDiv = baseFreqCfgVal[0],
            .cntValue = baseFreqCfgVal[1],
            .cnt2High = cnt2High,
            .cnt2Low = cnt2Low,
            .mode = 0,
        };
        memcpy((void *)PWMCH(chIdx), (void *)&pwmCCR, sizeof(pwmCCR));
        return 0;                                                           /* ok */
    }
    else
    {
        return -1;                                                          /* error */
    }
}

/**
 * @brief  config the pwm channel pwm mode of PWM_MODE_BREATHE_AUTO, mode @ref PWM_mode
 * @param  chIdx: specifies the pwm channel index number
 * @param  pPara: pointer to the parameter of PWM mode 1, @ref PWM_BreModeTypeDef
 * @retval == 0, config OK
 * @retval == -1, config error, base frequency is out of range
 * @retval == -2, config error, fade in time and fade out time is out of range
 */
int32_t HalPwmConfigMode1(uint8_t chIdx, void *pPara)
{
    PWM_BreModeTypeDef *pModePara = (PWM_BreModeTypeDef *)pPara;
    uint32_t baseFreqCfgVal[2];
    if (hal_pwm_config_base_freq(pModePara->baseFreq, &baseFreqCfgVal[0]) == 0)
    {
        uint32_t cnt2High = 0;
        uint32_t stepStart = (uint32_t)(pModePara->dutyRatioMin * (float_t)baseFreqCfgVal[1]);
        uint32_t stepEnd = (uint32_t)(pModePara->dutyRatioMax * (float_t)baseFreqCfgVal[1]);
        uint32_t changeCnt = 1;
        uint32_t t1, t2;
        int32_t step1, step2;
        t1 = pModePara->breatheFadeInTime * pModePara->baseFreq * (baseFreqCfgVal[0] + 1);
        t2 = pModePara->breatheFadeOutTime * pModePara->baseFreq * (baseFreqCfgVal[0] + 1);
        do {
            step1 = ((stepEnd - stepStart) * changeCnt * 1000 + (t1 >> 1)) / t1;
            step2 = ((stepEnd - stepStart) * changeCnt * 1000 + (t2 >> 1)) / t2;
            changeCnt++;
        } while ((step1 == 0) || (step2 == 0));
        if ((changeCnt > 257) || (step1 > 32768) || (step2 > 32768))
        {
            return -2;                                                      /* error */
        }

        PWM_CCR_TypeDef pwmCCR = {
            .preDiv = baseFreqCfgVal[0],
            .cntValue = baseFreqCfgVal[1],
            .cnt2High = cnt2High,
            .cnt2Low = stepEnd,
            .stepStart = stepStart,
            .stepEnd = stepEnd,
            .changeCnt = (changeCnt - 1),
            .mode = 1,
        };
        memcpy((void *)PWMCH(chIdx), (void *)&pwmCCR, sizeof(pwmCCR));
        HAL_PWM_2STEP_SET(chIdx, ((uint32_t)(-step2)), ((uint32_t)step1));
        return 0;                                                           /* ok */
    }
    else
    {
        return -1;                                                          /* error */
    }
}

/**
 * @brief  config the pwm channel pwm mode of PWM_MODE_BREATHE_ONCE, mode @ref PWM_mode, if configed to this mode, pwm irq handler should be added to adjust step for next pwm output
 * @param  chIdx: specifies the pwm channel index number
 * @param  pPara: pointer to the parameter of PWM mode 2, @ref PWM_BreModeTypeDef
 * @retval == 0, config OK
 * @retval == -1, config error, base frequency is out of range
 * @retval == -2, config error, fade time is out of range
 */
int32_t HalPwmConfigMode2(uint8_t chIdx, void *pPara)
{
    PWM_BreModeTypeDef *pModePara = (PWM_BreModeTypeDef *)pPara;
    uint32_t baseFreqCfgVal[2];
    if (hal_pwm_config_base_freq(pModePara->baseFreq, &baseFreqCfgVal[0]) == 0)
    {
        uint32_t cnt2High = 0;
        uint32_t stepStart = (uint32_t)(pModePara->dutyRatioMin * (float_t)baseFreqCfgVal[1]);
        uint32_t stepEnd = (uint32_t)(pModePara->dutyRatioMax * (float_t)baseFreqCfgVal[1]);

        uint32_t changeCnt = 1;
        uint32_t t;
        int32_t step;
        t = pModePara->breatheFadeInTime * pModePara->baseFreq * (baseFreqCfgVal[0] + 1);
        do {
            step = ((stepEnd - stepStart) * changeCnt * 1000 + (t >> 1)) / t;
            changeCnt++;
        } while (step == 0);
        if ((changeCnt > 257) || (step > 32768))
        {
            return -2;                                                      /* error */
        }

        PWM_CCR_TypeDef pwmCCR = {
            .preDiv = baseFreqCfgVal[0],
            .cntValue = baseFreqCfgVal[1],
            .cnt2High = cnt2High,
            .cnt2Low = stepEnd,
            .stepStart = stepStart,
            .stepEnd = stepEnd,
            .changeCnt = (changeCnt - 1),
            .mode = 2,
        };
        memcpy((void *)PWMCH(chIdx), (void *)&pwmCCR, sizeof(pwmCCR));
        HAL_PWM_1STEP_SET(chIdx, ((uint32_t)step));
        return 0;                                                           /* ok */
    }
    else
    {
        return -1;                                                          /* error */
    }
}

/**
 * @brief  config the pwm channel pwm mode of PWM_MODE_CNT_RISE_EDGE and PWM_MODE_CNT_FALL_EDGE, mode @ref PWM_mode, if configed to this mode, pwm irq handler should be added to record the count value
 * @param  chIdx: specifies the pwm channel index number
 * @param  pPara: pointer to the parameter of PWM mode 3 and 4, @ref PWM_CntModeTypeDef
 * @retval == 0, config OK
 * @retval == -1, config error, count pulse width is out of range
 */
int32_t HalPwmConfigMode34(uint8_t chIdx, void *pPara)
{
    PWM_CntModeTypeDef *pModePara = (PWM_CntModeTypeDef *)pPara;
    uint32_t preDiv = 2;
    float_t t =  (float_t)65535 * (float_t)1000000 / (float_t)SystemPclk;

    while (pModePara->pulseWidthMax > (uint32_t)(t * (float_t)preDiv))
    {
        preDiv++;
    }
    if (preDiv <= 0x10000)
    {
        PWM_CCR_TypeDef pwmCCR = {
            .preDiv = (preDiv - 1),
            .cntValue = 0xFFFF,
            .mode = (pModePara->modeSel34 == 0) ? 3 : 4,
        };
        memcpy((void *)PWMCH(chIdx), (void *)&pwmCCR, sizeof(pwmCCR));
        return 0;
    }
    else
    {
        return -1;
    }
}

void HalPwmIrqHandler(PWM_InitTypeDef *pInit)
{
    PWM_HandleTypeDef   *pHandle = pInit->pHandle;
    for (uint8_t i = 0; i < pInit->handleNumber; i++)
    {
        if (HAL_PWM_INT_CHECK(pHandle->chIdx) != 0)
        {
            if (pHandle->pFunCb != NULL)
            {
                pHandle->pFunCb((void *)pHandle);
            }
            HAL_PWM_INT_CLR(pHandle->chIdx);
        }
        pHandle++;
    }
    return;
}

void HalPwmInit(PWM_InitTypeDef *pInit)
{
    *PWM = pInit->baseCfg;
    NVIC_EnableIRQ(PWM_IRQn);
    return;
}

#endif
/**
 * @}
 */
/**
 * @}
 */
