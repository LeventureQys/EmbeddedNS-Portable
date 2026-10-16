/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_timer.h
 * @brief   Header file of TIMER and DUAL-TIMER HAL module.
 * @version V1.0.3
 * @date    2022/04/07
 * @author  huangzheng
 ******************************************************************************
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __A0_HAL_TIMER_H
#define __A0_HAL_TIMER_H

/* Includes -----------------------------------------------------------------*/
#include "jat_typedef.h"
#include "a0.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/**
 * @addtogroup TIMER
 * @brief 4 simple Timer module
 * @{
 */
/**
 * @defgroup TIMER_Exported_Types TIMER Exported Types
 * @{
 */
/**
 * @brief  TIMER Configuration Structure definition
 */
typedef struct
{
    FUNCP               cb;                 /*!< callback function for timer    */
    float_t             time;               /*!< interrupt time in ms           */
    uint32_t            timerIdx;           /*!< timer index numebr, 0~3        */
}TIM_InitTypeDef;
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup DUAL_TIMER
 * @brief 1 dual-timer module
 * @{
 */
/**
 * @defgroup DUAL_TIMER_Exported_Types DUAL TIMER Exported Types
 * @{
 */
/**
 * @}
 */
/**
 * @}
 */

/* Exported constants -------------------------------------------------------*/
/* Exported macros ----------------------------------------------------------*/
/**
 * @addtogroup TIMER
 * @{
 */
/**
 * @defgroup TIMER_Exported_Macros TIMER Exported Macros
 * @{
 */
/**
 * @brief  Timer ctrl register value set
 * @param  TIM     Timer instance, TIM0/TIM1/TIM2/TIM3, @ref Peripheral_Declaration
 * @param  CTRL    Value write to ctrl register, @ref TIM_CR_TypeDef
 * @return None
 */
#define HAL_TIM_CTRL_SET(TIM, CTRL)                                     ((TIM)->CR = (CTRL))

/**
 * @brief  Timer ctrl register value get
 * @param  TIM     Timer instance, TIM0/TIM1/TIM2/TIM3, @ref Peripheral_Declaration
 * @return Value read from ctrl register, @ref TIM_CR_TypeDef
 */
#define HAL_TIM_CTRL_GET(TIM)                                           ((TIM)->CR)

/**
 * @brief  Timer count register value set
 * @param  TIM     Timer instance, TIM0/TIM1/TIM2/TIM3, @ref Peripheral_Declaration
 * @param  CNT     Value write to count register
 * @return None
 */
#define HAL_TIM_COUNT_SET(TIM, CNT)                                     ((TIM)->CNTR = (CNT))

/**
 * @brief  Timer count register value clear to zero
 * @param  TIM     Timer instance, TIM0/TIM1/TIM2/TIM3, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_TIM_COUNT_CLR(TIM)                                          ((TIM)->CNTR = 0)

/**
 * @brief  Timer count register value get
 * @param  TIM     Timer instance, TIM0/TIM1/TIM2/TIM3, @ref Peripheral_Declaration
 * @return Value read from count register
 */
#define HAL_TIM_COUNT_GET(TIM)                                          ((TIM)->CNTR)

/**
 * @brief  Timer reload register value set
 * @param  TIM     Timer instance, TIM0/TIM1/TIM2/TIM3, @ref Peripheral_Declaration
 * @param  RLD     Value write to reload register
 * @return None
 */
#define HAL_TIM_RELOAD_SET(TIM, RLD)                                    ((TIM)->RLDR = (RLD))

/**
 * @brief  Timer reload register value get
 * @param  TIM     Timer instance, TIM0/TIM1/TIM2/TIM3, @ref Peripheral_Declaration
 * @return Value read from reload register
 */
#define HAL_TIM_RELOAD_GET(TIM)                                         ((TIM)->RLDR)

/**
 * @brief  Timer interrupt clear
 * @param  TIM     Timer instance, TIM0/TIM1/TIM2/TIM3, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_TIM_INT_CLR(TIM)                                            ((TIM)->ISR = BIT0)

/**
 * @brief  Timer enable to count down
 * @param  TIM: Timer instance, TIM0/TIM1/TIM2/TIM3, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_TIM_ENABLE(TIM)                                             ((TIM)->CR |= ( BIT0))

/**
 * @brief  Timer disable to count down
 * @param  TIM     Timer instance, TIM0/TIM1/TIM2/TIM3, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_TIM_DISABLE(TIM)                                            ((TIM)->CR &= (~BIT0))

/**
 * @brief  Timer interrupt enable
 * @param  TIM     Timer instance, TIM0/TIM1/TIM2/TIM3, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_TIM_INT_ENABLE(TIM)                                         ((TIM)->CR |= ( BIT3))

/**
 * @brief  Timer interrupt disable
 * @param  TIM     Timer instance, TIM0/TIM1/TIM2/TIM3, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_TIM_INT_DISABLE(TIM)                                        ((TIM)->CR &= (~BIT3))

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
 * @defgroup DUAL_TIMER_Exported_Macros DUAL TIMER Exported Macros
 * @{
 */

/**
 * @brief  Dual-Timer load register value set
 * @param  DTIM    Dual-Timer instance, DTIM0/DTIM1, @ref Peripheral_Declaration
 * @param  LD      Value write to load register
 * @return None
 */
#define HAL_DTIM_LOAD_SET(DTIM, LD)                                     ((DTIM)->LR = (LD))

/**
 * @brief  Dual-Timer load register value get
 * @param  DTIM    Dual-Timer instance, DTIM0/DTIM1, @ref Peripheral_Declaration
 * @return Value read from load register
 */
#define HAL_DTIM_LOAD_GET(DTIM)                                         ((DTIM)->LR)

/**
 * @brief  Dual-Timer count register value get
 * @param  DTIM    Dual-Timer instance, DTIM0/DTIM1, @ref Peripheral_Declaration
 * @return Value read from count register
 */
#define HAL_DTIM_COUNT_GET(DTIM)                                        ((DTIM)->CNTR)

/**
 * @brief  Dual-Timer ctrl register value set
 * @param  DTIM    Dual-Timer instance, DTIM0/DTIM1, @ref Peripheral_Declaration
 * @param  CTRL    Value write to ctrl register
 * @return None
 */
#define HAL_DTIM_CTRL_SET(DTIM, CTRL)                                   ((DTIM)->CR = (CTRL))

/**
 * @brief  Dual-Timer ctrl register value get
 * @param  DTIM    Dual-Timer instance, DTIM0/DTIM1, @ref Peripheral_Declaration
 * @return Value read from ctrl register
 */
#define HAL_DTIM_CTRL_GET(DTIM)                                         ((DTIM)->CR)

/**
 * @brief  Dual-Timer interrupt clear
 * @param  DTIM    Dual-Timer instance, DTIM0/DTIM1, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_DTIM_INT_CLR(DTIM)                                          ((DTIM)->ICR = 0)

/**
 * @brief  Dual-Timer interrupt check
 * @param  DTIM    Dual-Timer instance, DTIM0/DTIM1, @ref Peripheral_Declaration
 * @retval != 0    interrupt occured
 * @retval == 0    no interrupt occured
 */
#define HAL_DTIM_INT_CHECK(DTIM)                                        ((DTIM)->ISR)

/**
 * @brief  Dual-Timer background load register value set
 * @param  DTIM    Dual-Timer instance, DTIM0/DTIM1, @ref Peripheral_Declaration
 * @param  BGLD    Value write to background load register
 * @return None
 */
#define HAL_DTIM_BG_LOAD_SET(DTIM, BGLD)                                ((DTIM)->BLR = (BGLD))

/**
 * @brief  Dual-Timer background load register value get
 * @param  DTIM    Dual-Timer instance, DTIM0/DTIM1, @ref Peripheral_Declaration
 * @return Value read from background load register
 */
#define HAL_DTIM_BG_LOAD_GET(DTIM)                                      ((DTIM)->BLR)
/**
 * @}
 */
/**
 * @}
 */

/* Exported functions -------------------------------------------------------*/
/**
 * @addtogroup TIMER
 * @{
 */
/**
 * @addtogroup TIMER_Exported_Functions TIMER Exported Functions
 * @{
 */

/**
 * @brief  timer irq handler function
 * @param  timerIdx    0~3, index number of timer
 * @return None
 */
void HalTimerIrqHandler(uint32_t timerIdx);
/**
 * @brief  timer module initial process function
 * @param  pInit    pointer to structure of @ref TIM_InitTypeDef, time max is 44 seconds at 96MHz PCLK
 * @retval == 0    initial ok
 * @retval == -1   initial error, timerIdx is out of range, it must be 0~3
 * @retval == -2   initial error, callback function of input parameter is null
 */
int32_t HalTimerInit(TIM_InitTypeDef *pInit);
/**
 * @brief  timer module de-initial process function
 * @param  timerIdx    0~3, index number of timer
 * @retval == 0    de-initial ok
 * @retval == -1   de-initial error, timerIdx is out of range, it must be 0~3
 */
int32_t HalTimerDeinit(uint32_t timerIdx);
/**
 * @brief  timer module state get
 * @param  timerIdx    0~3, index number of timer
 * @retval == 0    state in idle, timer of the input tiemrIdx is idle, can be initialled to use
 * @retval == 1    state in use, timer of the input timerIdx is in use, it can not be initialled to use, but can be de-initialled to release
 * @retval == -1,  error, timerIdx is out of range, it must be 0~3
 */
int32_t HalTimerStateGet(uint32_t timerIdx);

/* for code compatible */
#define hal_timer_irq_handler                   HalTimerIrqHandler
#define hal_timer_init                          HalTimerInit
#define hal_timer_deinit                        HalTimerDeinit
#define hal_timer_state_get                     HalTimerStateGet

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
 * @addtogroup DUAL_TIMER_Exported_Functions DUAL TIMER Exported Functions
 * @{
 */
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
