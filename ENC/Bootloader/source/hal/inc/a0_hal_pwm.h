/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_pwm.h
 * @brief   Header file of PWM HAL module.
 * @version V1.0.3
 * @date    2022/04/07
 * @author  huangzheng
 ******************************************************************************
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __A0_HAL_PWM_H
#define __A0_HAL_PWM_H

/* Includes -----------------------------------------------------------------*/
#include "jat_typedef.h"
#include "a0.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup PWM
 * @brief 16 channels Pulse Width Modulation (PWM) module, it include several working @ref PWM_Mode
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/**
 * @defgroup PWM_Exported_Types PWM Exported Types
 * @{
 */
/**
 * @brief  PWM Standard Mode Configuration Structure definition
 */
typedef struct
{
    uint32_t    baseFreq;                               /* pwm base frequency (Hz) */
    float_t     dutyRatio;                              /* (high_level_cnt/cycle_cnt), range is 0.0f~1.0f */
}PWM_StdModeTypeDef;

/**
 * @brief  PWM Breathing Mode Configuration Structure definition
 */
typedef struct
{
    uint32_t    baseFreq;                               /* pwm base frequency (Hz) */
    uint32_t    breatheFadeInTime;                      /* light off change to light on time (ms), in breathe once mode, this is current fade time (ms) */
    uint32_t    breatheFadeOutTime;                     /* light on change to light off time (ms) */
    float_t     dutyRatioMin;                           /* min value of (high_level_cnt/cycle_cnt), range is 0.0f~1.0f */
    float_t     dutyRatioMax;                           /* max value of (high_level_cnt/cycle_cnt), range is 0.0f~1.0f */
}PWM_BreModeTypeDef;

/**
 * @brief  PWM Count Mode Configuration Structure definition
 */
typedef struct
{
    uint32_t    pulseWidthMin;                          /* us, max/min < 65535 */
    uint32_t    pulseWidthMax;                          /* us, */
    uint32_t    modeSel34;                              /* ==0, mode3, !=0, mode4 */
}PWM_CntModeTypeDef;

/**
 * @brief  PWM Handler Structure definition
 */
typedef struct
{
    void                    *pHVar;                     /*!< handle variables */
    void                    (*pFunCb)(void *pHandle);   /*!< callback function for pwm irq     */
    uint32_t                chIdx;                      /*!< pwm channel index number, 0~15 */
}PWM_HandleTypeDef;

/**
 * @brief  PWM Init Configuration Structure definition
 */
typedef struct
{
    PWM_TypeDef             baseCfg;
    PWM_HandleTypeDef       *pHandle;
    uint32_t                handleNumber;
}PWM_InitTypeDef;
/**
 * @}
 */

/* Exported constants -------------------------------------------------------*/
/**
 * @defgroup PWM_Exported_Constants PWM Exported Constants
 * @{
 */
/**
 * @defgroup PWM_Mode PWM Mode
 * @brief PWM working mode config value, use for 'mode' in @ref PWM_CCR_TypeDef
 * @{
 */
#define PWM_MODE_STD                        (0x0U)  /*!< Standard PWM Output Mode */
#define PWM_MODE_BREATHE_AUTO               (0x1U)  /*!< Auto LED Breathing Lamp Mode */
#define PWM_MODE_BREATHE_ONCE               (0x2U)  /*!< LED Breathing Lamp Once, Occur IRQ */
#define PWM_MODE_CNT_RISE_EDGE              (0x3U)  /*!< Count Cycle Begin With Rise Edge */
#define PWM_MODE_CNT_FALL_EDGE              (0x4U)  /*!< Count Cycle Begin With Fall Edge */
/**
 * @}
 */
/**
 * @}
 */

/* Exported macros ----------------------------------------------------------*/
/**
 * @defgroup PWM_Exported_Macros PWM Exported Macros
 * @{
 */
/**
 * @brief  pwm channel restart, clr bit of (1 << CH) in register 'CR' and set bit of (1 << CH) in register 'CR', @ref PWM_TypeDef
 * @param  CH      pwm channel index number, 0~15
 * @return None
 */
#define HAL_PWM_RESTART(CH)                                             \
do {                                                                    \
    PWM->CR &= (~(1UL << (CH)));                                        \
    PWM->CR |= (1UL << (CH));                                           \
} while (0U)

/**
 * @brief  pwm channel interrupt flag check, read bit of (BIT16 << CH) in register 'ICR', @ref PWM_TypeDef
 * @param  CH      pwm channel index number, 0~15
 * @retval == 0    no interrupt
 * @retval != 0    interrupt occured
 */
#define HAL_PWM_INT_CHECK(CH)                                           (PWM->ICR & (0x00010000UL << (CH)))
/**
 * @brief  pwm channel interrupt flag clear, clr bit of (BIT16 << CH) in register 'ICR', @ref PWM_TypeDef
 * @param  CH      pwm channel index number, 0~15
 * @return None
 */
#define HAL_PWM_INT_CLR(CH)                                             (PWM->ICR &= (~(0x00010000UL << (CH))))
/**
 * @brief  pwm all channel interrupt flag get, read the high 16 bit in register 'ICR', @ref PWM_TypeDef
 * @return 16 bits value indicated the interrupt flag of 16 channels. e.g. 0x3000 means the interrupt of channel 12 and channel 13 is occured.
 */
#define HAL_PWM_INT_GET()                                               (PWM->ICR >> 16)
/**
 * @brief  pwm channel two steps set for auto breathing mode(1), write register 'CSR' in @ref PWM_CHR_TypeDef
 * @param  CH      pwm channel index number, 0~15
 * @param  STEP2   value of step2 wrtie to high 16bit of CSR
 * @param  STEP1   value of step1 write to low 16bit of CSR
 * @return None
 */
#define HAL_PWM_2STEP_SET(CH, STEP2, STEP1)                             (PWMCH(CH)->CSR = ((STEP2) << 16) | (STEP1))
/**
 * @brief  pwm channel one step set for once breathing mode(2), write register 'CSR' in @ref PWM_CHR_TypeDef
 * @param  CH      pwm channel index number, 0~15
 * @param  STEP    value of step wrtie to CSR
 * @return None
 */
#define HAL_PWM_1STEP_SET(CH, STEP)                                     (PWMCH(CH)->CSR = (STEP))
/**
 * @brief  pwm channel step value get, read register 'CSR' in @ref PWM_CHR_TypeDef
 * @param  CH      pwm channel index number, 0~15
 * @return current step value in CSR
 */
#define HAL_PWM_STEP_GET(CH)                                            (PWMCH(CH)->CSR)
/**
 * @brief  pwm channel count value get, read register 'CCVR' in @ref PWM_CHR_TypeDef
 * @param  CH      pwm channel index number, 0~15
 * @return current count value in CCVR, bit15~bit00 is fall edge cnt, bit31~bit16 is rise edge cnt
 */
#define HAL_PWM_CH_CNT_GET(CH)                                          (PWMCH(CH)->CCVR)
/**
 * @}
 */

/* Exported functions -------------------------------------------------------*/
/**
 * @addtogroup PWM_Exported_Functions PWM Exported Functions
 * @{
 */
/**
 * @brief  pwm module initial process function
 * @param  pInit    pointer to structure of @ref PWM_InitTypeDef
 * @return None
 */
void HalPwmInit(PWM_InitTypeDef *pInit);

/**
 * @brief  pwm module irq handler function
 * @param  pInit    pointer to structure of @ref PWM_InitTypeDef
 * @return None
 */
void HalPwmIrqHandler(PWM_InitTypeDef *pInit);

/**
 * @brief  config the pwm channel pwm mode of PWM_MODE_STD, mode @ref PWM_Mode
 * @param  chIdx   specifies the pwm channel index number
 * @param  pPara   pointer to the parameter of PWM mode 0, @ref PWM_StdModeTypeDef
 * @retval == 0    config OK
 * @retval == -1   config error, base frequency is out of range
 */
int32_t HalPwmConfigMode0(uint8_t chIdx, void *pPara);

/**
 * @brief  config the pwm channel pwm mode of PWM_MODE_BREATHE_AUTO, mode @ref PWM_Mode
 * @param  chIdx   specifies the pwm channel index number
 * @param  pPara   pointer to the parameter of PWM mode 1, @ref PWM_BreModeTypeDef
 * @retval == 0    config OK
 * @retval == -1   config error, base frequency is out of range
 * @retval == -2   config error, fade in time and fade out time is out of range
 */
int32_t HalPwmConfigMode1(uint8_t chIdx, void *pPara);

/**
 * @brief  config the pwm channel pwm mode of PWM_MODE_BREATHE_ONCE, mode @ref PWM_Mode, if configed to this mode, pwm irq handler should be added to adjust step for next pwm output
 * @param  chIdx   specifies the pwm channel index number
 * @param  pPara   pointer to the parameter of PWM mode 2, @ref PWM_BreModeTypeDef
 * @retval == 0    config OK
 * @retval == -1   onfig error, base frequency is out of range
 * @retval == -2   config error, fade time is out of range
 */
int32_t HalPwmConfigMode2(uint8_t chIdx, void *pPara);

/**
 * @brief  config the pwm channel pwm mode of PWM_MODE_CNT_RISE_EDGE and PWM_MODE_CNT_FALL_EDGE, mode @ref PWM_Mode, if configed to this mode, pwm irq handler should be added to record the count value
 * @param  chIdx   specifies the pwm channel index number
 * @param  pPara   pointer to the parameter of PWM mode 3 and 4, @ref PWM_CntModeTypeDef
 * @retval == 0    config OK
 * @retval == -1   config error, count pulse width is out of range
 */
int32_t HalPwmConfigMode34(uint8_t chIdx, void *pPara);

/**
 * @}
 */
/**
 * @}
 */
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
