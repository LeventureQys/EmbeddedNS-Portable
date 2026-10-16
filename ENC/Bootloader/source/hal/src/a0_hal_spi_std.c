/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_spi_std.c
 * @brief   SPI HAL module.
 * @version V1.0.4
 * @date    2022/04/07
 * @author  huangzheng
 ******************************************************************************
 */

/**
 * 版本日志：
 * V1.0.4, 2022/04/07
 *      1. 函数命名按照大驼峰方式命名，头文件增加原函数名的define，为了兼容原代码，后续希望渐渐过渡到统一风格书写规范
 *      2. 整理头文件中依赖关系，使用jat_typedef.h作为数据类型的头文件，并把include放到extern "C" 之外
 *      3. 将一些配置选项移到a0_hal_conf.h，统一进行配置
 * V1.0.3, 2021/10/11,
 *      1. HalSpiTxData, HalSpiRxData, 这2个函数中原来控制CS拉低和拉高的去掉，让调用这个函数的地方去进行CS控制
 * V1.0.2, 2021/09/22,
 *      1. 原来头文件中GROUP SPI的描述有误，实际上是3组，更新描述
 * V1.0.1, 2021/09/15,
 *      1. 头文件中的注释部分调整，适应Doxygen
 *      2. 增加部分宏接口，用于读写寄存器相关位
 * V1.0.0, 2021/05/26,
 *      1. 初版，单线SPI控制器HAL层函数接口，不配置IO复用，需要上层应用控制IO复用
 *      2. 主模式下，TX/RX数据采用查询，PIO方式进行读写，DMA和中断方式以及从模式参考a0_test_spi.c
 */

/* Includes -----------------------------------------------------------------*/
#include "a0_hal_conf.h"
/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup SPI
 * @{
 */

#ifdef HAL_SPI_STD_MODULE_ENABLED
#include "a0_hal_spi_std.h"
#include "a0_hal_def.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Private function prototypes ----------------------------------------------*/
/* Exported functions -------------------------------------------------------*/

/**
 * @defgroup SPI_Exported_Functions SPI Exported Functions
 * @{
 */
int32_t HalSpiStdInit(SPI_InitTypeDef *pInit)
{
    int32_t result;
    if ((pInit->spiIdx >= 1) && (pInit->spiIdx <= 3))
    {
        SPI_CR_TypeDef cr = {
            .b = {
                .mode = pInit->spiMode,
                .bitDir = pInit->bitDir,
                .enable = 1,
                .byteDir = SPI_BYTE_LOW2HIGH,       /* default, little endian */
                .clkDiv = pInit->clkDiv,
            },
        };
        HAL_SPI_CTRL_SET(SPIN(pInit->spiIdx), cr.w);
        result = 0;
    }
    else
    {
        result = -1;
    }
  return result;
}

//spiIdx: 1~3
void HalSpiTxData(uint8_t spiIdx, uint8_t *pTxData, size_t byteSize)
{
    SPI_TypeDef *SPIx = SPIN(spiIdx);
    SPI_SR_TypeDef  sr;

    HAL_SPI_OP_MODE_SET(SPIx, SPI_OP_TX);
    HAL_SPI_FIFO_RST(SPIx);
    uint32_t wordSize = byteSize >> 2;
    byteSize &= 0x3;
    if (wordSize != 0)
    {
        __packed uint32_t *pu32TxData = (__packed uint32_t *)pTxData;
        /* 当TX数据大于等于4字节时，先按照32Bit进行传输 */
        HAL_SPI_FIFO_ACCESS_WIDTH_SET(SPIx, SPI_32BIT);
        HAL_SPI_TRANS_WIDTH_SET(SPIx, SPI_32BIT);

        HAL_SPI_TRANS_NUM_SET(SPIx, wordSize);
        while (wordSize != 0)
        {
            sr.w = HAL_SPI_STATE_GET(SPIx);
            if (sr.b.txFifoCnt <= 12)
            {
                HAL_SPI_DATA_WRITE(SPIx, *pu32TxData++);
                wordSize--;
            }
        }
        pTxData = (uint8_t *)pu32TxData;
    }
    if (byteSize != 0)
    {
        /* 剩下数据按照字节传输· */
        HAL_SPI_FIFO_ACCESS_WIDTH_SET(SPIx, SPI_8BIT);
        HAL_SPI_TRANS_WIDTH_SET(SPIx, SPI_8BIT);

        HAL_SPI_TRANS_NUM_SET(SPIx, byteSize);
        while (byteSize != 0)
        {
            sr.w = HAL_SPI_STATE_GET(SPIx);
            if (sr.b.txFifoCnt <= 15)
            {
                HAL_SPI_DATA_WRITE(SPIx, *pTxData++);
                byteSize--;
            }
        }
    }
    while (HAL_SPI_TRANS_NUM_GET(SPIx) != 0)
    {
        ;
    }
    HAL_SPI_OP_MODE_SET(SPIx, SPI_OP_NONE);
  return;
}

//spiIdx: 1~3
void HalSpiRxData(uint8_t spiIdx, uint8_t *pRxData, size_t byteSize)
{
    SPI_TypeDef *SPIx = SPIN(spiIdx);
    SPI_SR_TypeDef  sr;

    HAL_SPI_OP_MODE_SET(SPIx, SPI_OP_RX);
  HAL_SPI_FIFO_RST(SPIx);
    uint32_t wordSize = byteSize >> 2;
    byteSize &= 0x3;
    if (wordSize != 0)
    {
        __packed uint32_t *pu32RxData = (__packed uint32_t *)pRxData;
        /* 当RX数据大于等于4字节时，先按照32Bit进行传输 */
        HAL_SPI_FIFO_ACCESS_WIDTH_SET(SPIx, SPI_32BIT);
        HAL_SPI_TRANS_WIDTH_SET(SPIx, SPI_32BIT);

        HAL_SPI_TRANS_NUM_SET(SPIx, wordSize);
        while (wordSize != 0)
        {
            sr.w = HAL_SPI_STATE_GET(SPIx);
            if (sr.b.rxFifoCnt >= 4)
            {
                *pu32RxData++ = HAL_SPI_DATA_READ(SPIx);
                wordSize--;
            }
        }
        pRxData = (uint8_t *)pu32RxData;
    }
    if (byteSize != 0)
    {
        /* 剩下数据按照字节传输· */
        HAL_SPI_FIFO_ACCESS_WIDTH_SET(SPIx, SPI_8BIT);
        HAL_SPI_TRANS_WIDTH_SET(SPIx, SPI_8BIT);

        HAL_SPI_TRANS_NUM_SET(SPIx, byteSize);
        while (byteSize != 0)
        {
            sr.w = HAL_SPI_STATE_GET(SPIx);
            if (sr.b.rxFifoCnt != 0)
            {
                *pRxData++ = HAL_SPI_DATA_READ(SPIx);
                byteSize--;
            }
        }
    }
    HAL_SPI_OP_MODE_SET(SPIx, SPI_OP_NONE);
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
