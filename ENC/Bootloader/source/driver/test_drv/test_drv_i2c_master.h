/**
  ******************************************************************************
  * @file    app_i2c.c
  * @brief   i2c app module.
  * @version V1.0.0
  * @date    2021/09/28
  * @author  lijixuan 
  ******************************************************************************
  */
 
/**
  * 版本日志：
  * V1.0.0, 2021/09/28
  * 相关说明见app_i2c.c文件
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/  
#ifndef __TEST_DRV_I2C_H
#define __TEST_DRV_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "a0_hal_i2c_master.h"
#include "a0_hal_i2c_slave.h"

/** @addtogroup I2C_APP
  * @{
  */

/** @addtogroup I2C
  * @{
  */ 

/* Exported constants --------------------------------------------------------*/
/** @defgroup I2C_App_Exported_Constants I2C_App Exported Constants
  * @{
  */
  
/** @defgroup I2C_Device_Address 
  * @{
  */
#define TEST_DRV_IIC
#define I2C_DEV_HANDLE_ADDR_CODEC                   (0x1A)  /*!< codec devices address                */
#define I2C_DEV_HANDLE_ADDR_SLAVE                   (0x5A)  /*!< slave devices address                */
#define I2C_DEV_HANDLE_ADDR_EEPROM                  (0x50)  /*!< eeprom devices address               */

/**
  * @}
  */

/**
  * @}
  */
  
/* Exported functions --------------------------------------------------------*/
/** @defgroup I2C_App_Exported_Types_Functions I2C_App exported types functions
  * @{
  */
void TestDrvI2cModeSelect(uint32_t testI2cFlag);
void TestDrvI2cModeSletM2S(uint32_t testI2cFlag);
void TestDrvI2C(void);
void TestDrvI2CReadCheck(uint32_t addr1, uint32_t addr2, uint32_t len);
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

