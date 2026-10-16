/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_adc.c
 * @brief   ADC HAL module.
 * @version V1.0.2
 * @date    2022/10/09
 * @author  huangzheng
 ******************************************************************************
 */

/**
 * 版本日志：
 * V1.0.2, 2022/10/09
 *      1. HalAdcInit中，增加cr.w = 0，初始化清零操作，确保cr的其他未赋值的位域结构体成员是0
 * V1.0.1, 2022/04/07
 *      1. 函数命名按照大驼峰方式命名，头文件增加原函数名的define，为了兼容原代码，后续希望渐渐过渡到统一风格书写规范
 *      2. 整理头文件中依赖关系，使用jat_typedef.h作为数据类型的头文件，并把include放到extern "C" 之外
 *      3. 将一些配置选项移到a0_hal_conf.h，统一进行配置
 * V1.0.0, 2021/10/11,
 *      1. 初版，从GPIO的文件中独立出来
 *      2. 21路ADC控制器HAL层函数接口，头文件中包含各寄存器的接口宏定义
 */
/* Includes -----------------------------------------------------------------*/
#include "a0_hal_conf.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup ADC
 * @{
 */

#ifdef HAL_ADC_MODULE_ENABLED
#include "a0_hal_adc.h"
#include "a0_hal_def.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Private function prototypes ----------------------------------------------*/
/* Exported functions -------------------------------------------------------*/
/**
 * @defgroup ADC_Exported_Functions ADC Exported Functions
 * @{
 */
void HalAdcInit(ADC_InitTypeDef *pInit)
{
    ADC_CR_TypeDef cr;
    cr.w = 0;                                       /* default set zero */
    cr.b.rst = 1;                                   /* reset */
    cr.b.mode = 0;                                  /* use multi channel convert */
    cr.b.sCnt = pInit->delayCycle;
    cr.b.fCnt = (((SystemPclk + (pInit->adcClk >> 1)) / pInit->adcClk) + 1) / 2 - 1;
    HAL_ADC_CTRL_SET(cr.w);
    HAL_ADC_MULTI_CH_SELECT_SET(pInit->chSel);
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
