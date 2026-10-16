/**
  ******************************************************************************
  * @file    drv_i2c_master.c
  * @brief   i2c master drv module.
  * @version V1.0.0
  * @date    2021/09/27
  * @author  lijixuan 
  ******************************************************************************
  */
 
/**
  * 版本日志：
  * V1.0.0, 2021/09/27
  * 1. 同时挂3个I2C从设备。
  * 2. 该I2C驱动只定义了主机只能挂载3个从机设备，更改从机挂载数量需改I2C_DEV_HANDLE_NUM的值
  * 3. 自身从设备的读写和协议相关，这里自定义的协议，具体参考 drv_i2c_slave.c
*/

/* Define to prevent recursive inclusion -------------------------------------*/ 
#include "conf_drv_en.h"
#ifdef DRV_I2C_MASTER_ENABLED

/* Includes ------------------------------------------------------------------*/
#include "drv_i2c_master.h"

/* Define to prevent recursive inclusion -------------------------------------*/
#ifdef HAL_I2C_MASTER_MODULE_ENABLED

/* Includes ------------------------------------------------------------------*/
#include "timer.h"

/** @addtogroup I2C_Master_Driver
  * @{
  */
  
/** @addtogroup I2C_Master
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @defgroup Define_I2C_Master_Device_Buff
  * @{
  */
 I2C_M_Typedef    i2cMasterVar;

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup UART_DRV_Exported_Functions I2C_Master_DRV exported types functions
  * @{
  */
 
 /**
  * @brief  Initializes the I2C_Master_Codec peripheral.
  * @param  I2C_M_CDC_Typedef  Pointer to a I2C_Master_Codec structure initial master2codec parameter.
  *         This parameter can be the value of @ref I2C master2codec initial parameter
  * @retval none     
  *                
  */
 void DrvI2cMasterInitCodec(I2C_M_CDC_Typedef* pInitCDC)
{
    I2C_M_Typedef       *pVar = &i2cMasterVar;
//    I2C_M_InitTypeDef   *pVarInit = &pVar->I2C_M_InitStruct;    
    I2C_M_HandleTypeDef *pHandle = pVar->i2cHandle + pInitCDC->handleIdx;
    
    pHandle->devAddr = pInitCDC->devAddr;
    pHandle->len = pInitCDC->len;
    pHandle->pWrData = (uint8_t *)&pInitCDC->txDataCodec;
    pHandle->timeOutMs = pInitCDC->timeOutMs;
    return;
}

 /**
  * @brief  Initializes the I2C_Master_Slave peripheral.
  * @param  I2C_M_S_Typedef  Pointer to a I2C_Master_Slave structure initial master2slave parameter.
  *         This parameter can be the value of @ref I2C master2slaver initial parameter
  * @retval none     
  *                
  */
 void DrvI2cMasterInitSlave(I2C_M_S_Typedef* pInitMS)
{
    I2C_M_Typedef       *pVar = &i2cMasterVar;
//    I2C_M_InitTypeDef   *pVarInit = &pVar->I2C_M_InitStruct;    
    I2C_M_HandleTypeDef *pHandle = pVar->i2cHandle + pInitMS->handleIdx;
    
    pHandle->devAddr = pInitMS->devAddr;
    pHandle->pWrData = (uint8_t *)&pInitMS->txDataSlave;
    pHandle->pRdData = (uint8_t *)&pInitMS->rxDataSlave;
    pHandle->timeOutMs = pInitMS->timeOutMs;
    return;
}

 /**
  * @brief  Initializes the I2C_Master_EEPROM peripheral.
  * @param  I2C_M_E2R_Typedef  Pointer to a I2C_Master_EEPROM structure initial master2eeprom parameter.
  *         This parameter can be the value of @ref I2C master2eeprom initial parameter
  * @retval none     
  *                
  */
 void DrvI2cMasterInitEeprom(I2C_M_E2R_Typedef* pInitEER)
{
    I2C_M_Typedef       *pVar = &i2cMasterVar;
    I2C_M_HandleTypeDef *pHandle = (I2C_M_HandleTypeDef *)pVar->i2cHandle + pInitEER->handleIdx;  
	
    pHandle->devAddr = pInitEER->devAddr;
    pHandle->timeOutMs = pInitEER->timeOutMs;
    return;
}

/**
  * @brief  Write Data to codec regester.
  * @param  regAddr  Codec regester address to write
  * @param  regData  Codec data to write in regester
  * @param  len  The number of data write in regester
  * @param  pInitCDC The codec device identifier structure
  * @retval This parameter can be one of the following values:
  *         @ref  Error state in the HAL of I2C master
  */
int32_t DrvI2cCodecWriteReg(uint8_t regAddr, uint16_t regData,uint32_t len,I2C_M_CDC_Typedef* pInitCDC)
{
    I2C_M_Typedef       *pVar = &i2cMasterVar;
    I2C_M_InitTypeDef   *pVarInit = &pVar->I2C_M_InitStruct;   
    I2C_M_HandleTypeDef *pHandle = pVarInit->pHandle + pInitCDC->handleIdx;    
    pInitCDC->txDataCodec = (regData & 0x1FF) + ((uint16_t)regAddr << 9);
    pInitCDC->txDataCodec = ((pInitCDC->txDataCodec >> 8) & 0xFF) | (pInitCDC->txDataCodec << 8);
    pHandle->pWrData = (uint8_t *)&pInitCDC->txDataCodec;
    pHandle->len = len;    
    return (HalI2cMasterWrite(pVarInit,pInitCDC->handleIdx, 0));    
}

/**
  * @brief  Write data to eeprom.
  * @param  pData   The data pointer write to eeprom.
  * @param  len  The number of data write in eeprom
  * @param  pInitEER  The eeprom device identifier structure
  * @retval This parameter can be one of the following values:
  *         @ref  Error state in the HAL of I2C master
  */
int32_t DrvI2cEepromWrite(uint8_t *pData,uint32_t len,I2C_M_E2R_Typedef* pInitEER)
{
    I2C_M_Typedef       *pVar = &i2cMasterVar;
    I2C_M_InitTypeDef   *pVarInit = &pVar->I2C_M_InitStruct;
    I2C_M_HandleTypeDef *pHandle = pVar->i2cHandle + pInitEER->handleIdx;
 
    pHandle->pWrData = pData;
    pHandle->len = len;
    return (HalI2cMasterWrite(pVarInit,pInitEER->handleIdx, 0));
}

/**
  * @brief  Write data to i2c slave device with A0.
  * @param  ramAddr  The address where the data was started to be written
  * @param  pRamData  The data pointer write to i2c slave device with A0.
  * @param  byteNum  The number of data write in eeprom
  * @param  pInitMS The i2c slave device with A0 identifier structure
  * @retval This parameter can be one of the following values:
  *         @ref  Error state in the HAL of I2C master
  */
int32_t DrvI2cSlaveCtrlWrite(uint32_t ramAddr, uint8_t *pRamData, uint32_t byteNum,I2C_M_S_Typedef* pInitMS)
{
    I2C_M_Typedef       *pVar = &i2cMasterVar;
    I2C_M_InitTypeDef   *pVarInit = &pVar->I2C_M_InitStruct;
    I2C_M_HandleTypeDef *pHandle = pVarInit->pHandle + pInitMS->handleIdx;
    int32_t retVal;

    uint32_t *pTxData = (uint32_t *)pInitMS->txDataSlave;
    *pTxData++ = ramAddr;

    pHandle->len = 16;
    while (byteNum > 12)
    {
        pHandle->pWrData = (uint8_t *)&pInitMS->txDataSlave;        
        memcpy(pTxData, pRamData, 12);
        if ((retVal = HalI2cMasterWrite(pVarInit,pInitMS->handleIdx, 0)) != I2C_M_ERROR_NONE)
        {
            byteNum = 0;
            break;
        }
        pInitMS->txDataSlave[0] += 12;
        pRamData += 12;
        byteNum -= 12;
    }
    if (byteNum > 0)
    {
        pHandle->pWrData = (uint8_t *)&pInitMS->txDataSlave;        
        pHandle->len = 4 + byteNum;
        memcpy(pTxData, pRamData, byteNum);
        retVal = HalI2cMasterWrite(pVarInit,pInitMS->handleIdx,0);
    }    
    return retVal;
}

/**
  * @brief  Read data from eeprom.
  * @param  pData   The data pointer to receive from eeprom.
  * @param  len  The number of data read from eeprom
  * @param  pInitEER  The eeprom device identifier structure
  * @retval This parameter can be one of the following values:
  *         @ref  Error state in the HAL of I2C master
  */
int32_t DrvI2cEepromRead(uint8_t *pData,uint32_t len,I2C_M_E2R_Typedef* pInitEER)
{
    I2C_M_Typedef       *pVar = &i2cMasterVar;
    I2C_M_InitTypeDef   *pVarInit = &pVar->I2C_M_InitStruct; 
    I2C_M_HandleTypeDef *pHandle = pVarInit->pHandle + pInitEER->handleIdx;    
    int32_t retVal;    
    pHandle->pWrData = pData;
    pHandle->pRdData = pData+2;
    pHandle->len = 2;
    if ((retVal = HalI2cMasterWrite(pVarInit,pInitEER->handleIdx, 1)) == I2C_M_ERROR_NONE)
    {
        pHandle->len = len;
        retVal = HalI2cMasterRead(pVarInit, pInitEER->handleIdx, 1);
    }
    return retVal;
}

/**
  * @brief  Read data from i2c slave device with A0.
  * @param  ramAddr   The address where the data is read out
  * @param  pRamData  The data pointer to receive from eeprom
  * @param  byteNum  The number of data read from i2c slave device with A0 
  * @param  pInitMS  The i2c slave device with A0 identifier structure
  * @retval This parameter can be one of the following values:
  *         @ref  Error state in the HAL of I2C master
  */
int32_t DrvI2cSlaveCtrlRead(uint32_t ramAddr, uint8_t *pRamData, uint32_t byteNum,I2C_M_S_Typedef* pInitMS)
{
    I2C_M_Typedef       *pVar = &i2cMasterVar;
    I2C_M_InitTypeDef   *pVarInit = &pVar->I2C_M_InitStruct; 
    I2C_M_HandleTypeDef *pHandle = pVarInit->pHandle + pInitMS->handleIdx;
    int32_t retVal;

    pHandle->len = 4;
    uint32_t *pTxData = (uint32_t *)pInitMS->txDataSlave;
    *pTxData++ = ramAddr;

    pHandle->pWrData = (uint8_t *)&pInitMS->txDataSlave;
    if ((retVal = HalI2cMasterWrite(pVarInit,pInitMS->handleIdx, 0)) == I2C_M_ERROR_NONE)
    {
        pHandle->len = 16;
        while (byteNum > 16)
        {
            pHandle->pRdData = (uint8_t *)&pInitMS->rxDataSlave;            
            if ((retVal = HalI2cMasterRead(pVarInit,pInitMS->handleIdx, 0)) == I2C_M_ERROR_NONE)
            {
                memcpy(pRamData, pInitMS->rxDataSlave, 16);
                pInitMS->rxDataSlave[0] += 16;
                pRamData += 16;
                byteNum -= 16;
            }
            else
            {
                byteNum = 0;
                break;
            }            
        }
        if (byteNum > 0)
        {
            pHandle->pRdData = (uint8_t *)&pInitMS->rxDataSlave;               
            pHandle->len = byteNum;
            if ((retVal = HalI2cMasterRead(pVarInit,pInitMS->handleIdx, 0)) == I2C_M_ERROR_NONE)
            {
                memcpy(pRamData, pInitMS->rxDataSlave, byteNum);
            }
        }
    }
    return retVal;
}

 /**
  * @brief  Initializes the I2C_Master peripheral.
  * @retval none                
  */
void DrvI2cMasterInit(void)
{
    I2C_M_Typedef       *pVar = &i2cMasterVar;
    I2C_M_InitTypeDef   *pVarInit = &pVar->I2C_M_InitStruct;
    I2C_M_HandleTypeDef *pHandle = pVar->i2cHandle;
    
    pVarInit->speed = I2C_SPEED;
    pVarInit->handleNumber = I2C_DEV_HANDLE_NUM;
    pVarInit->pHandle = pHandle;
    HalI2cMasterInit(pVarInit);
    return;
}

#else

#endif

#endif

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/******************************************************************************/
