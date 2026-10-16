/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_spi_std.h
 * @brief   Header file of SPI HAL module.
 * @version V1.0.4
 * @date    2022/04/07
 * @author  huangzheng
 ******************************************************************************
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __A0_HAL_SPI_STD_H
#define __A0_HAL_SPI_STD_H

/* Includes -----------------------------------------------------------------*/
#include "jat_typedef.h"
#include "a0.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup SPI
 * @brief 3 groups Serial Peripheral Interface (SPI) module
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/**
 * @defgroup SPI_Exported_Types SPI Exported Types
 * @{
 */
/**
 * @brief  SPI Configuration Structure definition
 */
typedef struct
{
    uint32_t    spiMode;                    /*!< cpha/cpol mode, 0~3 @ref SPI_MODE          */
    uint32_t    bitDir;                     /*!< from high to low (0), from low to high (1) */
    uint32_t    clkDiv;                     /*!< spi base clock is from PCLK, div >= 8      */
    uint32_t    spiIdx;                     /*!< spi index number to init, 1~3              */
}SPI_InitTypeDef;
/**
 * @}
 */

/* Exported constants -------------------------------------------------------*/
/**
 * @defgroup SPI_Exported_Constants SPI Exported Constants
 * @{
 */
/**
 * @defgroup SPI_MODE SPI Mode
 * @brief SPI mode config value, use for 'mode' in @ref SPI_CR_TypeDef
 * @{
 */
#define SPI_CPHA0CPOL0              (0x0U)  /*!< device get data at start edge(from idle state)(CPHA0), SCK is low level(in idle state)(CPOL0) */
#define SPI_CPHA0CPOL1              (0x1U)  /*!< device get data at start edge(from idle state)(CPHA0), SCK is high level(in idle state)(CPOL1) */
#define SPI_CPHA1CPOL0              (0x2U)  /*!< device get data at end edge(from idle state)(CPHA1), SCK is low level(in idle state)(CPOL0) */
#define SPI_CPHA1CPOL1              (0x3U)  /*!< device get data at end edge(from idle state)(CPHA1), SCK is high level(in idle state)(CPOL1) */
/**
 * @}
 */
/**
 * @defgroup SPI_DATA_WIDTH SPI Data Width
 * @brief SPI data witdh config value, for WIDTH in macro @ref HAL_SPI_FIFO_ACCESS_WIDTH_SET and @ref HAL_SPI_TRANS_WIDTH_SET
 * @{
 */
#define SPI_WIDTH_FIFO_OFFSET       (6)     /*!< spi data width of fifo access offset       */
#define SPI_WIDTH_TRANS_OFFSET      (10)    /*!< spi data width of transfer offset          */
#define SPI_WIDTH_MASK              (0x3U)  /*!< spi data width bit value mask              */
#define SPI_8BIT                    (0x0U)  /*!< spi data width is 8bit                     */
#define SPI_16BIT                   (0x1U)  /*!< spi data width is 16bit                    */
#define SPI_32BIT                   (0x2U)  /*!< spi data width is 32bit                    */
/**
 * @}
 */
/**
 * @defgroup SPI_BYTE_DIR SPI Byte Direction
 * @brief SPI transfer byte direction, for 'byteDir' in @ref SPI_CR_TypeDef
 * @{
 */
#define SPI_BYTE_HIGH2LOW           (0x0U)  /*!< transfer data from high byte to low byte   */
#define SPI_BYTE_LOW2HIGH           (0x1U)  /*!< transfer data from low byte to high byte   */
/**
 * @}
 */
/**
 * @defgroup SPI_RX_FIFO_TRIG SPI Rx Fifo Trigger
 * @brief SPI rx fifo trigger config value, for 'rxFifoTrigCfg' in @ref SPI_FCR_TypeDef
 * @{
 */
#define SPI_RX_FIFO_NE0             (0x0U)  /*!< spi rx fifo data number != 0               */
#define SPI_RX_FIFO_BE4             (0x1U)  /*!< spi rx fifo data number >= 4               */
#define SPI_RX_FIFO_BE8             (0x2U)  /*!< spi rx fifo data number >= 8               */
#define SPI_RX_FIFO_E16             (0x3U)  /*!< spi rx fifo data number == 16              */
/**
 * @}
 */
/**
 * @defgroup SPI_TX_FIFO_TRIG SPI Tx Fifo Trigger
 * @brief SPI tx fifo trigger config value, for 'txFifoTrigCfg' in @ref SPI_FCR_TypeDef
 * @{
 */
#define SPI_TX_FIFO_E0              (0x0U)  /*!< spi tx fifo data number == 0               */
#define SPI_TX_FIFO_SE4             (0x1U)  /*!< spi tx fifo data number <= 4               */
#define SPI_TX_FIFO_SE8             (0x2U)  /*!< spi tx fifo data number <= 8               */
#define SPI_TX_FIFO_SE16            (0x3U)  /*!< spi tx fifo data number <= 16              */
/**
 * @}
 */
/**
 * @defgroup SPI_Operation_Mode SPI Operation Mode
 * @brief SPI operation mode config value, for OP in macro @ref HAL_SPI_OP_MODE_SET
 * @{
 */
#define SPI_OP_BIT_OFFSET           (7)     /*!< spi operate mode bit offset in register    */
#define SPI_OP_MASK                 (0x3U)  /*!< spi operate mode bit mask value            */
#define SPI_OP_NONE                 (0x0U)  /*!< spi operate mode is no tx no rx            */
#define SPI_OP_TX                   (0x1U)  /*!< spi operate mode is only tx                */
#define SPI_OP_RX                   (0x2U)  /*!< spi operate mode is only rx                */
#define SPI_OP_TXRX                 (0x3U)  /*!< spi operate mode is tx and rx              */
/**
 * @}
 */

/**
 * @defgroup SPI_DMA_Mode SPI DMA Mode
 * @brief SPI DMA mode config value, for 'dmaMode' in @ref SPI_CR_TypeDef
 * @{
 */
#define SPI_DMA_UNUSED              (0x0U)  /*!< Dma unused                                 */
#define SPI_DMA_RX                  (0x1U)  /*!< Dma use for rx                             */
#define SPI_DMA_TX                  (0x2U)  /*!< Dma use for tx                             */
#define SPI_DMA_TXRX                (0x3U)  /*!< Dma use for tx and rx                      */
/**
 * @}
 */
/**
 * @}
 */

/* Exported macros ----------------------------------------------------------*/
/**
 * @defgroup SPI_Exported_Macros SPI Exported Macros
 * @{
 */
/**
 * @brief  Spi ctrl value set, write register 'CR' in @ref SPI_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @param  CV      Ctrl value write to register
 * @return None
 */
#define HAL_SPI_CTRL_SET(SPI, CV)                                       ((SPI)->CR = (CV))

/**
 * @brief  Spi ctrl value get, read register 'CR' in @ref SPI_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @return Ctrl value read from register
 */
#define HAL_SPI_CTRL_GET(SPI)                                           ((SPI)->CR)

/**
 * @brief  Spi fifo access width set, write 'fifoAccWidth' in register 'CR', @ref SPI_CR_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @param  WIDTH   Spi fifo access width, 8Bit/16Bit/32Bit, @ref SPI_DATA_WIDTH
 * @return None
 */
#define HAL_SPI_FIFO_ACCESS_WIDTH_SET(SPI, WIDTH)                       \
((SPI)->CR = (((SPI)->CR) & (~(SPI_WIDTH_MASK << SPI_WIDTH_FIFO_OFFSET)) | ((WIDTH) << SPI_WIDTH_FIFO_OFFSET)))

/**
 * @brief  Spi transfer width set, write 'transWidth' in register 'CR', @ref SPI_CR_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @param  WIDTH   Spi transfer width, 8Bit/16Bit/32Bit, @ref SPI_DATA_WIDTH
 * @return None
 */
#define HAL_SPI_TRANS_WIDTH_SET(SPI, WIDTH)                             \
((SPI)->CR = (((SPI)->CR) & (~(SPI_WIDTH_MASK << SPI_WIDTH_TRANS_OFFSET)) | ((WIDTH) << SPI_WIDTH_TRANS_OFFSET)))

/**
 * @brief  Spi cs output high level in master mode, set bit 'csCtrl' in register 'CR', @ref SPI_CR_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_SPI_CS_SET(SPI)                                             ((SPI)->CR |= ( BIT12))

/**
 * @brief  Spi cs output low level in master mode, clr bit 'csCtrl' in register 'CR', @ref SPI_CR_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_SPI_CS_CLR(SPI)                                             ((SPI)->CR &= (~BIT12))

/**
 * @brief  Spi module enable, set bit 'enable' in register 'CR', @ref SPI_CR_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_SPI_ENABLE(SPI)                                             ((SPI)->CR |= ( BIT4))

/**
 * @brief  Spi module disable, clr bit 'enable' in register 'CR', @ref SPI_CR_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_SPI_DISABLE(SPI)                                            ((SPI)->CR &= (~BIT4))

/**
 * @brief  Spi fifo ctrl value set, write register 'FCR' in @ref SPI_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @param  FCV     Fifo ctrl value write to register
 * @return None
 */
#define HAL_SPI_FIFO_CTRL_SET(SPI, FCV)                                 ((SPI)->FCR = (FCV))

/**
 * @brief  Spi fifo ctrl value set, read register 'FCR' in @ref SPI_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @return Fifo ctrl value read from register
 */
#define HAL_SPI_FIFO_CTRL_GET(SPI)                                      ((SPI)->FCR)

/**
 * @brief  Spi operation mode set, write 'rwMode' in register 'FCR', @ref SPI_FCR_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @param  OP      Operation mode, @ref SPI_Operation_Mode
 * @return None
 */
#define HAL_SPI_OP_MODE_SET(SPI, OP)                                    \
((SPI)->FCR = (((SPI)->FCR) & (~(SPI_OP_MASK << SPI_OP_BIT_OFFSET)) | ((OP) << SPI_OP_BIT_OFFSET)))

/**
 * @brief  Spi fifo reset, set bit 'fifoRst' in register 'FCR', @ref SPI_FCR_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_SPI_FIFO_RST(SPI)                                           ((SPI)->FCR |= ( BIT9))

/**
 * @brief  Spi transfer number set, write register 'TSR' in @ref SPI_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @param  VAL     Transfer number value write to register
 * @return None
 */
#define HAL_SPI_TRANS_NUM_SET(SPI, VAL)                                 ((SPI)->TSR = (VAL))

/**
 * @brief  Spi transfer number get, read register 'TSR' in @ref SPI_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @return Transfer number value read from register
 */
#define HAL_SPI_TRANS_NUM_GET(SPI)                                      ((SPI)->TSR)

/**
 * @brief  Spi data write, write register 'DR' in @ref SPI_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @param  DATA    Data write to register
 * @return None
 */
#define HAL_SPI_DATA_WRITE(SPI, DATA)                                   ((SPI)->DR = (DATA))

/**
 * @brief  Spi data read, read register 'DR' in @ref SPI_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @return Data read from register
 */
#define HAL_SPI_DATA_READ(SPI)                                          ((SPI)->DR)

/**
 * @brief  Spi rx timeout value write, write register 'RTOR' in @ref SPI_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @param  TOV     Rx time out value write to register
 * @return None
 */
#define HAL_SPI_TIMEOUT_SET(SPI, TOV)                                   ((SPI)->RTOR = (TOV))

/**
 * @brief  Spi rx timeout value read, read register 'RTOR' in @ref SPI_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @return Rx time out value read from register
 */
#define HAL_SPI_TIMEOUT_GET(SPI)                                        ((SPI)->RTOR)

/**
 * @brief  Spi state read, read register 'SR' in @ref SPI_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @return State value read from register
 */
#define HAL_SPI_STATE_GET(SPI)                                          ((SPI)->SR)

/**
 * @brief  Spi data number in rx fifo get, read 'rxFifoCnt' in register 'SR', @ref SPI_SR_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @return Rx count
 */
#define HAL_SPI_RX_CNT_GET(SPI)                                         (((SPI)->SR >> 0) & 0x1F)

/**
 * @brief  Spi data number in tx fifo get, read 'txFifoCnt' in register 'SR', @ref SPI_SR_TypeDef
 * @param  SPI     Spi instance, SPI1/SPI2/SPI3, @ref Peripheral_Declaration
 * @return Tx count
 */
#define HAL_SPI_TX_CNT_GET(SPI)                                         (((SPI)->SR >> 8) & 0x1F)
/**
 * @}
 */

/* Exported functions -------------------------------------------------------*/
/**
 * @addtogroup SPI_Exported_Functions SPI Exported Functions
 * @{
 */
/**
 * @brief  spi module initial process function
 * @param  pInit       Pointer to structure of @ref SPI_InitTypeDef
 * @retval == 0        Initial OK
 * @retval == -1       spi index number is out of range, it must be 1~3
 */
int32_t HalSpiStdInit(SPI_InitTypeDef *pInit);

/**
 * @brief  spi module initial process function
 * @param  spiIdx      Spi index number, 1~3
 * @param  pTxData     Pointer to tx source memory
 * @param  byteSize    Byte size need to write to tx fifo
 * @return None
 */
void HalSpiTxData(uint8_t spiIdx, uint8_t *pTxData, size_t byteSize);

/**
 * @brief  spi module initial process function
 * @param  spiIdx      Spi index number, 1~3
 * @param  pRxData     Pointer to rx destination memory
 * @param  byteSize    Byte size need to read from rx fifo
 * @return None
 */
void HalSpiRxData(uint8_t spiIdx, uint8_t *pRxData, size_t byteSize);

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
