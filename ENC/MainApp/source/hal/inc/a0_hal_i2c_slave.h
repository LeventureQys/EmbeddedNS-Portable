/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_i2c_slave.h
 * @brief   Header file of I2C SLAVE HAL module.
 * @version V1.0.3
 * @date    2022/04/07
 * @author  huangzheng
 ******************************************************************************
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __A0_HAL_I2C_SLAVE_H
#define __A0_HAL_I2C_SLAVE_H

/* Includes -----------------------------------------------------------------*/
#include "jat_typedef.h"
#include "a0.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup I2C_Slave
 * @brief Inter-Integrated Circuit (I2C) slave module
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/**
 * @defgroup I2C_Slave_Exported_Types I2C Slave Exported Types
 * @{
 */
/**
 * @brief  I2C Slave Handler Structure definition
 */
typedef struct
{
    void                (*pFunRxCb)(uint8_t *pRxData, uint8_t rxNum);   /*!< callback function for i2c slave irq rx handle */
    void                (*pFunTxCb)(uint8_t *pTxData, uint8_t txNum);   /*!< callback function for i2c slave irq tx handle */
}I2C_S_HandleTypeDef;
/**
 * @brief  I2C Slave Configuration Structure definition
 */
typedef struct
{
    uint8_t             sclHoldEn;                          /*!< 1, enable scl hold; 0, disable scl hold; */
    uint8_t             devAddr;                            /*!< Config device address in i2c slave mode 7bit vaild */
    I2C_S_HandleTypeDef *pHandle;
}I2C_S_InitTypeDef;
/**
 * @}
 */

/* Exported constants -------------------------------------------------------*/
/**
 * @defgroup I2C_Slave_Exported_Constants I2C Slave Exported Constants
 * @{
 */
/**
 * @defgroup I2C_Slave_State I2C Slave State
 * @brief Value of 'state' in @ref I2C_S_CR_TypeDef for user to check
 * @{
 */
#define I2C_S_STATE_IDLE                    (0x0U)  /*!< i2c slave is in idle state */
#define I2C_S_STATE_START                   (0x1U)  /*!< i2c slave received start */
#define I2C_S_STATE_DEVADDR_RW              (0x2U)  /*!< i2c slave received device address with R/W */
#define I2C_S_STATE_ADDRACK                 (0x3U)  /*!< i2c slave address is correct and return ACK to master */
#define I2C_S_STATE_WDATA                   (0x4U)  /*!< i2c slave received data from master */
#define I2C_S_STATE_WDATAACK                (0x5U)  /*!< i2c slave return ACK for received data to master */
#define I2C_S_STATE_RDATA                   (0x6U)  /*!< i2c slave sent data to master */
#define I2C_S_STATE_RDATAACK                (0x7U)  /*!< i2c slave received ACK from master of sent data */
/**
 * @}
 */
/**
 * @}
 */

/* Exported macros ----------------------------------------------------------*/
/**
 * @defgroup I2C_Slave_Exported_Macros I2C Slave Exported Macros
 * @{
 */
/**
 * @brief  i2c slave ctrl register set, write register 'CR' in @ref I2C_S_TypeDef
 * @param  CTRL    set value write to i2c slave ctrl register
 * @return None
 */
#define HAL_I2C_S_CTRL_SET(CTRL)                                        (I2CSlave->CR = (CTRL))
/**
 * @brief  i2c slave ctrl register get, read register 'CR' in @ref I2C_S_TypeDef
 * @return get value read from i2c slave ctrl register
 */
#define HAL_I2C_S_CTRL_GET()                                            (I2CSlave->CR)
/**
 * @brief  i2c slave rx data get, read register 'RDR' according to IDX in @ref I2C_S_TypeDef
 * @param  IDX     index nunmber of i2c slave rx data register
 * @return 32bit rx data read from rx data register
 */
#define HAL_I2C_S_RX_DATA_GET(IDX)                                      (I2CSlave->RDR[(IDX)])
/**
 * @brief  i2c slave tx data set, write register 'RDR' according to IDX in @ref I2C_S_TypeDef
 * @param  IDX     index nunmber of i2c slave tx data register
 * @param  DATA    32bit tx data write to tx data register
 * @return None
 */
#define HAL_I2C_S_TX_DATA_SET(IDX, DATA)                                (I2CSlave->TDR[(IDX)] = (DATA))
/**
 * @}
 */

/* Exported functions -------------------------------------------------------*/
/**
 * @addtogroup I2C_Slave_Exported_Functions I2C Slave Exported Functions
 * @{
 */
/**
 * @brief  i2c slave module initial process function
 * @param  pInit    pointer to structure of @ref I2C_S_InitTypeDef
 * @return None
 */
void HalI2cSlaveInit(I2C_S_InitTypeDef *pInit);

/**
 * @brief  i2c slave module irq handler function
 * @param  pInit    pointer to structure of @ref I2C_S_InitTypeDef
 * @return None
 */
void HalI2cSlaveIrqHandler(I2C_S_InitTypeDef *pInit);

/* for code compatible */
#define hal_i2c_slave_init                      HalI2cSlaveInit
#define hal_i2c_slave_irq_handler               HalI2cSlaveIrqHandler
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
