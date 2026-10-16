/**
  ******************************************************************************
  * @file    drv_matrix_key.h
  * @brief   Header file of Matrix Key DRV module.
  * @version V1.0.0
  * @date    2021/09/27
  * @author  ziyingli
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/  
#ifndef __DRV_MATRIX_KEY_H__
#define __DRV_MATRIX_KEY_H__

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"   
#include "a0_hal_gpio.h"  

/** @addtogroup Matrix_Key_Drive_Driver
  * @{
  */
	
/** @addtogroup Matrix_Key
  * @{
  */ 

/* Exported constants --------------------------------------------------------*/
/** @defgroup Matrix_Key_Exported_Constants Matrix Key Exported Constants
  * @{
  */
 /**
  * @brief  Define the number of rows and columns
  */

#define	KEY_ROW_NUM		3
#define	KEY_COL_NUM		3

/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup Matrix_Key_Exported_Types Matrix Key Exported Types
  * @{
  */
  
  /**
  * @brief  Key Configuration Structure definition
  */
typedef struct _STRUCT_GPIO_ 
{
	GPIO_TypeDef* GPIOx;
	uint32_t GPIO_Pin;
}GpioStruct, *pGpioStruct;


typedef struct _STRUCT_KEYPAD_
{
	GpioStruct rowNum[KEY_ROW_NUM];
	GpioStruct colNum[KEY_COL_NUM];	
    
}KeypadGpioStruct,*pKeypadGpioStruct;

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup Matrix_Key_Exported_Functions  Matrix Key Exported Functions
  * @{
  */
void drv_matrix_key_Init(void);
uint32_t drv_matrix_key_scan(void);
extern void HAL_TickDelay(volatile uint32_t Delay);	
/**
  * @}
  */

#endif
