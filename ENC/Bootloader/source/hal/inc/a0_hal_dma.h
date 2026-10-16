/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_dma.h
 * @brief   Header file of DMA HAL module.
 * @version V1.0.4
 * @date    2023/12/01
 * @author  huangzheng
 ******************************************************************************
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __A0_HAL_DMA_H
#define __A0_HAL_DMA_H

/* Includes ------------------------------------------------------------------*/
#include "jat_typedef.h"
#include "a0.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup DMA
 * @brief  2 groups (16 channels in all) Direct Memory Access (DMA) modlue
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/**
 * @defgroup DMA_Exported_Constants DMA Exported Constants
 * @{
 */
#define DMA_CH_MAX                          (16)    /*!< DMA max channel number, 8 channel in group0, 8 channel in group1 */

#define DMA_DIR_REG2MEM                     (0)     /*!< DMA direction register to memory index value */
#define DMA_DIR_MEM2REG                     (1)     /*!< DMA direction memory to register index value */

/**
 * @defgroup DMA_Address_Ctrl DMA Address Ctrl Config
 * @brief DMA address control, value of 'dstCtrl' and 'srcCtrl' in LLD @ref DMA_LLD_TypeDef and 'CCR' @ref DMA_CH_CCR_TypeDef
 * @{
 */
#define DMA_ADDR_INC                        (0)     /*!< DMA address ctrl increment */
#define DMA_ADDR_DEC                        (1)     /*!< DMA address ctrl decrement */
#define DMA_ADDR_FIXED                      (2)     /*!< DMA address ctrl fixed     */
/**
 * @}
 */

/**
 * @defgroup DMA_MODE DMA Mode Config
 * @brief DMA working mode, value of 'mode' in 'CCR' @ref DMA_CH_CCR_TypeDef
 * @{
 */
#define DMA_MODE_NORMAL                     (0)     /*!< DMA transfer in normal mode */
#define DMA_MODE_HARDWARE                   (1)     /*!< DMA transfer in hardware handshake mode */
/**
 * @}
 */

/**
 * @defgroup DMA_DATA_WIDTH DMA Data Width Config
 * @brief DMA data width, value of 'dstWidth' and 'srcWidth' in LLD @ref DMA_LLD_TypeDef and 'CCR' @ref DMA_CH_CCR_TypeDef
 * @{
 */
#define DMA_WIDTH_8BIT                      (0)     /*!< DMA data width is 8bit */
#define DMA_WIDTH_16BIT                     (1)     /*!< DMA data width is 16bit */
#define DMA_WIDTH_32BIT                     (2)     /*!< DMA data width is 32bit */
/**
 * @}
 */

/**
 * @defgroup DMA_BURST_SIZE DMA Burst Size Config
 * @brief DMA burst size, value of 'srcSize' in 'CCR' @ref DMA_CH_CCR_TypeDef
 * @{
 */
#define DMA_BURST_1                         (0)     /*!< DMA burst size is 1 */
#define DMA_BURST_4                         (1)     /*!< DMA burst size is 4 */
#define DMA_BURST_8                         (2)     /*!< DMA burst size is 8 */
#define DMA_BURST_16                        (3)     /*!< DMA burst size is 16 */
#define DMA_BURST_32                        (4)     /*!< DMA burst size is 32 */
#define DMA_BURST_64                        (5)     /*!< DMA burst size is 64 */
#define DMA_BURST_128                       (6)     /*!< DMA burst size is 128 */
#define DMA_BURST_256                       (7)     /*!< DMA burst size is 256 */
/**
 * @}
 */

/**
 * @defgroup DMA_FIFO_TH   DMA FIFO Threshold Config
 * @brief DMA fifo threshold, value of 'fifoTh' in LLD @ref DMA_LLD_TypeDef and 'CCR' @ref DMA_CH_CCR_TypeDef
 * @{
 */
#define DMA_FIFO_TH_1                       (0)     /*!< DMA FIFO thershold 1 */
#define DMA_FIFO_TH_2                       (1)     /*!< DMA FIFO thershold 2 */
#define DMA_FIFO_TH_4                       (2)     /*!< DMA FIFO thershold 4, max */
/**
 * @}
 */

/**
 * @defgroup DMA_Request_Source DMA Request Source Config
 * @brief DMA request source, value write to register 'SSCFG[CH]', use macro @ref HAL_DMA_GROUP_REQ_SEL
 * @{
 */
#define DMA_REQ_DMACH0                      (0)     /*!< dma ch0 transfer complete */
#define DMA_REQ_DMACH1                      (1)     /*!< dma ch1 transfer complete */
#define DMA_REQ_DMACH2                      (2)     /*!< dma ch0 transfer complete */
#define DMA_REQ_DMACH3                      (3)     /*!< dma ch1 transfer complete */
#define DMA_REQ_DMACH4                      (4)     /*!< dma ch0 transfer complete */
#define DMA_REQ_DMACH5                      (5)     /*!< dma ch1 transfer complete */
#define DMA_REQ_DMACH6                      (6)     /*!< dma ch0 transfer complete */
#define DMA_REQ_DMACH7                      (7)     /*!< dma ch1 transfer complete */
#define DMA_REQ_SEL00                       (8+ 0)  /*!< dma select 0 ack */
#define DMA_REQ_SEL01                       (8+ 1)  /*!< dma select 1 ack */
#define DMA_REQ_SEL02                       (8+ 2)  /*!< dma select 2 ack */
#define DMA_REQ_SEL03                       (8+ 3)  /*!< dma select 3 ack */
#define DMA_REQ_SEL04                       (8+ 4)  /*!< dma select 4 ack */
#define DMA_REQ_SEL05                       (8+ 5)  /*!< dma select 5 ack */
#define DMA_REQ_SEL06                       (8+ 6)  /*!< dma select 6 ack */
#define DMA_REQ_SEL07                       (8+ 7)  /*!< dma select 7 ack */
#define DMA_REQ_SEL08                       (8+ 8)  /*!< dma select 8 ack */
#define DMA_REQ_SEL09                       (8+ 9)  /*!< dma select 9 ack */
#define DMA_REQ_SEL10                       (8+10)  /*!< dma select 10 ack */
#define DMA_REQ_SEL11                       (8+11)  /*!< dma select 11 ack */
#define DMA_REQ_SEL12                       (8+12)  /*!< dma select 12 ack */
#define DMA_REQ_SEL13                       (8+13)  /*!< dma select 13 ack */
#define DMA_REQ_SEL14                       (8+14)  /*!< dma select 14 ack */
#define DMA_REQ_SEL15                       (8+15)  /*!< dma select 15 ack */
#define DMA_REQ_UART0TX                     (24)    /*!< dma select uart0 tx */
#define DMA_REQ_UART0RX                     (25)    /*!< dma select uart0 rx */
#define DMA_REQ_UART1TX                     (26)    /*!< dma select uart1 tx */
#define DMA_REQ_UART1RX                     (27)    /*!< dma select uart1 rx */
#define DMA_REQ_UART2TX                     (28)    /*!< dma select uart2 tx */
#define DMA_REQ_UART2RX                     (29)    /*!< dma select uart2 rx */
#define DMA_REQ_UART3TX                     (30)    /*!< dma select uart3 tx */
#define DMA_REQ_UART3RX                     (31)    /*!< dma select uart3 rx */
#define DMA_REQ_SPI1TX                      (32)    /*!< dma select spi1 tx */
#define DMA_REQ_SPI1RX                      (33)    /*!< dma select spi1 rx */
#define DMA_REQ_SPI2TX                      (34)    /*!< dma select spi2 tx */
#define DMA_REQ_SPI2RX                      (35)    /*!< dma select spi2 rx */
#define DMA_REQ_SPI3TX                      (36)    /*!< dma select spi3 tx */
#define DMA_REQ_SPI3RX                      (37)    /*!< dma select spi3 rx */
#define DMA_REQ_I2S                         (38)    /*!< dma select i2s */
#define DMA_REQ_I2S_S1                      (39)    /*!< dma select i2s slave1 */
#define DMA_REQ_I2S_S2                      (40)    /*!< dma select i2s slave2 */
#define DMA_REQ_SPIFC                       (41)    /*!< dma select spi flash controller */
#define DMA_REQ_SDC                         (42)    /*!< dma select SDC */
#define DMA_REQ_USB                         (43)    /*!< dma select USB */
#define DMA_REQ_TIMER0                      (44)    /*!< dma select timer0 */
#define DMA_REQ_TIMER1                      (45)    /*!< dma select timer1 */
#define DMA_REQ_TIMER2                      (46)    /*!< dma select timer2 */
#define DMA_REQ_TIMER3                      (47)    /*!< dma select timer3 */
#define DMA_REQ_DTIMER                      (48)    /*!< dma select dual timer */
#define DMA_REQ_I2CTX                       (49)    /*!< dma select i2c master tx */
#define DMA_REQ_I2CRX                       (50)    /*!< dma select i2c master rx */
#define DMA_REQ_I2CS                        (51)    /*!< dma select i2c slave */
#define DMA_REQ_DSPI                        (52)    /*!< dma select dsp interrupt */
#define DMA_REQ_DSPE                        (53)    /*!< dma select dsp end */
/**
 * @}
 */
/**
 * @}
 */

/* Exported macros -----------------------------------------------------------*/
/**
 * @defgroup DMA_Exported_Macros DMA Exported Macros
 * @{
 */
/**
 * @brief  Enable group dma module, write register 'SYNC' in @ref DMA_CTRL_TypeDef
 * @param  DMA_CTRL    Specifies the dma group ctrl register, DMA0CTRL or DMA1CTRL, @ref Peripheral_Declaration
 * @param  BCH         Hardware shakehand channel configed to sync, BIT0 to BIT7 indicated DMA ch0 to ch7 (e.g. 0x0F to config ch0 to ch3)
 * @return None
 */
#define HAL_DMA_GROUP_HD_SYNC(DMA_CTRL, BCH)                            ((DMA_CTRL)->SYNC = (BCH))

/**
 * @brief  Enable group dma module, write register 'SYNC' in @ref DMA_CTRL_TypeDef
 * @param  DMA_CTRL    Specifies the dma group ctrl register, DMA0CTRL or DMA1CTRL, @ref Peripheral_Declaration
 * @param  BCH         Hardware shakehand channel configed to sync, BIT0 to BIT7 indicated DMA ch0 to ch7 (e.g. 0x0F to config ch0 to ch3), but do not affect on ch4 to ch7
 * @return None
 */
#define HAL_DMA_GROUP_HD_SYNC_SET_BIT(DMA_CTRL, BCH)                    ((DMA_CTRL)->SYNC |= (BCH))

/**
 * @brief  Enable group dma module, write '1' to reigster 'CR' in @ref DMA_CTRL_TypeDef
 * @param  DMA_CTRL    Specifies the dma group ctrl register, DMA0CTRL or DMA1CTRL, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_DMA_GROUP_ENABLE(DMA_CTRL)                                  ((DMA_CTRL)->CR = 1)

/**
 * @brief  Disable group dma module, write '0' to register 'CR' in @ref DMA_CTRL_TypeDef
 * @param  DMA_CTRL    Specifies the dma group ctrl register, DMA0CTRL or DMA1CTRL, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_DMA_GROUP_DISABLE(DMA_CTRL)                                 ((DMA_CTRL)->CR = 0)

/**
 * @brief  Group dma terminal count interrupt state clear, write register 'TCICR' in @ref DMA_CTRL_TypeDef
 * @param  DMA_CTRL    Specifies the dma group ctrl register, DMA0CTRL or DMA1CTRL, @ref Peripheral_Declaration
 * @param  BCH         Channel configed to clear, BIT0 to BIT7 indicated DMA ch0 to ch7 (e.g. 0x0F to config ch0 to ch3)
 * @return None
 */
#define HAL_DMA_GROUP_TC_INT_CLR(DMA_CTRL, BCH)                         ((DMA_CTRL)->TCICR = (BCH))

/**
 * @brief  Group dma terminal count interrupt state get, read register 'TCISR' in @ref DMA_CTRL_TypeDef
 * @param  DMA_CTRL    Specifies the dma group ctrl register, DMA0CTRL or DMA1CTRL, @ref Peripheral_Declaration
 * @return Channel terminal count interrupt state get, BIT0 to BIT7 indicated DMA ch0 to ch7 (e.g. 0x0F means config ch0 to ch3)
 */
#define HAL_DMA_GROUP_TC_INT_GET(DMA_CTRL)                              ((DMA_CTRL)->TCISR)

/**
 * @brief  Group dma error and abort interrupt state clear, write register 'EAICR' in @ref DMA_CTRL_TypeDef
 * @param  DMA_CTRL    Specifies the dma group ctrl register, DMA0CTRL or DMA1CTRL, @ref Peripheral_Declaration
 * @param  BCH         Channel configed to clear, BIT0 to BIT7 indicated DMA error ch0 to ch7, BIT16 to BIT23 indicated DMA abort ch0 to ch7.
 * @return None
 */
#define HAL_DMA_GROUP_EA_INT_CLR(DMA_CTRL, BCH)                         ((DMA_CTRL)->EAICR = (BCH))

/**
 * @brief  Group dma error and abort interrupt state get, read register 'EAISR' in @ref DMA_CTRL_TypeDef
 * @param  DMA_CTRL    Specifies the dma group ctrl register, DMA0CTRL or DMA1CTRL, @ref Peripheral_Declaration
 * @return Channel error and abort interrupt state get, BIT0 to BIT7 indicated DMA error ch0 to ch7, BIT16 to BIT23 indicated DMA abort ch0 to ch7.
 */
#define HAL_DMA_GROUP_EA_INT_GET(DMA_CTRL)                              ((DMA_CTRL)->EAISR)

/**
 * @brief  Group dma interrupt state get, read register 'ISR' in @ref DMA_CTRL_TypeDef
 * @param  DMA_CTRL    Specifies the dma group ctrl register, DMA0CTRL or DMA1CTRL, @ref Peripheral_Declaration
 * @return Channel interrupt state get, BIT0 to BIT7 indicated DMA ch0 to ch7.
 */
#define HAL_DMA_GROUP_INT_GET(DMA_CTRL)                                 ((DMA_CTRL)->ISR)

/**
 * @brief  Config dma request source of 16 select channel, write register 'SSCFG[CH]' in @ref DMA_SEL_TypeDef
 * @param  DMA_SELECT  Specifies the dma group select register, DMA0SELECT or DMA1SELECT, @ref Peripheral_Declaration
 * @param  CH          Select chanel index number, 0~15
 * @param  REQ         @ref DMA_Request_Source
 * @return None
 */
#define HAL_DMA_GROUP_REQ_SEL(DMA_SELECT, CH, REQ)                      ((DMA_SELECT)->SSCFG[CH] = (REQ))

/**
 * @brief  Dma channel ctrl set, write register 'CCR' in @ref DMA_CH_TypeDef
 * @param  DMA_CH      Specifies the dma channel, DMA0CH0~DMA0CH7, DMA1CH0~DMACH7, @ref Peripheral_Declaration
 * @param  CTRL        Channel ctrl value write to the channel ctrl register
 * @return None
 */
#define HAL_DMA_CH_CTRL_SET(DMA_CH, CTRL)                               ((DMA_CH)->CCR = (CTRL))

/**
 * @brief  Dma channel config value set, write register 'CCFG' in @ref DMA_CH_TypeDef
 * @param  DMA_CH      Specifies the dma channel, DMA0CH0~DMA0CH7, DMA1CH0~DMACH7, @ref Peripheral_Declaration
 * @param  CFG         Channel config value write to the channel config register
 * @return None
 */
#define HAL_DMA_CH_CFG_SET(DMA_CH, CFG)                                 ((DMA_CH)->CCFG = (CFG))

/**
 * @brief  Dma channel source address set, write register 'CSRC' in @ref DMA_CH_TypeDef
 * @param  DMA_CH      Specifies the dma channel, DMA0CH0~DMA0CH7, DMA1CH0~DMACH7, @ref Peripheral_Declaration
 * @param  ADDR        Channel source address
 * @return None
 */
#define HAL_DMA_CH_SRC_ADDR_SET(DMA_CH, ADDR)                           ((DMA_CH)->CSRC = (ADDR))

/**
 * @brief  Dma channel destinaiton address set, write register 'CDST' in @ref DMA_CH_TypeDef
 * @param  DMA_CH      Specifies the dma channel, DMA0CH0~DMA0CH7, DMA1CH0~DMACH7, @ref Peripheral_Declaration
 * @param  ADDR        Channel destination address
 * @return None
 */
#define HAL_DMA_CH_DST_ADDR_SET(DMA_CH, ADDR)                           ((DMA_CH)->CDST = (ADDR))

/**
 * @brief  Dma channel link list pointer address set, write register 'CLLP' in @ref DMA_CH_TypeDef
 * @param  DMA_CH      Specifies the dma channel, DMA0CH0~DMA0CH7, DMA1CH0~DMACH7, @ref Peripheral_Declaration
 * @param  ADDR        Channel link list pointer address align to 4-byte
 * @return None
 */
#define HAL_DMA_CH_LLP_ADDR_SET(DMA_CH, ADDR)                           ((DMA_CH)->CLLP = (ADDR))

/**
 * @brief  Dma channel transfer size set, write register 'CTSR' in @ref DMA_CH_TypeDef
 * @param  DMA_CH      Specifies the dma channel, DMA0CH0~DMA0CH7, DMA1CH0~DMACH7, @ref Peripheral_Declaration
 * @param  SIZE        Channel transfer size value
 * @return None
 */
#define HAL_DMA_CH_TRANS_SIZE_SET(DMA_CH, SIZE)                         ((DMA_CH)->CTSR = (SIZE))

/**
 * @brief  Dma channel enable, set bit 'chEn' in register 'CCR', @ref DMA_CH_CCR_TypeDef
 * @param  DMA_CH      Specifies the dma channel, DMA0CH0~DMA0CH7, DMA1CH0~DMACH7, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_DMA_CH_ENABLE(DMA_CH)                                       ((DMA_CH)->CCR |= ( BIT0))

/**
 * @brief  Dma channel disable, clr bit 'chEn' in register 'CCR', @ref DMA_CH_CCR_TypeDef
 * @param  DMA_CH      Specifies the dma channel, DMA0CH0~DMA0CH7, DMA1CH0~DMACH7, @ref Peripheral_Declaration
 * @return None
 */
#define HAL_DMA_CH_DISABLE(DMA_CH)                                      ((DMA_CH)->CCR &= (~BIT0))
/**
 * @}
 */

/* Private macros ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/**
 * @addtogroup DMA_Exported_Functions DMA Exported Functions
 * @{
 */
/**
 * @brief  dma module irq handler function
 * @param  DMACtrl     Specifies the dma group ctrl register, DMA0CTRL or DMA1CTRL
 * @return None
 * @details
        This irq handler function should be called in DMA IRQ handle, DMA0_IRQHandler and DMA1_IRQHandler
        1. Check 'ISR' in the group of DMA
        2. If bit value in 'TCISR' is set, will call callback function @ref HalDmaTcCallback which can be rewrite in user code
        3. If bit value in 'EAISR' is set, will call callback function @ref HalDmaEaCallback which can be rewrite in user code
 */
void HalDmaIrqHandler(DMA_CTRL_TypeDef *DMACtrl);

/* for code compatible */
#define hal_dma_irq_handler                     HalDmaIrqHandler
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
 * @addtogroup HAL_WEAK_Functions_DMA Weak Functions of DMA
 * @brief Weak functions in a0_hal_dma.c
 * @{
 */
/**
 * @brief  dma terminal count interrupt callback function
 * @param  DMACtrl     Specifies the dma group ctrl register, DMA0CTRL or DMA1CTRL
 * @param  tcIsr       read value from 'TCISR' in @ref DMA_CTRL_TypeDef, for user to check
 * @return None
 * @details
        This is a weak function, it can be rewritten in user code to decide how to do when terminal count interrupt occured.
 */
void HalDmaTcCallback(DMA_CTRL_TypeDef *DMACtrl, uint32_t tcIsr);
/**
 * @brief  dma error or abort interrupt callback function
 * @param  DMACtrl     Specifies the dma group ctrl register, DMA0CTRL or DMA1CTRL
 * @param  eaIsr       read value from 'EAISR' in @ref DMA_CTRL_TypeDef, for user to check
 * @return None
 * @details
        This is a weak function, it can be rewritten in user code to decide how to do when error or abort interrupt occured.
 */
void HalDmaEaCallback(DMA_CTRL_TypeDef *DMACtrl, uint32_t eaIsr);

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
