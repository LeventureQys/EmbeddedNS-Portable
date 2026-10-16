/**
  ******************************************************************************
  * @file    demo_pwm.h
  * @brief   Header file of pwm demo .
  * @version V1.0.0
  * @date    2022/04/16
  * @author  lijixuan
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/  
#ifndef __DEMO_PWM_H
#define __DEMO_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "a0_hal_gpio.h"
#include "a0_hal_conf.h"
#include "a0_hal_def.h"

/* Exported constants --------------------------------------------------------*/
/** @defgroup PWM_DEMO_Exported_Constants PWM Exported Constants
  * @{
  */
  
 /** @defgroup 
  * @brief PWM mode select
  * @{
  */ 


 /** @defgroup 
  * @brief PWM register control
  * @{
  */ 

/**
  * @}
  */
  
 /**
  * @}
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup PWM Demo Exported Types
  * @{
  */
  
 /**
  * @brief  pwm channel count value get
  */


 /**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup PWM_Demo_Exported_Functions PWM Demo Exported Functions
  * @{
  */
void DemoPwm(void);
void DemoPwmIrq(void);
 /**
  * @}
  */

#ifdef __cplusplus
}
#endif


#endif /*__DEMO_PWM_H*/


