/**
  ******************************************************************************
  * @file    codec_wm8731.h
  * @author  Tao
  * @brief   Header file of CODEC Driver module.
  * 
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/  
#ifndef _WM8731_H_
#define _WM8731_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "a0_hal_def.h"
#include "a0_hal_i2c_master.h"
#include "app_i2c_master.h"
/** @addtogroup Driver
  * @{
  */

/** @addtogroup CODEC
  * @{
  */ 

/* Exported types ------------------------------------------------------------*/
/** @defgroup I2S_Exported_Types I2S Exported Types
  * @{
  */

/**
  * @brief  I2S Configuration Structure definition
  */


/**
  * @}
  */



/* Exported constants --------------------------------------------------------*/
/** @defgroup I2S_Exported_Constants I2S Exported Constants
  * @{
  */



/** @defgroup WM8731 addrress
  * @{
  */
#define WM8731_ADDR				0X1A      //WM8731的器件地址，固定为0X1A 
/**
  * @}
  */


/* Exported macros -----------------------------------------------------------*/
/** @defgroup I2S_Exported_Macros WM8731 Exported Macros
  * @{
  */

/**
  * @}
  */
 
/* Private macros --------------------------------------------------------*/
/** @defgroup I2S_Private_Macros   WM8731 Private Macros
  * @{
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup WM8731_Exported_Functions
  * @{
  */

uint8_t WM8731_Write_Reg(uint8_t reg,uint16_t val);
uint8_t WM8731_Init(void);
void WM8731_HPvol_Set(uint8_t voll,uint8_t volr);
//void WM8731_SPKvol_Set(uint8_t volx);

/**
  * @}
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

/** 
  * @}
  */


#ifdef __cplusplus
}
#endif

#endif

