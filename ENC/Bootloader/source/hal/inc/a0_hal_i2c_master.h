/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_i2c_master.h
 * @brief   Header file of I2C MASTER HAL module.
 * @version V1.0.3
 * @date    2022/04/07
 * @author  huangzheng
 ******************************************************************************
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __A0_HAL_I2C_MASTER_H
#define __A0_HAL_I2C_MASTER_H

/* Includes -----------------------------------------------------------------*/
#include "jat_typedef.h"
#include "a0.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup I2C_Master
 * @brief Inter-Integrated Circuit (I2C) master module
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/**
 * @defgroup I2C_Master_Exported_Types I2C Master Exported Types
 * @{
 */
/**
 * @brief  I2C Master Handler Structure definition
 */
typedef struct
{
    uint32_t            devAddr;                            /* 0x00~0x7F */
    uint32_t            len;
    uint32_t            timeOutMs;
    uint8_t             *pWrData;
    uint8_t             *pRdData;
}I2C_M_HandleTypeDef;
/**
 * @brief  I2C Master Configuration Structure definition
 */
typedef struct
{
    uint32_t            speed;                              /*!< Config i2c master speed */
    I2C_M_HandleTypeDef *pHandle;
    uint32_t            handleNumber;
}I2C_M_InitTypeDef;
/**
 * @}
 */

/* Exported constants -------------------------------------------------------*/
/**
 * @defgroup I2C_Master_Exported_Constants I2C Master Exported Constants
 * @{
 */
/**
 * @defgroup I2C_Master_Error I2C Master Error Type
 * @brief Error state in the HAL of I2C master
 * @{
 */
#define I2C_M_ERROR_NONE                    (0x0000U)   /*!< No error */
#define I2C_M_ERROR_LENZERO                 (0x0001U)   /*!< Length is zero error */
#define I2C_M_ERROR_NACK                    (0x0002U)   /*!< Get nack error */
#define I2C_M_ERROR_TIMEOUT                 (0x0004U)   /*!< Time out error */
#define I2C_M_ERROR_HANDLE                  (0x0008U)   /*!< Handle index is out of range */
#define I2C_M_ERROR_INIT                    (0x0010U)   /*!< I2C speed config init error */
/**
 * @}
 */

/**
 * @defgroup I2C_Master_WR_Mode I2C Master Write Mode
 * @brief I2C master write mode for eeprom or other, use for 'mode' of @ref HalI2cMasterWrite
 * @{
 */
#define I2C_M_WR_MODE_NORMAL                (0x00U)     /*!< End with stop in write mode */
#define I2C_M_WR_MODE_EEPROM                (0x01U)     /*!< Do not send stop with correct write, prepare to read eeprom */
/**
 * @}
 */

/**
 * @defgroup I2C_Master_RD_Mode I2C Master Read Mode
 * @brief I2C master read mode for eeprom or other, use for 'mode' of @ref HalI2cMasterRead
 * @{
 */
#define I2C_M_RD_MODE_NORMAL                (0x00UL)    /*!< Check idle and begin with start */
#define I2C_M_RD_MODE_EEPROM                (0x01UL)    /*!< Do not check idle and begin with restart */
/**
 * @}
 */
/**
 * @}
 */

/* Exported macros ----------------------------------------------------------*/
/**
 * @defgroup I2C_Master_Exported_Macros I2C Master Exported Macros
 * @{
 */
/**
 * @brief  i2c master busy flag check, read bit 'busy' in register 'CR', @ref I2C_M_CR_TypeDef
 * @retval == 0    bus idle
 * @retval != 0    bus busy
 */
#define HAL_I2C_M_BUSY_CHECK()                                          (I2CMaster->CR & (BIT14))
/**
 * @brief  i2c master transport state flag check, read bit 'tip' in register 'CR', @ref I2C_M_CR_TypeDef
 * @retval == 0    transport complete
 * @retval != 0    transporting
 */
#define HAL_I2C_M_TIP_CHECK()                                           (I2CMaster->CR & (BIT13))
/**
 * @brief  i2c master rx ack check, read bit 'ackRx' in register 'CR', @ref I2C_M_CR_TypeDef
 * @retval == 0    receive NACK
 * @retval != 0    receive ACK
 */
#define HAL_I2C_M_ACK_CHECK()                                           (I2CMaster->CR & (BIT15))
/**
 * @brief  i2c master ctrl register set, write reigster 'CR' in @ref I2C_M_TypeDef
 * @param  CTRL    set value write to i2c master ctrl register
 * @return None
 */
#define HAL_I2C_M_CTRL_SET(CTRL)                                        (I2CMaster->CR = (CTRL))
/**
 * @brief  i2c master clock register set, write reigster 'CSR' in @ref I2C_M_TypeDef
 * @param  VAL     set value write to i2c master clock set register
 * @return None
 */
#define HAL_I2C_M_CLOCK_SET(VAL)                                        (I2CMaster->CSR = (VAL))
/**
 * @brief  i2c master tx data write, write register 'TDR' in @ref I2C_M_TypeDef
 * @param  DATA    byte data write to i2c master tx data register
 * @return None
 */
#define HAL_I2C_M_TX_DATA_WR(DATA)                                      (I2CMaster->TDR = (DATA))
/**
 * @brief  i2c master rx data read, read register 'RDR' in @ref I2C_M_TypeDef
 * @return byte data read from i2c master rx data register
 */
#define HAL_I2C_M_RX_DATA_RD()                                          (I2CMaster->RDR)
/**
 * @}
 */

/* Exported functions -------------------------------------------------------*/
/**
 * @addtogroup I2C_Master_Exported_Functions I2C Master Exported Functions
 * @{
 */
/**
 * @brief  i2c master module initial process function
 * @param  pInit       Pointer to structure of @ref I2C_M_InitTypeDef
 * @return return state, @ref I2C_Master_Error
 */
int32_t HalI2cMasterInit(I2C_M_InitTypeDef *pInit);

/**
 * @brief  i2c master module write process function
 * @param  pInit       Pointer to structure of @ref I2C_M_InitTypeDef
 * @param  handleIdx   index number of handle
 * @param  mode        @ref I2C_Master_WR_Mode
 * @return return state, @ref I2C_Master_Error
 */
int32_t HalI2cMasterWrite(I2C_M_InitTypeDef *pInit, uint32_t handleIdx, uint8_t mode);

/**
 * @brief  i2c master module read process function
 * @param  pInit       Pointer to structure of @ref I2C_M_InitTypeDef
 * @param  handleIdx   index number of handle
 * @param  mode        @ref I2C_Master_RD_Mode
 * @return return state, @ref I2C_Master_Error
 */
int32_t HalI2cMasterRead(I2C_M_InitTypeDef *pInit, uint32_t handleIdx, uint8_t mode);

/* for code compatible */
#define hal_i2c_master_init                     HalI2cMasterInit
#define hal_i2c_master_write                    HalI2cMasterWrite
#define hal_i2c_master_read                     HalI2cMasterRead
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
