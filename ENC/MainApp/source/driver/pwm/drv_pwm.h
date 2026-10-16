/**
  ******************************************************************************
  * @file    drv_pwm.h
  * @brief   Header file of PWM DRV module.
  * @version V1.0.0
  * @date    2022/04/16
  * @author  lijixuan
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/  
#ifndef __DRV_PWM_H
#define __DRV_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "a0_hal_conf.h"
#include "a0_hal_pwm.h"

/** @addtogroup PWM_Drive_Driver
  * @{
  */
  
/* Exported types ------------------------------------------------------------*/
/** @defgroup PWM_DRV_Exported_Types pwm Exported Types
  * @{
  */

/**
  * @brief  PWM Configuration Structure 
  */
typedef struct
{
    uint8_t                     mode;
    uint8_t                     chIdx;
    uint8_t                     inverseEn;          //[0,1]
    union
    {
        PWM_StdModeTypeDef  paraStandard;  
        PWM_BreModeTypeDef  paraBreatheAuto;
        PWM_BreModeTypeDef  paraBreatheOnce;
        PWM_CntModeTypeDef  paraCount;
    }modePara;
}PWM_MODE_PARA_TypeDef;

/**
  * @brief  PWM_MODE0 Configuration Structure 
  */
typedef struct
{
    PWM_StdModeTypeDef parMode0;
    uint8_t chIdx;
    uint8_t handleNum;    
}PAR_PWM_MODE0_TypeDef;

/**
  * @brief PWM_MODE1 Configuration Structure
  */
typedef struct
{
    PWM_BreModeTypeDef parMode1;
    uint8_t chIdx; 
    uint8_t handleNum;     
}PAR_PWM_MODE1_TypeDef;

/**
  * @brief  PWM_MODE2 Configuration Structure
  */
typedef struct
{
    PWM_BreModeTypeDef parMode2;
    uint8_t chIdx; 
    uint8_t handleNum;     
}PAR_PWM_MODE2_TypeDef;

/**
  * @brief  PWM_MODE34 Configuration Structure
  */
typedef struct
{
    PWM_CntModeTypeDef parMode34;
    uint8_t chIdx;  
    uint8_t handleNum;     
}PAR_PWM_MODE34_TypeDef;
/** 
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup PWM_DRV_Exported_Functions PWM Drv Exported Functions
  * @{
  */
int32_t DrvPwmInit(PWM_MODE_PARA_TypeDef* pInit);
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
