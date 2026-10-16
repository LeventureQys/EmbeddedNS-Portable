
#ifndef __TEST_DRV_PWM_H
#define __TEST_DRV_PWM_H

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
void TestDrvPwm(void);
void TestDrvPwmIrq(void);
 /**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif

