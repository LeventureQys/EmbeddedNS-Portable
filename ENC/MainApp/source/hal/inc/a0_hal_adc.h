/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_adc.h
 * @brief   Header file of ADC HAL module.
 * @version V1.0.2
 * @date    2022/10/09
 * @author  huangzheng
 ******************************************************************************
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __A0_HAL_ADC_H
#define __A0_HAL_ADC_H

/* Includes -----------------------------------------------------------------*/
#include "jat_typedef.h"
#include "a0.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup ADC
 * @brief  10 bit Analog Digithal Convert (ADC) module
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/**
 * @defgroup ADC_Exported_Types ADC Exported Types
 * @{
 */
/**
 * @brief  ADC Configuration Structure definition
 */
typedef struct
{
    uint32_t            chSel;              /*!< adc channel select, bit20~bit0 indicated ch20~ch0  */
    uint32_t            adcClk;             /*!< adc clock config, 1MHz~16MHz                       */
    uint32_t            delayCycle;         /*!< adc delay cycle before convert, 5~15               */
}ADC_InitTypeDef;
/**
 * @}
 */

/* Exported constants -------------------------------------------------------*/
/**
 * @defgroup ADC_Exported_Constants ADC Exported Constants
 * @{
 */
/**
 * @defgroup ADC_CONVERT_CH_MODE ADC Convert Channel Mode
 * @brief ADC Convert Channel Mode, value of 'mode' in register 'CR', @ref ADC_CR_TypeDef
 * @{
 */
#define ADC_CH_MODE_SINGLE                  (1U)    /*!< ADC convert only one channel after every start                     */
#define ADC_CH_MODE_MULTI                   (0U)    /*!< ADC convert multi channels witch configed in register ADC->MCHSR   */
/**
 * @}
 */
/**
 * @}
 */

/* Exported macros ----------------------------------------------------------*/
/**
 * @defgroup ADC_Exported_Macros ADC Exported Macros
 * @{
 */
/**
 * @brief  Adc module convert busy state check, read bit 'busy' in register 'CR', @ref ADC_CR_TypeDef
 * @retval !=0     adc convert busy
 * @retval ==0     adc convert idle
 */
#define HAL_ADC_BUSY_CHECK()                                            (ADC->CR & (BIT30))
/**
 * @brief  Adc channel data get in multi channel convert mode, read register 'DR[CH]', @ref ADC_TypeDef
 * @param  CH      channel number, 0~20
 * @return 10 bit adc convert value of (CH) channel
 */
#define HAL_ADC_CH_DATA_GET(CH)                                         (ADC->DR[(CH)])
/**
 * @brief  Adc control register get, read register 'CR', @ref ADC_TypeDef
 * @return Value read from adc control register
 */
#define HAL_ADC_CTRL_GET()                                              (ADC->CR)
/**
 * @brief  Adc control register set, write register 'CR', @ref ADC_TypeDef
 * @param  CTRL    value write to adc control register
 * @return None
 */
#define HAL_ADC_CTRL_SET(CTRL)                                          (ADC->CR = (CTRL))
/**
 * @brief  Adc channels select get in multi channel convert mode, read register 'MCHSR', @ref ADC_TypeDef
 * @return Value read from adc channels select register
 */
#define HAL_ADC_MULTI_CH_SELECT_GET()                                   (ADC->MCHSR)
/**
 * @brief  Adc channels select set in multi channel convert mode, write register 'MCHSR', @ref ADC_TypeDef
 * @param  SEL     value of channels select, e.g. write 0x5 to select ch0 and ch2 in multi channel convert mode.
 * @return None
 */
#define HAL_ADC_MULTI_CH_SELECT_SET(SEL)                                (ADC->MCHSR = (SEL))
/**
 * @brief  Adc module power down disable,  clr bit 'powerDown' in register 'CR', @ref ADC_CR_TypeDef
 * @return None
 */
#define HAL_ADC_PD_DIS()                                                (ADC->CR &= (~BIT29))
/**
 * @brief  Adc module power down enable, set bit 'powerDown' in register 'CR', @ref ADC_CR_TypeDef
 * @return None
 */
#define HAL_ADC_PD_EN()                                                 (ADC->CR |= ( BIT29))
/**
 * @brief  Adc module start to convert, set bit 'start' in register 'CR', @ref ADC_CR_TypeDef
 * @return None
 */
#define HAL_ADC_START()                                                 (ADC->CR |= ( BIT28))
/**
 * @}
 */

/* Private macros -----------------------------------------------------------*/
/* Exported functions -------------------------------------------------------*/
/**
 * @addtogroup ADC_Exported_Functions ADC Exported Functions
 * @{
 */
/**
 * @brief  hal adc initial function
 * @param  *pInit  pointer to a struct variable of @ref ADC_InitTypeDef
 * @return None
 */
void HalAdcInit(ADC_InitTypeDef *pInit);

/* for code compatible */
#define hal_adc_init                        HalAdcInit
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
