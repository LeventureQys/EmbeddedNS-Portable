/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_i2c_slave.c
 * @brief   I2C SLAVE HAL module.
 * @version V1.0.3
 * @date    2022/04/07
 * @author  huangzheng
 ******************************************************************************
 */

/**
 *  版本日志：
 * V1.0.3, 2022/04/07
 *      1. 函数命名按照大驼峰方式命名，头文件增加原函数名的define，为了兼容原代码，后续希望渐渐过渡到统一风格书写规范
 *      2. 整理头文件中依赖关系，使用jat_typedef.h作为数据类型的头文件，并把include放到extern "C" 之外
 *      3. 将一些配置选项移到a0_hal_conf.h，统一进行配置
 * V1.0.2, 2021/10/11,
 *      1. a0.h中Slave的CR寄存器联合体定义名称修改，和其他改成一致的风格，对应代码适配修改
 *      2. 头文件中的注释部分调整，适应Doxygen
 * V1.0.1, 2021/04/28,
 *      1. 简化寄存器控制，因为只有一个从控制器，寄存器地址是确定的，初始化的时候不需要额外指定模块寄存器基地址，寄存器操作全部用宏替换了
 *      2. 头文件中定义的I2C_Slave_State更新说明内容。
 * V1.0.0, 2021/01/22,
 *      1. 初版，I2C从控制器HAL层函数接口，初始化仅配置时钟，不配置IO复用，需要上层应用控制IO复用
 */

/* Includes -----------------------------------------------------------------*/
#include "a0_hal_conf.h"
/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup I2C
 * @{
 */

#ifdef HAL_I2C_SLAVE_MODULE_ENABLED
#include "a0_hal_i2c_slave.h"
#include "a0_hal_def.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Private function prototypes ----------------------------------------------*/
/* Exported functions -------------------------------------------------------*/
/**
 * @defgroup I2C_Slave_Exported_Functions Slave I2C Exported Functions
 * @{
 */
void HalI2cSlaveInit(I2C_S_InitTypeDef *pInit)
{
    I2C_S_CR_TypeDef cr = {
        .b = {
            .rdHoldEn = pInit->sclHoldEn,
            .devAddr = pInit->devAddr,
            .intFlag = 0,
            .intEn = 1,
            .wrAckEn = 1,
            .rdAckEn = 1,
        }
    };
    HAL_I2C_S_CTRL_SET(cr.w);
    NVIC_EnableIRQ(I2C_S_IRQn);
    return;
}

void HalI2cSlaveIrqHandler(I2C_S_InitTypeDef *pInit)
{
    I2C_S_CR_TypeDef cr = {
        .w = HAL_I2C_S_CTRL_GET(),
    };
    if ((cr.b.state == I2C_S_STATE_WDATA) || (cr.b.state == I2C_S_STATE_WDATAACK))
    {
        uint32_t rxData[4] = {
            HAL_I2C_S_RX_DATA_GET(0),
            HAL_I2C_S_RX_DATA_GET(1),
            HAL_I2C_S_RX_DATA_GET(2),
            HAL_I2C_S_RX_DATA_GET(3),
        };
        pInit->pHandle->pFunRxCb((uint8_t *)&rxData[0], cr.b.wrByteNum);
        uint32_t txData[4];
        pInit->pHandle->pFunTxCb((uint8_t *)&txData[0], 0);
        for (uint8_t i = 0; i < 4; i++)
        {
            HAL_I2C_S_TX_DATA_SET(i, txData[i]);
        }
    }
    else if ((cr.b.state == I2C_S_STATE_RDATA) || (cr.b.state == I2C_S_STATE_RDATAACK))
    {
        uint32_t txData[4];
        pInit->pHandle->pFunTxCb((uint8_t *)&txData[0], cr.b.rdByteNum);
        for (uint8_t i = 0; i < 4; i++)
        {
            HAL_I2C_S_TX_DATA_SET(i, txData[i]);
        }
    }
    cr.b.intFlag = 0;
    cr.b.wrAckEn = 1;
    cr.b.rdAckEn = 1;
    HAL_I2C_S_CTRL_SET(cr.w);
    return;
}
/**
 * @}
 */

#endif
/**
 * @}
 */
/**
 * @}
 */
