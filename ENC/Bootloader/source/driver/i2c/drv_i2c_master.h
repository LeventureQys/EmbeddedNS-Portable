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
  * 版本日志：
  * V1.0.0, 2021/09/27
  * 说明见drv_i2c_master.c
  */

/* Define to prevent recursive inclusion -------------------------------------*/  
#ifndef __DRV_I2C_M_H
#define __DRV_I2C_M_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "a0_hal_conf.h"
#include "a0_hal_def.h"
#include "a0_hal_i2c_master.h"

/** @addtogroup I2C_Master_Driver
  * @{
  */

/** @addtogroup I2C_Master
  * @{
  */ 

/* Exported constants --------------------------------------------------------*/
/** @defgroup I2C_Master_Driver_Exported_Constants I2C_Master Exported Constants
  * @{
  */

/** @defgroup Allocate_Num_Device Allocate device number
  * @brief Allocate device number
  * @{
  */
#define I2C_DEV_HANDLE_NUM                          (3)     /*!< max devices support                 */
#define I2C_SPEED                                   (10000) /*!< i2c speed                           */  
/**
  * @}
  */
  
/**
  * @}
  */

/* Exported types------------------------------------------------------------*/
/** @defgroup I2C_Master_DRV_Exported_Types  I2C_Master_Exported Types
  * @{
  */

/**
  * @brief  I2C master initial parameter
  */
typedef struct
{
    I2C_M_InitTypeDef   I2C_M_InitStruct;                /*!< i2c master initial parameter          */
    I2C_M_HandleTypeDef i2cHandle[I2C_DEV_HANDLE_NUM];   /*!< i2c master handle parameter           */
}I2C_M_Typedef;

/**
  * @brief  I2C master2codec initial parameter
  */
typedef struct
{
    uint32_t    handleIdx;                               /*!< codec number ID                       */                          
    uint32_t    devAddr;                                 /*!< codec address                         */  
    uint32_t    len;                                     /*!< codec write data length               */  
    uint16_t    txDataCodec;                             /*!< codec data buffer                     */  
    uint32_t    timeOutMs;                               /*!< codec timeout set                     */  
}I2C_M_CDC_Typedef;

/**
  * @brief  I2C master2slaver initial parameter
  */
typedef struct
{
    uint32_t    handleIdx;                              /*!< slave device number ID                */  
    uint32_t    devAddr;                                /*!< slave device address                  */  
    uint32_t    len;                                    /*!< slave device write data length        */  
    uint32_t    txDataSlave[4];                         /*!< slave device tx data buffer           */  
    uint32_t    rxDataSlave[4];                         /*!< slave device rx data buffer           */  
    uint32_t    timeOutMs;                              /*!< slave device timeout set              */  
}I2C_M_S_Typedef;

/**
  * @brief  I2C master2eeprom initial parameter
  */
typedef struct
{
    uint32_t    handleIdx;                              /*!< eeprom number ID                      */  
    uint32_t    devAddr;                                /*!< eeprom device address                 */  
    uint32_t    len;                                    /*!< eeprom write data length              */  
    uint32_t    timeOutMs;                              /*!< eeprom timeout set                    */      
}I2C_M_E2R_Typedef;

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup I2C_Master_DRV_Exported_Types_Functions I2C_Master_DRV exported types functions
  * @{
  */
void DrvI2cMasterInit(void);
void DrvI2cMasterInitCodec(I2C_M_CDC_Typedef* pInitCDC);
void DrvI2cMasterInitSlave(I2C_M_S_Typedef* pInitMS);
void DrvI2cMasterInitEeprom(I2C_M_E2R_Typedef* pInitEER);
int32_t DrvI2cCodecWriteReg(uint8_t regAddr, uint16_t regData,uint32_t len,I2C_M_CDC_Typedef* pInitCDC);
int32_t DrvI2cSlaveCtrlWrite(uint32_t ramAddr, uint8_t *pRamData, uint32_t byteNum,I2C_M_S_Typedef* pInitMS);
int32_t DrvI2cSlaveCtrlRead(uint32_t ramAddr, uint8_t *pRamData, uint32_t byteNum,I2C_M_S_Typedef* pInitMS);
int32_t DrvI2cEepromWrite(uint8_t *pData,uint32_t len,I2C_M_E2R_Typedef* pInitEER);
int32_t DrvI2cEepromRead(uint8_t *pData,uint32_t len,I2C_M_E2R_Typedef* pInitEER);

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
