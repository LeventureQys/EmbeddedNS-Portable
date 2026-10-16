/**
  ******************************************************************************
  * @file    drv_i2c_master.c
  * @brief   i2c drv module.
  * @version V1.0.0
  * @date    2021/09/27
  * @author  lijixuan 
  ******************************************************************************
  */

/**
  * ∞Ê±æ»’÷æ£∫
  * V1.0.0, 2021/09/27
  *
  */

/* Define to prevent recursive inclusion -------------------------------------*/  
#ifndef __DRV_I2C_S_H
#define __DRV_I2C_S_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "a0_hal_conf.h"
#include "a0_hal_def.h"
#include "a0_hal_i2c_slave.h"

/** @addtogroup I2C_Slave_Driver
  * @{
  */

/** @addtogroup I2C_Slave
  * @{
  */ 

/* Exported constants --------------------------------------------------------*/
/** @defgroup I2C_Master_Driver__Exported_Constants I2C_Master Exported Constants
  * @{
  */

/** @defgroup Adderss_Slave_Device Adderss Slave Device
  * @brief Adderss Slave Device
  * @{
  */
#define I2C_DEV_ADDR                                        (0x5A)    /*!< slave device address                  */ 
/**
  * @}
  */

/**
  * @}
  */

/* Exported types------------------------------------------------------------*/
/** @defgroup I2C_Slave_DRV_Exported_Types  I2C slave drv exported types
  * @{
  */
  
/**
  * @brief  I2C slave initial parameter
  */ 
typedef struct
{
    uint32_t            address;                                         /*!< slave device address                */ 
    I2C_S_InitTypeDef   I2C_S_InitStruct;                                /*!< i2c slave initial parameter         */
    I2C_S_HandleTypeDef i2cHandle[1];                                    /*!< i2c slave handle parameter          */ 
}DRV_VAR_I2C_S_TypeDef;

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup I2C_Slave_DRV_Exported_Types_Functions I2C_Slave_DRV exported types functions
  * @{
  */
void DrvI2cSlaveInit(void);
void DrvI2cSlaveIrqHandle(void);

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
