/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_i2c_master.c
 * @brief   I2C MASTER HAL module.
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
 *      1. 简化寄存器控制，因为只有一个主控制器，寄存器地址是确定的，初始化的时候不需要额外指定模块寄存器基地址，寄存器操作全部用宏替换
 *      2. 由于M4F自带Tick计数器，统一使用该计数器作为毫秒计数，初始化的时候不需要额外指定毫秒计数相关函数
 * V1.0.0, 2021/01/21,
 *      1. 初版，I2C主控制器HAL层函数接口，初始化仅配置时钟，不配置IO复用，需要上层应用控制IO复用
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

#ifdef HAL_I2C_MASTER_MODULE_ENABLED
#include "a0_hal_i2c_master.h"
#include "a0_hal_def.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
#define bI2CNack          (0x00000001UL)
#define bI2CWrite         (0x00000002UL)
#define bI2CRead          (0x00000004UL)
#define bI2CRestart       (0x00000008UL)
#define bI2CStop          (0x00000010UL)
#define bI2CStart         (0x00000020UL)
/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Private function prototypes ----------------------------------------------*/
static int32_t HalI2cMasterCheckBusy(uint32_t timeOutMs)
{
    uint32_t t0 = HalI2cMasterTickGet();
    while(HAL_I2C_M_BUSY_CHECK() != 0)
    {
        if ((HalI2cMasterTickGet() - t0) > timeOutMs)
        {
            return I2C_M_ERROR_TIMEOUT;
        }
    }
    return I2C_M_ERROR_NONE;
}
static int32_t HalI2cMasterCheckTip(uint32_t timeOutMs)
{
    uint32_t t0 = HalI2cMasterTickGet();
    //check tip
    while(HAL_I2C_M_TIP_CHECK() != 0)
    {
        if ((HalI2cMasterTickGet() - t0) > timeOutMs)
        {
            return I2C_M_ERROR_TIMEOUT;
        }
    }
    return I2C_M_ERROR_NONE;
}
static int32_t HalI2cMasterCheckTipAck(uint32_t timeOutMs)
{
    uint32_t t0 = HalI2cMasterTickGet();
    //check tip
    while(HAL_I2C_M_TIP_CHECK() != 0)
    {
        if ((HalI2cMasterTickGet() - t0) > timeOutMs)
        {
            return I2C_M_ERROR_TIMEOUT;
        }
    }
    //check ack
    if (HAL_I2C_M_ACK_CHECK() == 0)
    {
        return I2C_M_ERROR_NACK;
    }
    return I2C_M_ERROR_NONE;
}

/* Exported functions -------------------------------------------------------*/
/**
 * @defgroup I2C_Master_Exported_Functions I2C Master Exported Functions
 * @{
 */
int32_t HalI2cMasterInit(I2C_M_InitTypeDef *pInit)
{
    int32_t t1 = pInit->speed * 5;
    int32_t t2 = ((int32_t)SystemPclk + (t1 >> 1)) / t1 - 1;
    int32_t retVal;
    if (t2 > 0)
    {
        HAL_I2C_M_CLOCK_SET(t2);
        retVal = I2C_M_ERROR_NONE;
    }
    else
    {
        retVal = I2C_M_ERROR_INIT;
    }
    return retVal;
}

//mode: 0, normal, end with stop
//mode: 1, for read eeprom, do not send stop with correct write
int32_t HalI2cMasterWrite(I2C_M_InitTypeDef *pInit, uint32_t handleIdx, uint8_t mode)
{
    I2C_M_HandleTypeDef *pHandle = pInit->pHandle;

    int32_t retVal;
    if (handleIdx < pInit->handleNumber)
    {
        pHandle += handleIdx;
        if (pHandle->len != 0)
        {
            retVal = HalI2cMasterCheckBusy(pHandle->timeOutMs);
            if (retVal == I2C_M_ERROR_NONE)
            {
                //send start and address
                HAL_I2C_M_TX_DATA_WR(pHandle->devAddr << 1);
                HAL_I2C_M_CTRL_SET(bI2CStart|bI2CWrite);
                retVal = HalI2cMasterCheckTipAck(pHandle->timeOutMs);
                if (retVal == I2C_M_ERROR_NONE)
                {
                    //send data
                    while(pHandle->len)
                    {
                        pHandle->len--;
                        HAL_I2C_M_TX_DATA_WR(*pHandle->pWrData++);
                        HAL_I2C_M_CTRL_SET(bI2CWrite);
                        retVal = HalI2cMasterCheckTipAck(pHandle->timeOutMs);
                        if (retVal != I2C_M_ERROR_NONE)
                        {
                            //send stop
                            pHandle->len++;
                            break;
                        }
                    }
                }
                if (mode == I2C_M_WR_MODE_NORMAL)
                {
                    //send stop
                    HAL_I2C_M_CTRL_SET(bI2CStop);
                    if (pHandle->len == 0)
                    {
                        retVal = HalI2cMasterCheckTip(pHandle->timeOutMs);
                    }
                }
                else //if (mode == HAL_I2C_M_WR_MODE_EEPROM)
                {
                    //only error, send stop
                    if (pHandle->len != 0)
                    {
                        HAL_I2C_M_CTRL_SET(bI2CStop);
                    }
                }
            }
        }
        else
        {
            retVal = I2C_M_ERROR_LENZERO;
        }
    }
    else
    {
        retVal = I2C_M_ERROR_HANDLE;
    }
    if (retVal != 0)
    {
        DBG_PRINT("I2C master write error! Error code is: %d\n", retVal);
    }
    return retVal;
}

//mode: 0, normal read, check idle first, begin with start
//mode: 1, restart read, do not check idle, begin with restart
int32_t HalI2cMasterRead(I2C_M_InitTypeDef *pInit, uint32_t handleIdx, uint8_t mode)
{
    I2C_M_HandleTypeDef *pHandle = pInit->pHandle;

    int32_t retVal;
    if (handleIdx < pInit->handleNumber)
    {
        pHandle += handleIdx;
        if (pHandle->len != 0)
        {
            uint32_t firstSend;
            if (mode == I2C_M_RD_MODE_NORMAL)
            {
                retVal = HalI2cMasterCheckBusy(pHandle->timeOutMs);
                firstSend = (bI2CStart|bI2CWrite);
            }
            else// if (mode == HAL_I2C_M_RD_MODE_EEPROM)
            {
                retVal = I2C_M_ERROR_NONE;
                firstSend = (bI2CRestart|bI2CWrite);
            }
            if (retVal == I2C_M_ERROR_NONE)
            {
                //send start and address
                HAL_I2C_M_TX_DATA_WR((pHandle->devAddr << 1) | 0x1);
                HAL_I2C_M_CTRL_SET(firstSend);
                retVal = HalI2cMasterCheckTipAck(pHandle->timeOutMs);
                if (retVal == I2C_M_ERROR_NONE)
                {
                    //get data
                    while(pHandle->len > 1)
                    {
                        HAL_I2C_M_CTRL_SET(bI2CRead);
                        retVal = HalI2cMasterCheckTip(pHandle->timeOutMs);
                        if (retVal != I2C_M_ERROR_NONE)
                        {
                            break;
                        }
                        *pHandle->pRdData++ = HAL_I2C_M_RX_DATA_RD();
                        pHandle->len--;
                    }
                    if (pHandle->len == 1)
                    {
                        //last byte
                        HAL_I2C_M_CTRL_SET(bI2CRead|bI2CNack);
                        retVal = HalI2cMasterCheckTip(pHandle->timeOutMs);
                        if (retVal == I2C_M_ERROR_NONE)
                        {
                            *pHandle->pRdData++ = HAL_I2C_M_RX_DATA_RD();
                            pHandle->len--;
                        }
                    }
                }
                //send stop
                HAL_I2C_M_CTRL_SET(bI2CStop);
                if (pHandle->len == 0)
                {
                    retVal = HalI2cMasterCheckTip(pHandle->timeOutMs);
                }
            }
        }
        else
        {
            retVal = I2C_M_ERROR_LENZERO;
        }
    }
    else
    {
        retVal = I2C_M_ERROR_HANDLE;
    }
    if (retVal != 0)
    {
        DBG_PRINT("I2C master read error! Error code is: %d\n", retVal);
    }
    return retVal;
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
