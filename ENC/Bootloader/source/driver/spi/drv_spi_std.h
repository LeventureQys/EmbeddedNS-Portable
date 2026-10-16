/**
  ******************************************************************************
  * @file    drv_spi_std.h
  * @brief   Header file of DRV_SPI_STD module.
  * @version V1.0.0
  * @date    2021/10/26
  * @author  lijixuan
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/  
#ifndef __DRV_SPI_STD_H
#define __DRV_SPI_STD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "a0_hal_conf.h"
#include "a0_hal_def.h"
#include "a0_hal_gpio.h"
#include "a0_hal_spi_std.h"

/** @addtogroup Drive_SPI_STD_Driver
  * @{
  */

/** @addtogroup UART
  * @{
  */ 

/* Exported types ------------------------------------------------------------*/
/** @defgroup DRV_SPI_STD_Exported_Types SPI Exported Types
  * @{
  */

/**
  * @brief  SPI_STD_Master Configuration Structure the Tx_Buff/Rx_Buff definition
  */
typedef struct{
    int32_t     buffSize;              /*!< Spi RingBuff size          */
    int32_t     wrIdx;                 /*!< Spi RingBuff of head       */
    int32_t     rdIdx;                 /*!< Spi RingBuff of tail       */
    uint8_t*    pBuffStart;            /*!< Spi RingBuff buffer begin  */
}DRV_SPI_BUFF_TypeDef;

/** 
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup SPI_STD_Master_Exported_Constants SPI Exported Constants
  * @{
  */

/** @defgroup Allocate_buffer_space Allocate buffer space for TxBuff/RxBuff
  * @brief Customize the size of the TxBuff/RxBuff
  * @{
  */
#define DRV_SPI0_RX_BUFF_SIZE     256 	
#define DRV_SPI1_RX_BUFF_SIZE     256 
#define DRV_SPI2_RX_BUFF_SIZE     256 
#define DRV_SPI3_RX_BUFF_SIZE     256 
#define DRV_SPI0_TX_BUFF_SIZE     256 
#define DRV_SPI1_TX_BUFF_SIZE     256 
#define DRV_SPI2_TX_BUFF_SIZE     256 
#define DRV_SPI3_TX_BUFF_SIZE     256 
/**
  * @}
  */
  
/** @defgroup SPI_BIT_ItTring_Ctrl SPI interrupt control
  * @brief SPI interrupt trigger
  * @{
  */ 
#define   SPI_IT_RX_TRING_EN_MSK     (0x00000020U)
#define   SPI_IT_TX_TRING_EN_MSK     (0x00000040U)
#define   SPI_IT_TX_RX_TRING_DIS     (0x00000000U)
#define   SPI_IT_RX_TRING_EN         (0x00000020U)               /*!< SPI TX interrupt enable          */
#define   SPI_IT_TX_TRING_EN         (0x00000040U)               /*!< SPI RX interrupt enable          */
/**
  * @}
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup SPI_STD_Master_Exported_Macros SPI Exported Macros
  * @{
  */
  
/** @defgroup SPI_BIT_ItTring_Ctrl SPI interrupt control
  * @brief  SPI IRQ interrupt control
  * @param  SPIx : The ID of UART
  *         This parameter can be one of the these values
  *         @arg 1: SPI1
  *         @arg 2: SPI2
  *         @arg 3: SPI3
  * @{
  */
#define DRV_SPI_INT_CTRL_SET(SPI,ICV_Msk,ICV)         HAL_SPI_FIFO_CTRL_SET(SPI,(((SPI)->FCR & (~ICV_Msk))|ICV))
#define DRV_SPI_TX_IRQ_EN(SPIx)                   {NVIC_ClearPendingIRQ(((IRQn_Type)(SPI1_TX_IRQn + ((SPIx - 1) << 1))));NVIC_EnableIRQ(((IRQn_Type)(SPI1_TX_IRQn + ((SPIx - 1) << 1))));}
#define DRV_SPI_RX_IRQ_EN(SPIx)                   NVIC_EnableIRQ(((IRQn_Type)(SPI1_RX_IRQn + ((SPIx - 1) << 1))))
#define DRV_SPI_TX_IRQ_DIS(SPIx)                  NVIC_DisableIRQ(((IRQn_Type)(SPI1_TX_IRQn + ((SPIx - 1) << 1))))
#define DRV_SPI_RX_IRQ_DIS(SPIx)                  NVIC_DisableIRQ(((IRQn_Type)(SPI1_RX_IRQn + ((SPIx - 1) << 1))))
/**
  * @}
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup SPI_STD_Master_Exported_Functions SPI Drv Exported Functions
  * @{
  */
int32_t DrvSpiExtFlashBusyWait(uint8_t spiIdx,uint8_t rdsrVar);
void DrvSpiTest(void);
int32_t DrvSpiInitStd(SPI_InitTypeDef* pInitSpiStd);
void DrvSpiExtFlashDeviceId(uint8_t spiIdx);
int32_t DrvSpiExtFlashInit(uint8_t spiIdx);
int32_t DrvSpiExtFlashReadData(uint8_t spiIdx,uint8_t* pDataRecive,uint32_t flashAddress,uint32_t length);
int32_t DrvSpiExtFlashPageWrite(uint8_t spiIdx,uint8_t* pDataWrite,uint32_t flashAddress,uint32_t length);
int32_t DrvSpiExtFlashSectorErase(uint8_t spiIdx,uint32_t flashAddress);
int32_t DrvSpiExtFlashOncePageWrite(uint8_t spiIdx,uint8_t* pDataWrite,uint32_t flashAddress,uint32_t length);
int32_t DrvSpiRxNumGet(uint8_t spiIdx);
int32_t DrvSpiTxNumGet(uint8_t spiIdx);
int32_t DrvSpiTxDataWrite(uint8_t spiIdx, uint8_t *pu8Buff, size_t num);
void DrvSpiBuffInit(void);
void DrvSpiTxIrqHandle(uint8_t spiIdx);
int32_t DrvSpiRxDataRead(uint8_t spiIdx, uint8_t *pu8Buff, size_t num);
void DrvSpiRxIrqHandle(uint8_t spiIdx);
void DrvSpiRxData(uint8_t spiIdx, uint8_t *pRxData, size_t byteSize);
void DrvSpiTxData(uint8_t spiIdx,uint8_t *pTxData,size_t byteSize);
int DrvSpiStdMasterInit(SPI_InitTypeDef *pInit);
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
