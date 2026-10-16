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
  *
  */
/*
1. i2c从机接收数据的前4个字节表示地址，当结束数据超过4个字节之后，后面的数据表示写入该地址的数据，当小于4个字节时，直接作为数据写入地址
2. 地址范围仅限制在[0x20004000,0x20007FFF]
3. 该地址会做记录
4. i2c从机发送数据默认从接收的地址开始发送，每次发送完之后，地址自动累加
*/

/* Define to prevent recursive inclusion -------------------------------------*/ 
#include "conf_drv_en.h"
#ifdef DRV_I2C_MASTER_ENABLED

/* Includes ------------------------------------------------------------------*/
#include "drv_i2c_slave.h"

/* Define to prevent recursive inclusion -------------------------------------*/
#ifdef HAL_I2C_SLAVE_MODULE_ENABLED

/** @addtogroup I2C_Slave_Driver
  * @{
  */

/** @addtogroup I2C_Slave
  * @{
  */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/** @defgroup Define_I2C_Slave_Device_Buff
  * @{
  */  
static DRV_VAR_I2C_S_TypeDef    i2cSlaveVar;
/**
  * @}
  */
  
/* Private functions ---------------------------------------------------------*/
static void I2cSlaveTxCallback(uint8_t *pTxData, uint8_t txNum);
static void I2cSlaveRxCallback(uint8_t *pRxData, uint8_t rxNum);

/* Exported functions --------------------------------------------------------*/
/** @addtogroup I2C_Slave_DRV_Exported_Types_Functions I2C_Slave_DRV exported types functions
  * @{
  */
  
 /**
  * @brief  Initializes the I2C_Master peripheral.
  * @retval none                
  */
void DrvI2cSlaveInit(void)
{
    DRV_VAR_I2C_S_TypeDef       *pVar = &i2cSlaveVar;
    I2C_S_InitTypeDef   *pVarInit = &pVar->I2C_S_InitStruct;
    I2C_S_HandleTypeDef *pHandle = pVar->i2cHandle;
    
    pVarInit->devAddr = I2C_DEV_ADDR;
    pVarInit->sclHoldEn = 0;
    pVarInit->pHandle = pHandle;

    pHandle->pFunRxCb = I2cSlaveRxCallback;
    pHandle->pFunTxCb = I2cSlaveTxCallback;
    
    HalI2cSlaveInit(pVarInit);
    return;
}

 /**
  * @brief  Interrupt i2c slave Handler
  * @retval none                
  */
void DrvI2cSlaveIrqHandle(void)
{
    DRV_VAR_I2C_S_TypeDef       *pVar = &i2cSlaveVar;   
    I2C_S_InitTypeDef   *pVarInit = &pVar->I2C_S_InitStruct;
    
    HalI2cSlaveIrqHandler(pVarInit);
    return;
}

/**
  * @brief  Callback after i2c master read over
  * @param  pTxData   Data pointer for i2c master read data next time
  * @param  txNum  Byte number i2c master read just now
                   This parameter can be one of the values:0~16
  * @retval none                 
  */
static void I2cSlaveTxCallback(uint8_t *pTxData, uint8_t txNum)
{
    DRV_VAR_I2C_S_TypeDef       *pVar = &i2cSlaveVar; 
    pVar->address += txNum;
    memcpy(pTxData, (uint8_t *)pVar->address, 16);
    return;
}

/**
  * @brief  Callback after i2c master write over
  * @param  pTxData   Data pointer for i2c master write data next time
  * @param  rxNum  Byte number i2c master write just now
  *                This parameter can be one of the values:0~16
  * @retval none                 
  */
static void I2cSlaveRxCallback(uint8_t *pRxData, uint8_t rxNum)
{
    DRV_VAR_I2C_S_TypeDef       *pVar = &i2cSlaveVar; 
    if (rxNum >= 4)
    {
        pVar->address = *((uint32_t *)pRxData);
        pRxData += 4;
        rxNum -= 4;
    }
    while (rxNum--)
    {
        if ((pVar->address >= 0x20004000) && (pVar->address <= 0x20007FFF))
        {
            *((uint8_t *)pVar->address) = *pRxData++;
            pVar->address++;
        }
        else
        {
            break;
        }
    }
    return;
}
#else
void DrvI2cSlaveInit(void)
{
    return;
}

void DrvI2cSlaveIrqHandle(void)
{
    return;
}
#endif

#else
void DrvI2cSlaveIrqHandle(void)
{
    return;
}
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
