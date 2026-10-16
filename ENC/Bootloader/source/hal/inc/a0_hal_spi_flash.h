/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_spi_flash.h
 * @brief   Header file of SPI FLASH HAL module.
 * @version V1.0.6
 * @date    2024/03/27
 * @author  huangzheng
 ******************************************************************************
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __A0_HAL_SPI_FLASH_H
#define __A0_HAL_SPI_FLASH_H

/* Includes -----------------------------------------------------------------*/
#include "jat_typedef.h"
#include "a0.h"
#include "a0_hal_conf.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup SPI_FLASH
 * @brief Serial Peripheral Interface (SPI) flash access controller module
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/**
 * @defgroup SPI_Flash_Exported_Types SPI Flash Exported Types
 * @{
 */
/**
 * @brief  SPI Flash Configuration Structure definition
 */
typedef struct
{
    SPIFC_CMDR_TypeDef  cmdReg;
    uint32_t            timeout;
    uint8_t             *pRead;             /*!< for RDSR, if cmdReg->b.dataCounter == 0, this pointer will be ignored */
    uint8_t             *pWrite;            /*!< for WRSR, PP, QPP, if cmdReg->b.dataCounter == 0, this pointer will be ignored */
}SPIFC_CmdTypeDef;
/**
 * @}
 */

/* Exported constants -------------------------------------------------------*/
/**
 * @defgroup SPI_Flash_Exported_Constants SPI Flash Exported Constants
 * @{
 */
#define SPIF_PAGE_SIZE                      (256)   /* page write size is 256 bytes */
/**
 * @}
 */

/* Exported macros ----------------------------------------------------------*/
/**
 * @defgroup SPI_Flash_Exported_Macros SPI Flash Exported Macros
 * @{
 */
/**
 * @brief  spi flash controller cache clean, set bit10 in register 'XIPCR' of @ref SPIFC_TypeDef
 * @return None
 */
#define HAL_SPIFC_CACHE_CLEAN()                                         (SPIFC->XIPCR |= (BIT10))

/**
 * @brief  spi flash controller cache disable, set bit3 in register 'XIPCR' of @ref SPIFC_TypeDef
 * @return None
 */
#define HAL_SPIFC_CACHE_DISABLE()                                       (SPIFC->XIPCR |= (BIT3))

/**
 * @brief  spi flash controller cache clean and cache enable with sram latency setting 0 (latching the data at 1st cycle after address cycle)
 * @return None
 */
#define HAL_SPIFC_CACHE_EN()                                            (SPIFC->XIPCR = (BIT10))

/**
 * @brief  spi flash controller cache range set, write register 'CRANGE' of @ref SPIFC_TypeDef
 * @param  RANGE   cache range from address 0 to address RANGE in byte
 * @return None
 */
#define HAL_SPIFC_CACHE_RANGE_SET(RANGE)                                (SPIFC->CRANGE = ((RANGE) >> 3))
/**
 * @}
 */

/* Private macros -----------------------------------------------------------*/
/**
 * @defgroup SPI_Flash_Private_Macros SPI Flash Private Macros
 * @{
 */
/**
 * @brief  spi flash controller cmd complete state flag check, read bit0 in registere 'ISR' of @ref SPIFC_TypeDef
 * @retval == 0    cmd not complete
 * @retval != 0    cmd complete
 */
#define HAL_SPIFC_CMD_COMPLETE_CHECK()                                  (SPIFC->ISR & (BIT0))
/**
 * @brief  spi flash controller cmd complete state flag clear, set bit0 in registere 'ISR' of @ref SPIFC_TypeDef
 * @return None
 */
#define HAL_SPIFC_CMD_COMPLETE_CLR()                                    (SPIFC->ISR |= (BIT0))
/**
 * @brief  spi flash controller tx fifo empty state flag check, read bit0 in registere 'SR' of @ref SPIFC_TypeDef
 * @retval == 0    tx fifo is not ready
 * @retval != 0    tx fifo is ready to write
 */
#define HAL_SPIFC_TX_READY_CHECK()                                      (SPIFC->SR & (BIT0))
/**
 * @brief  spi flash controller rx fifo ready state flag check, read bit1 in registere 'SR' of @ref SPIFC_TypeDef
 * @retval == 0    rx fifo is not ready
 * @retval != 0    rx fifo is ready to read
 */
#define HAL_SPIFC_RX_READY_CHECK()                                      (SPIFC->SR & (BIT1))
/**
 * @brief  spi flash controller tx fifo write data 32bit, write 32bit data to the register in address 'SPIFC_DR_BASE'
 * @param  DATA    32bit data write to spi flash controller tx fifo
 * @return None
 */
#define HAL_SPIFC_FIFO_U32WR(DATA)                                      (U32REG(SPIFC_DR_BASE) = (DATA))
/**
 * @brief  spi flash controller tx fifo write data 16bit, write 16bit data to the register in address 'SPIFC_DR_BASE'
 * @param  DATA    16bit data write to spi flash controller tx fifo
 * @return None
 */
#define HAL_SPIFC_FIFO_U16WR(DATA)                                      (U16REG(SPIFC_DR_BASE) = (DATA))
/**
 * @brief  spi flash controller tx fifo write data 8bit, write 8bit data to the register in address 'SPIFC_DR_BASE'
 * @param  DATA    8bit data write to spi flash controller tx fifo
 * @return None
 */
#define HAL_SPIFC_FIFO_U8WR(DATA)                                       (U8REG(SPIFC_DR_BASE) = (DATA))
/**
 * @brief  spi flash controller rx fifo read data 32bit, read 32bit data from the register in address 'SPIFC_DR_BASE'
 * @return 32bit data read from spi flash controller rx fifo
 */
#define HAL_SPIFC_FIFO_U32RD()                                          (U32REG(SPIFC_DR_BASE))
/**
 * @brief  spi flash controller rx fifo read data 16bit, read 16bit data from the register in address 'SPIFC_DR_BASE'
 * @return 16bit data read from spi flash controller rx fifo
 */
#define HAL_SPIFC_FIFO_U16RD()                                          (U16REG(SPIFC_DR_BASE))
/**
 * @brief  spi flash controller rx fifo read data 8bit, read 8bit data from the register in address 'SPIFC_DR_BASE'
 * @return 8bit data read from spi flash controller rx fifo
 */
#define HAL_SPIFC_FIFO_U8RD()                                           (U8REG(SPIFC_DR_BASE))
/**
 * @}
 */

/* Exported functions -------------------------------------------------------*/
/**
 * @defgroup SPI_Flash_Exported_Functions SPI Flash Exported Functions
 * @{
 */
/**
 * @brief  spi flash erase operation
 * @param  argv    pointer to 3 input parameter, i.e. [FlashOffset, DataByteCnt, Type]
    | Index of argv | Name        | Detail                                                       |
    | ------------- | ----------- | ------------------------------------------------------------ |
    | 0             | FlashOffset | erase start address, if the address is not the start of this page or sector, the whole page or sector will be erased |
    | 1             | DataByteCnt | erase byte size, if the end address is not the end of this page or sector, the whole page or sector will be erased |
    | 2             | Type        | erase type select<br/>0: page(256Byte)<br/>1: sector(4KByte)<br/>2: block 32KByte<br/>3: block 64KByte<br/>4: chip |
 * @retval == 0    flash erase ok
 * @retval != 0    flash erase error
 */
int32_t HalFlashErase(uint32_t *argv);

/**
 * @brief  spi flash program operation
 * @param  argv    pointer to 3 input parameter, i.e. [FlashOffset, DataByteCnt, WriteDataPt]
    | Index of argv | Name        | Detail                                     |
    | ------------- | ----------- | ------------------------------------------ |
    | 0             | FlashOffset | program start address                      |
    | 1             | DataByteCnt | program byte size                          |
    | 2             | WriteDataPt | pointer to data which need to be programed |
 * @retval == 0    flash erase ok
 * @retval != 0    flash erase error
 */
int32_t HalFlashWrite(uint32_t *argv);

/**
 * @brief  internal spi flash init, config clk and spi mode of controller, software reset flash, quad enable, enter continue read in XIP
 * @retval == 0    init ok
 * @retval == -1   reset error
 * @retval == -2   quad enable error
 */
int32_t HalFlashInit(void);

/* for code compatible */
#define hal_flash_erase                         HalFlashErase
#define hal_flash_write                         HalFlashWrite
#define hal_flash_init                          HalFlashInit

#ifdef EXT_FLASH_SUPPORT
/**
 * @brief  internal spi flash init, config clk and spi mode of controller, software reset flash, quad enable, enter continue read in XIP
 * @param  pCmdPara    pointer to the structure of cmd data to sent @ref SPIFC_CmdTypeDef
 * @retval == 0        cmd send successed
 * @retval != 0        cmd send error
 */
int32_t HalFlashCmdSend(SPIFC_CmdTypeDef *pCmdPara);
/* for code compatible */
#define hal_flash_cmd_send                      HalFlashCmdSend
#endif
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup HAL_WEAK_Functions HAL WEAK Functions
 * @brief  Weak functions in HAL. These functions can be rewritten in user code.
 * @{
 */
/**
 * @addtogroup HAL_WEAK_Functions_SPIFC Weak Functions of SPIFC
 * @brief Weak functions in a0_hal_spi_flash.c
 * @{
 */
#ifdef EXT_FLASH_SUPPORT
/**
 * @brief  spi flash erase data in cmd mode
 * @param  type        erase cmd select, 0 for PE(0x81), 1 for SE(0x20), 2 for BE32(0x52), 3 for BE64(0xD8), 4 for CE(0x60)
 * @param  offset      erase start address in flash
 * @param  len         byte number need to erase
 * @retval == 1        this function need to be rewrite in DRV
 * @retval == 0        flash erase ok
 * @retval < 0         flash erase error
 */
int32_t HalExtFlashErase(uint8_t type, uint32_t offset, uint32_t len);

/**
 * @brief  spi flash write data in cmd mode
 * @param  pu8Data     pointer to the memery of data to write to flash
 * @param  offset      write start address in flash
 * @param  len         byte number need to program
 * @retval == 1        this function need to be rewrite in DRV
 * @retval == 0        flash write ok
 * @retval < 0         flash write error
 */
int32_t HalExtFlashWrite(uint8_t *pu8Data, uint32_t offset, uint32_t len);

/**
 * @brief  external spi flash init, software reset flash, quad enable, enter continue read in XIP
 * @retval == 1    this function need to be rewrite in DRV
 * @retval == 0    init ok
 * @retval == -1   reset error
 * @retval == -2   quad enable error
 * @retval == -3   enter continue read error
 */
int32_t HalExtFlashInit(void);
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

#ifdef __cplusplus
}
#endif

#endif
