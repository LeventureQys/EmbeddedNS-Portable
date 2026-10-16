/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_otg.h
 * @brief   Header file of OTG HAL module.
    | Version | Data       | Author     | Info                                                     |
    | ------- | ---------- | ---------- | -------------------------------------------------------- |
    | 1.0.0   | 2021/07/01 | huangzheng | First version                                            |
    | 1.0.1   | 2021/09/22 | huangzheng | Adjust comment for doxygen                               |
    | 1.0.2   | 2021/10/11 | huangzheng | Add macros for MPW version                               |
    | 1.0.3   | 2021/10/27 | huangzheng | Add SOF register access                                  |
    | 1.0.4   | 2022/03/10 | huangzheng | 1. Fixed macros of REG(0x160)                            |
    |         |            |            | 2. Add fifoBlockNum to otg_ep_fifo_cfg_t for fifo config |
    | 1.0.5   | 2022/04/07 | huangzheng | Adjust for Final version                                 |
    | 1.0.6   | 2022/10/21 | huangzheng | Add EP send zero-length packet macro                     |
 * @version V1.0.6
 * @date    2022/10/21
 * @author  huangzheng
 ******************************************************************************
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __A0_HAL_OTG_H
#define __A0_HAL_OTG_H

/* Includes -----------------------------------------------------------------*/
#include "jat_typedef.h"
#include "a0.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup OTG
 * @brief OTG module
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/**
 * @defgroup OTG_Exported_Types OTG Exported Types
 * @{
 */
/**
 * @brief  SETUP Packet Structure definition for usb
 */
typedef struct {
    union {
        struct {
            uint8_t recipient:5;    /*!< 0, Device; 1, Interface; 2, Endpoint; 3, Other; 4~31, Reserved */
            uint8_t type:2;         /*!< 0, Standard; 1, Class; 2, Vendor */
            uint8_t direction:1;    /*!< 0, OUT; 1, IN */
        } b;                        /*!< bit field access */
        uint8_t byte;               /*!< 8bit byte access */
    }bmRequestType;
    uint8_t  bRequest;              /*!< Specific standard request number */
    uint16_t wValue;                /*!< varies according to request */
    uint16_t wIndex;                /*!< varies according to request */
    uint16_t wLength;               /*!< Number of bytes to transfer */
}setup_packet_t;
/**
 * @brief  SETUP Packet Structure definition
 */
typedef struct {
    uint8_t     epIdx;              /*!< endpoint index number, 1~8 */
    uint8_t     transType;          /*!< transfer type, 1, iso, 2, bulk, 3, int */
    uint8_t     fifo1024:4;         /*!< fifo size, 0, 512, 1, 1024 */
    uint8_t     fifoBlockNum:4;     /*!< fifo block num, 0, single, 1, double, 2, triple */
    uint8_t     fifoInIdx:4;        /*!< fifo in index number, 0~7, 0xF means no use */
    uint8_t     fifoOutIdx:4;       /*!< fifo out index number, 0~7, 0xF means no use */
}otg_ep_fifo_cfg_t;
/**
 * @}
 */

/* Exported constants -------------------------------------------------------*/
/**
 * @defgroup OTG_Exported_Constants OTG Exported Constants
 * @{
 */
#define OTG_MAX_EP_NUM                      (8)         /*!< Endpoint max number in OTG module */
#define OTG_MAX_FIFO_NUM                    (8)         /*!< Fifo max number in OTG module */
#define OTG_MAX_EP0_PACK_SIZE               (64)        /*!< Endpoint0 packet max size in OTG module */
/**
 * @defgroup OTG_FIFO_BLOCK OTG Fifo Block
 * @brief OTG fifo block
 * @{
 */
#define OTG_FIFO_BLK_SINGLE                 (0)
#define OTG_FIFO_BLK_DOUBLE                 (1)
#define OTG_FIFO_BLK_TRIPL                  (2)
/**
 * @}
 */
/**
 * @defgroup OTG_FIFO_TYPE OTG Fifo Transfer Type
 * @brief OTG fifo transfer type
 * @{
 */
#define OTG_FIFO_TYPE_ISO                   (1)
#define OTG_FIFO_TYPE_BULK                  (2)
#define OTG_FIFO_TYPE_INT                   (3)
/**
 * @}
 */
/**
 * @defgroup OTG_REG_OFFSET OTG Regiseter Offset
 * @brief OTG reigster offset definition
 * @{
 */
#define OTG_HOST_USBCMD                     (0x10)      /*!< OTG Host Control USB Command Register                  */
#define OTG_HOST_USBSTS                     (0x14)      /*!< OTG Host Control USB Status Register                   */
#define OTG_HOST_USBINTR                    (0x18)      /*!< OTG Host Control USB Interrupt Enable Register         */
#define OTG_HOST_FRINDEX                    (0x1C)      /*!< OTG Host Control Frame Index Register                  */
#define OTG_HOST_PERIODICLISTBASE           (0x24)      /*!< OTG Host Control Periodic Frame List Base Addr Register*/
#define OTG_HOST_ASYNCLISTADDR              (0x28)      /*!< OTG Host Control Asynchronous List Address Register    */
#define OTG_HOST_PORTSC                     (0x30)      /*!< OTG Host Control Port Status and Control Register      */
#define OTG_HOST_HCMISC                     (0x40)      /*!< OTG Host Control Miscellaneous Register                */
#define OTG_HOST_FS_EOF                     (0x44)      /*!< OTG Host Control FS EOF1 Timing Point Register         */
#define OTG_HOST_HS_EOF                     (0x48)      /*!< OTG Host Control HS EOF1 Timing Point Register         */
#define OTG_CSR                             (0x80)      /*!< OTG Control Status Register                            */
#define OTG_ISR                             (0x84)      /*!< OTG Interrupt Status Register                          */
#define OTG_IER                             (0x88)      /*!< OTG Interrupt Enable Register                          */
#define OTG_DEV_CTL                         (0x100)     /*!< OTG Device Control Register                            */
#define OTG_DEV_ADR                         (0x104)     /*!< OTG Device Address Register                            */
#define OTG_DEV_TST                         (0x108)     /*!< OTG Device Test Register                               */
#define OTG_DEV_SFN                         (0x10C)     /*!< OTG Device SOF Frame Number Register                   */
#define OTG_DEV_SMT                         (0x110)     /*!< OTG Device SOF Mask Timer Register                     */
#define OTG_PHY_TST                         (0x114)     /*!< OTG PHY Test Register                                  */
#define OTG_DEV_CXCFE                       (0x120)     /*!< OTG Device Cx Config and Fifo Empty Status Register    */
#define OTG_DEV_ICR                         (0x124)     /*!< OTG Device Idle Counter Register                       */
#define OTG_DEV_MIGR                        (0x130)     /*!< OTG Device Mask of Interrupt Group Register            */
#define OTG_DEV_MISG0                       (0x134)     /*!< OTG Device Mask of Interrupt Source Group0 Register    */
#define OTG_DEV_MISG1                       (0x138)     /*!< OTG Device Mask of Interrupt Source Group1 Register    */
#define OTG_DEV_MISG2                       (0x13C)     /*!< OTG Device Mask of Interrupt Source Group2 Register    */
#define OTG_DEV_IGR                         (0x140)     /*!< OTG Device Interrupt Group Register                    */
#define OTG_DEV_ISG0                        (0x144)     /*!< OTG Device Interrupt Source Group0 Register            */
#define OTG_DEV_ISG1                        (0x148)     /*!< OTG Device Interrupt Source Group1 Register            */
#define OTG_DEV_ISG2                        (0x14C)     /*!< OTG Device Interrupt Source Group2 Register            */
#define OTG_DEV_RXZ                         (0x150)     /*!< OTG Device Receive Zero-Length Data Packet Register    */
#define OTG_DEV_TXZ                         (0x154)     /*!< OTG Device Transfer Zero-Length Data Packet Register   */
#define OTG_DEV_ISE                         (0x158)     /*!< OTG Device Isochronous Sequential Error/Abort Register */
#define OTG_DEV_INMPS                       (0x160)     /*!< OTG Device In Endpoint n MaxPacketSize Register        */
#define OTG_DEV_OUTMPS                      (0x180)     /*!< OTG Device Out Endpoint n MaxPacketSize Register       */
#define OTG_DEV_EPMAP0                      (0x1A0)     /*!< OTG Device Endpoint 1 to 4 Map Register                */
#define OTG_DEV_EPMAP1                      (0x1A4)     /*!< OTG Device Endpoint 5 to 8 Map Register                */
#define OTG_DEV_FMAP1                       (0x1A8)     /*!< OTG Device Fifo 0 to 3 Map Register                    */
#define OTG_DEV_FMAP2                       (0x1D8)     /*!< OTG Device Fifo 4 to 7 Map Register                    */
#define OTG_DEV_FCFG1                       (0x1AC)     /*!< OTG Device Fifo 0 to 3 Config Register                 */
#define OTG_DEV_FCFG2                       (0x1DC)     /*!< OTG Device Fifo 4 to 7 Config Register                 */
#define OTG_DEV_FIBC                        (0x1B0)     /*!< OTG Device Fifo n Reset and Byte Count Register        */
#define OTG_DMA_TFN                         (0x1C0)     /*!< OTG Device DMA Target FIFO Number Register             */
#define OTG_DMA_CPS1                        (0x1C8)     /*!< OTG Device DMA Controller Parameter Setting1 Register  */
#define OTG_DMA_CPS2                        (0x1CC)     /*!< OTG Device DMA Controller Parameter Setting2 Register  */
#define OTG_DMA_CPS3                        (0x1D0)     /*!< OTG Device DMA Controller Parameter Setting3 Register  */
/**
 * @}
 */
/**
 * @defgroup OTG_HOST_INT_BIT OTG Host Controller Interrupt bit definition
 * @brief Bit definition in OTG_HOST_USBSTS and OTG_HOST_USBINTR
 * @{
 */
#define OTGHC_INT                           (BIT0)
#define OTGHC_INT_USB_ERR                   (BIT1)
#define OTGHC_INT_PORT_CHG                  (BIT2)
#define OTGHC_INT_FRL_ROL                   (BIT3)
#define OTGHC_INT_SYS_ERR                   (BIT4)
#define OTGHC_INT_ASYNC_ADV                 (BIT5)
/**
 * @}
 */
/**
 * @defgroup OTG_INT_ENABLE_BIT OTG Controller Interrupt bit definition
 * @brief Bit definition in OTG_IER
 * @{
 */
#define OTGC_B_SRPDN                        (BIT0)
#define OTGC_A_SRPDET                       (BIT4)
#define OTGC_A_VBUSERR                      (BIT5)
#define OTGC_B_SESSEND                      (BIT6)
#define OTGC_RLCHG                          (BIT8)
#define OTGC_IDCHG                          (BIT9)
#define OTGC_A_OVC                          (BIT10)
#define OTGC_A_BPLGRMV                      (BIT11)
#define OTGC_A_APLGRMV                      (BIT12)
#define OTGC_INT_A_TYPE                     (OTGC_RLCHG|OTGC_IDCHG|OTGC_A_SRPDET|OTGC_A_VBUSERR|OTGC_A_OVC|OTGC_A_APLGRMV)
#define OTGC_INT_B_TYPE                     (OTGC_RLCHG|OTGC_IDCHG|OTGC_B_SRPDN)
/**
 * @}
 */
/**
 * @}
 */

/* Exported macros ----------------------------------------------------------*/
/**
 * @defgroup OTG_Exported_Macros OTG Exported Macros
 * @{
 */

#define U8USB(regOffset)                                                (U8REG(A0_USB_BASE + regOffset))
#define U32USB(regOffset)                                               (U32REG(A0_USB_BASE + regOffset))
#define OTG_IRQ_ENABLE()                                                (NVIC_EnableIRQ(USB_IRQn))
#define OTG_IRQ_DISABLE()                                               (NVIC_DisableIRQ(USB_IRQn))

//only use for MPW version
//in new version, address do not need to adjust
#define OTG_MEM_ADDR_ADJ(addr)                                          (addr)

/**
 * @defgroup HC_REG HOST Controller Register Operation
 * @{
 */
/**
 * @defgroup OTG_HOST_USBCMD_OP OTG_HOST_USBCMD (0x10) Register Operation
 * @brief Register operation, OTG_HOST_USBCMD (0x10)
 * @{
 */
#define OTGHC_FRAMELIST_SIZE_SET(val)                                   (U32USB(OTG_HOST_USBCMD) = (U32USB(OTG_HOST_USBCMD) & (~(BIT2|BIT3)) | (((uint32_t)val&0x3)<<2)))   /*0, 1024, 1, 512, 2, 256 */
#define OTGHC_PK_CNT_SET(val)                                           (U32USB(OTG_HOST_USBCMD) = (U32USB(OTG_HOST_USBCMD) & (~(BIT8|BIT9)) | (((uint32_t)val&0x3)<<8)))
#define OTGHC_RUN_CHECK()                                               (U32USB(OTG_HOST_USBCMD) &  ( BIT0))
#define OTGHC_RESET_CHECK()                                             (U32USB(OTG_HOST_USBCMD) &  ( BIT1))
#define OTGHC_PSCH_EN_CHECK()                                           (U32USB(OTG_HOST_USBCMD) &  ( BIT4))
#define OTGHC_ASCH_EN_CHECK()                                           (U32USB(OTG_HOST_USBCMD) &  ( BIT5))
#define OTGHC_RUN()                                                     (U32USB(OTG_HOST_USBCMD) |= ( BIT0))
#define OTGHC_RESET_SET()                                               (U32USB(OTG_HOST_USBCMD) |= ( BIT1))
#define OTGHC_PSCH_EN()                                                 (U32USB(OTG_HOST_USBCMD) |= ( BIT4))
#define OTGHC_ASCH_EN()                                                 (U32USB(OTG_HOST_USBCMD) |= ( BIT5))
#define OTGHC_STOP()                                                    (U32USB(OTG_HOST_USBCMD) &= (~BIT0))
#define OTGHC_PSCH_DIS()                                                (U32USB(OTG_HOST_USBCMD) &= (~BIT4))
#define OTGHC_ASCH_DIS()                                                (U32USB(OTG_HOST_USBCMD) &= (~BIT5))
/**
 * @}
 */
/**
 * @defgroup OTG_HOST_USBSTS_OP OTG_HOST_USBSTS (0x14) Register Operation
 * @brief Register operation, OTG_HOST_USBSTS (0x14)
 * @{
 */
#define OTGHC_USBSTS_SET(val)                                           (U32USB(OTG_HOST_USBSTS) =  (val))
#define OTGHC_USBSTS_RD()                                               (U32USB(OTG_HOST_USBSTS))
#define OTGHC_PSCH_CHECK()                                              (U32USB(OTG_HOST_USBSTS) &  ( BIT14))
#define OTGHC_ASCH_CHECK()                                              (U32USB(OTG_HOST_USBSTS) &  ( BIT15))
#define OTGHC_INT_CLR()                                                 (U32USB(OTG_HOST_USBSTS) |= ( BIT0 ))
#define OTGHC_INT_USBERR_CLR()                                          (U32USB(OTG_HOST_USBSTS) |= ( BIT1 ))
#define OTGHC_INT_PORT_CHG_CLR()                                        (U32USB(OTG_HOST_USBSTS) |= ( BIT2 ))
#define OTGHC_INT_FRL_ROL_CLR()                                         (U32USB(OTG_HOST_USBSTS) |= ( BIT3 ))
/**
 * @}
 */
/**
 * @defgroup OTG_HOST_USBINTR_OP OTG_HOST_USBINTR (0x18) Register Operation
 * @brief Register operation, OTG_HOST_USBINTR (0x18)
 * @{
 */
#define OTGHC_INT_SET(val)                                              (U32USB(OTG_HOST_USBINTR) = (val))
#define OTGHC_INT_GET()                                                 (U32USB(OTG_HOST_USBINTR))
/**
 * @}
 */
/**
 * @defgroup OTG_HOST_FRINDEX_OP OTG_HOST_FRINDEX (0x1C) Register Operation
 * @brief Register operation, OTG_HOST_FRINDEX (0x1C)
 * @{
 */
#define OTGHC_FRAME_IDX_RD()                                            (U32USB(OTG_HOST_FRINDEX) & 0x1FFF)
/**
 * @}
 */
/**
 * @defgroup OTG_HOST_PERIODICLISTBASE_OP OTG_HOST_PERIODICLISTBASE (0x24) Register Operation
 * @brief Register operation, OTG_HOST_PERIODICLISTBASE (0x24)
 * @{
 */
#define OTGHC_PERIODIC_BASE_ADDR_SET(val)                               (U32USB(OTG_HOST_PERIODICLISTBASE) = (val)) /* [31:12] effective */
/**
 * @}
 */
/**
 * @defgroup OTG_HOST_ASYNCLISTADDR_OP OTG_HOST_ASYNCLISTADDR (0x28) Register Operation
 * @brief Register operation, OTG_HOST_ASYNCLISTADDR (0x28), for MPW version, need to &0x2FFFFFFF
 * @{
 */
#define OTGHC_ASYNC_ADDR_SET(val)                                       (U32USB(OTG_HOST_ASYNCLISTADDR) = (val)&0x2FFFFFFF)     /* [31:5] effective */
/**
 * @}
 */
/**
 * @defgroup OTG_HOST_PORTSC_OP OTG_HOST_PORTSC (0x30) Register Operation
 * @brief Register operation, OTG_HOST_PORTSC (0x30)
 * @{
 */
#define OTGHC_CONNECT_CHECK()                                           (U32USB(OTG_HOST_PORTSC) &  ( BIT0))
#define OTGHC_CONNECT_CHANGE_CHECK()                                    (U32USB(OTG_HOST_PORTSC) &  ( BIT1))
#define OTGHC_PORT_EN_CHECK()                                           (U32USB(OTG_HOST_PORTSC) &  ( BIT2))
#define OTGHC_PORT_EN_DIS_CHANGE_CHECK()                                (U32USB(OTG_HOST_PORTSC) &  ( BIT3))
#define OTGHC_FORCE_PORT_RESUME_CHECK()                                 (U32USB(OTG_HOST_PORTSC) &  ( BIT6))
#define OTGHC_PORT_SUSPEND_CHECK()                                      (U32USB(OTG_HOST_PORTSC) &  ( BIT7))
#define OTGHC_PORT_RESET_CHECK()                                        (U32USB(OTG_HOST_PORTSC) &  ( BIT8))
#define OTGHC_CONNECT_CHANGE_CLR()                                      (U32USB(OTG_HOST_PORTSC) |= ( BIT1))
#define OTGHC_PORT_EN()                                                 (U32USB(OTG_HOST_PORTSC) |= ( BIT2))
#define OTGHC_PORT_EN_DIS_CHANGE_CLR()                                  (U32USB(OTG_HOST_PORTSC) |= ( BIT3))
#define OTGHC_PORT_SUSPEND_SET()                                        (U32USB(OTG_HOST_PORTSC) |= ( BIT7))
#define OTGHC_PORT_RESET_SET()                                          (U32USB(OTG_HOST_PORTSC) |= ( BIT8))
#define OTGHC_PORT_DIS()                                                (U32USB(OTG_HOST_PORTSC) &= (~BIT2))
#define OTGHC_PORT_RESET_CLR()                                          (U32USB(OTG_HOST_PORTSC) &= (~BIT8))
/**
 * @}
 */
/**
 * @defgroup OTG_HOST_HCMISC_OP OTG_HOST_HCMISC (0x40) Register Operation
 * @brief Register operation, OTG_HOST_HCMISC (0x40)
 * @{
 */
#define OTGHC_EOF1_TIME_SET(val)                                        (U32USB(OTG_HOST_HCMISC) = (U32USB(OTG_HOST_HCMISC) & (~(BIT2|BIT3)) | (((uint32_t)val&0x3)<<2)))
#define OTGHC_HOST_PHY_SUSPEND()                                        (U32USB(OTG_HOST_HCMISC) |= ( BIT6))
/**
 * @}
 */
/**
 * @defgroup OTG_HOST_FS_EOF_OP OTG_HOST_FS_EOF (0x44) Register Operation
 * @brief Register operation, OTG_HOST_FS_EOF (0x44)
 * @{
 */
#define OTGHC_EOF1_TIMEING_UNUSE()                                      (U32USB(OTG_HOST_FS_EOF) &= (~(BIT14|BIT13|BIT12)))     /* use config value in 0x44[3:2] */
/**
 * @}
 */
/**
 * @}
 */

/**
 * @defgroup OTGC_REG OTG Controller Register Operation
 * @{
 */
/**
 * @defgroup OTG_CSR_OP OTG_CSR (0x80) Register Operation
 * @brief Register operation, OTG_CSR (0x80)
 * @{
 */
#define OTGC_SPEED_RD()                                                 ((U32USB(OTG_CSR) >> 22) & 0x3)     /*!< return 0, fs, 1, ls, 2, hs */
#define OTGC_ROLE_CHECK_DEV()                                           (U32USB(OTG_CSR) &  ( BIT20))       /*!< return 0, host, 1, device */
#define OTGC_A_BUS_REQ_SET()                                            (U32USB(OTG_CSR) |= ( BIT4 ))
#define OTGC_A_BUS_DROP_CLR()                                           (U32USB(OTG_CSR) &= (~BIT5 ))
/**
 * @}
 */
/**
 * @defgroup OTG_ISR_OP OTG_ISR (0x84) Register Operation
 * @brief Register operation, OTG_ISR (0x84)
 * @{
 */
#define OTGC_INT_STATE_RD()                                             (U32USB(OTG_ISR))
#define OTGC_INT_STATE_CLR(val)                                         (U32USB(OTG_ISR) |= (val))
/**
 * @}
 */
/**
 * @defgroup OTG_IER_OP OTG_IER (0x88) Register Operation
 * @brief Register operation, OTG_IER (0x88)
 * @{
 */
#define OTGC_INT_EN_RD()                                                (U32USB(OTG_IER))
#define OTGC_INT_EN_SET(val)                                            (U32USB(OTG_IER) |= ( (val)))
#define OTGC_INT_EN_CLR(val)                                            (U32USB(OTG_IER) &= (~(val)))
/**
 * @}
 */
/**
 * @}
 */

/**
 * @defgroup DC_REG DEVICE Controller Register Operation
 * @{
 */
/**
 * @defgroup OTG_DEV_CTL_OP OTG_DEV_CTL (0x100) Register Operation
 * @brief Register operation, OTG_DEV_CTL (0x100)
 * @{
 */
#define OTG_DEV_REMOTE_WAKEUP_CHECK()                                   (U32USB(OTG_DEV_CTL) &  ( BIT0))
#define OTG_DEV_GLOBAL_INT_EN_CHECK()                                   (U32USB(OTG_DEV_CTL) &  ( BIT2))
#define OTG_DEV_SOFT_RST_CHECK()                                        (U32USB(OTG_DEV_CTL) &  ( BIT4))
#define OTG_DEV_HS_CHECK()                                              (U32USB(OTG_DEV_CTL) &  ( BIT6))
#define OTG_DEV_REMOTE_WAKEUP_SET()                                     (U32USB(OTG_DEV_CTL) |= ( BIT0))    /*!< enable device can be wakened up from L2 by wakeup singnal */
#define OTG_DEV_GLOBAL_INT_EN()                                         (U32USB(OTG_DEV_CTL) |= ( BIT2))
#define OTG_DEV_GO_SUSPEND()                                            (U32USB(OTG_DEV_CTL) |= ( BIT3))    /*!< active the suspend mode of PHY */
#define OTG_DEV_SOFT_RST()                                              (U32USB(OTG_DEV_CTL) |= ( BIT4))
#define OTG_DEV_CHIP_EN()                                               (U32USB(OTG_DEV_CTL) |= ( BIT5))
#define OTG_DEV_FS_SET()                                                (U32USB(OTG_DEV_CTL) |= ( BIT9))
#define OTG_DEV_REMOTE_WAKEUP_CLR()                                     (U32USB(OTG_DEV_CTL) &= (~BIT0))
#define OTG_DEV_GLOBAL_INT_DIS()                                        (U32USB(OTG_DEV_CTL) &= (~BIT2))
#define OTG_DEV_HS_SET()                                                (U32USB(OTG_DEV_CTL) &= (~BIT9))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_ADR_OP OTG_DEV_ADR (0x104) Register Operation
 * @brief Register operation, OTG_DEV_ADR (0x104)
 * @{
 */
#define OTG_DEV_ADDR_SET(val)                                           (U32USB(OTG_DEV_ADR) =  (uint32_t)(val))
#define OTG_DEV_CFG_CHECK()                                             (U32USB(OTG_DEV_ADR) &  ( BIT7))
#define OTG_DEV_CFG_SET()                                               (U32USB(OTG_DEV_ADR) |= ( BIT7))
#define OTG_DEV_CFG_CLR()                                               (U32USB(OTG_DEV_ADR) &= (~BIT7))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_TST_OP OTG_DEV_TST (0x108) Register Operation
 * @brief Register operation, OTG_DEV_TST (0x108)
 * @{
 */
#define OTG_DEV_CLR_ALL_FIFO()                                          (U32USB(OTG_DEV_TST) |= ( BIT0))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_SFN_OP OTG_DEV_SFN (0x10C) Register Operation
 * @brief Register operation, OTG_DEV_SFN (0x10C)
 * @{
 */
#define OTG_DEV_SOF_GET()                                               (U32USB(OTG_DEV_SFN))
#define OTG_DEV_SOF_FRAME_GET()                                         (U32USB(OTG_DEV_SFN) & 0x7FF)
#define OTG_DEV_SOF_MACRO_FRAME_GET()                                   ((U32USB(OTG_DEV_SFN) >> 11) & 0x7)
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_SMT_OP OTG_DEV_SMT (0x110) Register Operation
 * @brief Register operation, OTG_DEV_SMT (0x110)
 * @{
 */
#define OTG_DEV_SOF_MASK_HS()                                           (U32USB(OTG_DEV_SMT) = 0x044C)
#define OTG_DEV_SOF_MASK_FS()                                           (U32USB(OTG_DEV_SMT) = 0x2710)
/**
 * @}
 */
/**
 * @defgroup OTG_PHY_TST_OP OTG_PHY_TST (0x114) Register Operation
 * @brief Register operation, OTG_PHY_TST (0x114)
 * @{
 */
#define OTG_DEV_UNPLUG_SET()                                            (U32USB(OTG_PHY_TST) |= ( BIT0))
#define OTG_DEV_UNPLUG_CLR()                                            (U32USB(OTG_PHY_TST) &= (~BIT0))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_CXCFE_OP OTG_DEV_CXCFE (0x120) Register Operation
 * @brief Register operation, OTG_DEV_CXCFE (0x120)
 * @{
 */
#define OTG_DEV_CX_FIFO_BYTE_CNT()                                      ((U32USB(OTG_DEV_CXCFE) >> 24) & 0x7F)
#define OTG_DEV_CX_FIFO_FULL_CHECK()                                    (U32USB(OTG_DEV_CXCFE) &  ( BIT4))
#define OTG_DEV_CX_FIFO_EMPTY_CHECK()                                   (U32USB(OTG_DEV_CXCFE) &  ( BIT5))
#define OTG_DEV_EP0_DONE_SET()                                          (U32USB(OTG_DEV_CXCFE) |= ( BIT0))
#define OTG_DEV_TST_PKDONE_SET()                                        (U32USB(OTG_DEV_CXCFE) |= ( BIT1))
#define OTG_DEV_EP0_STALL_SET()                                         (U32USB(OTG_DEV_CXCFE) |= ( BIT2))
#define OTG_DEV_CX_FIFO_CLR()                                           (U32USB(OTG_DEV_CXCFE) |= ( BIT3))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_ICR_OP OTG_DEV_ICR (0x124) Register Operation
 * @brief Register operation, OTG_DEV_ICR (0x124)
 * @{
 */
#define OTG_DEV_IDLE_CNT_SET(val)                                       (U32USB(OTG_DEV_ICR) = (val))
#define OTG_DEV_IDLE_CNT_GET()                                          (U32USB(OTG_DEV_ICR))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_MIGR_OP OTG_DEV_MIGR (0x130) Register Operation
 * @brief Register operation, OTG_DEV_MIGR (0x130)
 * @{
 */
#define OTG_DEV_INT_GROUP_MASK_RD()                                     (U32USB(OTG_DEV_MIGR))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_MISG0_OP OTG_DEV_MISG0 (0x134) Register Operation
 * @brief Register operation, OTG_DEV_MISG0 (0x134)
 * @{
 */
#define OTG_DEV_INT_SRC0_MASK_RD()                                      (U32USB(OTG_DEV_MISG0))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_MISG1_OP OTG_DEV_MISG1 (0x138) Register Operation
 * @brief Register operation, OTG_DEV_MISG1 (0x138)
 * @{
 */
#define OTG_DEV_INT_SRC1_MASK_RD()                                      (U32USB(OTG_DEV_MISG1))
#define OTG_DEV_INT_FIFO0_7OUT_DIS()                                    (U32USB(OTG_DEV_MISG1) |= (0x0000FFFFU))
#define OTG_DEV_INT_FIFO0_7IN_DIS()                                     (U32USB(OTG_DEV_MISG1) |= (0x00FF0000U))
#define OTG_DEV_INT_FIFO_OUT_DIS(FIFOn)                                 (U32USB(OTG_DEV_MISG1) |= ( (3U<<((FIFOn)<<1))))
#define OTG_DEV_INT_FIFO_IN_DIS(FIFOn)                                  (U32USB(OTG_DEV_MISG1) |= ( (1U<<((FIFOn)+16))))
#define OTG_DEV_INT_FIFO_OUT_EN(FIFOn)                                  (U32USB(OTG_DEV_MISG1) &= (~(3U<<((FIFOn)<<1))))
#define OTG_DEV_INT_FIFO_IN_EN(FIFOn)                                   (U32USB(OTG_DEV_MISG1) &= (~(1U<<((FIFOn)+16))))
#define OTG_DEV_INT_FIFO0_OUT_DIS()                                     (U32USB(OTG_DEV_MISG1) |= ( (BIT0|BIT1)))
#define OTG_DEV_INT_FIFO1_OUT_DIS()                                     (U32USB(OTG_DEV_MISG1) |= ( (BIT2|BIT3)))
#define OTG_DEV_INT_FIFO2_OUT_DIS()                                     (U32USB(OTG_DEV_MISG1) |= ( (BIT4|BIT5)))
#define OTG_DEV_INT_FIFO3_OUT_DIS()                                     (U32USB(OTG_DEV_MISG1) |= ( (BIT6|BIT7)))
#define OTG_DEV_INT_FIFO4_OUT_DIS()                                     (U32USB(OTG_DEV_MISG1) |= ( (BIT8|BIT9)))
#define OTG_DEV_INT_FIFO5_OUT_DIS()                                     (U32USB(OTG_DEV_MISG1) |= ( (BIT10|BIT11)))
#define OTG_DEV_INT_FIFO6_OUT_DIS()                                     (U32USB(OTG_DEV_MISG1) |= ( (BIT12|BIT13)))
#define OTG_DEV_INT_FIFO7_OUT_DIS()                                     (U32USB(OTG_DEV_MISG1) |= ( (BIT14|BIT15)))
#define OTG_DEV_INT_FIFO0_IN_DIS()                                      (U32USB(OTG_DEV_MISG1) |= ( (BIT16)))
#define OTG_DEV_INT_FIFO1_IN_DIS()                                      (U32USB(OTG_DEV_MISG1) |= ( (BIT17)))
#define OTG_DEV_INT_FIFO2_IN_DIS()                                      (U32USB(OTG_DEV_MISG1) |= ( (BIT18)))
#define OTG_DEV_INT_FIFO3_IN_DIS()                                      (U32USB(OTG_DEV_MISG1) |= ( (BIT19)))
#define OTG_DEV_INT_FIFO4_IN_DIS()                                      (U32USB(OTG_DEV_MISG1) |= ( (BIT20)))
#define OTG_DEV_INT_FIFO5_IN_DIS()                                      (U32USB(OTG_DEV_MISG1) |= ( (BIT21)))
#define OTG_DEV_INT_FIFO6_IN_DIS()                                      (U32USB(OTG_DEV_MISG1) |= ( (BIT22)))
#define OTG_DEV_INT_FIFO7_IN_DIS()                                      (U32USB(OTG_DEV_MISG1) |= ( (BIT23)))
#define OTG_DEV_INT_FIFO0_OUT_EN()                                      (U32USB(OTG_DEV_MISG1) &= (~(BIT0|BIT1)))
#define OTG_DEV_INT_FIFO1_OUT_EN()                                      (U32USB(OTG_DEV_MISG1) &= (~(BIT2|BIT3)))
#define OTG_DEV_INT_FIFO2_OUT_EN()                                      (U32USB(OTG_DEV_MISG1) &= (~(BIT4|BIT5)))
#define OTG_DEV_INT_FIFO3_OUT_EN()                                      (U32USB(OTG_DEV_MISG1) &= (~(BIT6|BIT7)))
#define OTG_DEV_INT_FIFO4_OUT_EN()                                      (U32USB(OTG_DEV_MISG1) &= (~(BIT8|BIT9)))
#define OTG_DEV_INT_FIFO5_OUT_EN()                                      (U32USB(OTG_DEV_MISG1) &= (~(BIT10|BIT11)))
#define OTG_DEV_INT_FIFO6_OUT_EN()                                      (U32USB(OTG_DEV_MISG1) &= (~(BIT12|BIT13)))
#define OTG_DEV_INT_FIFO7_OUT_EN()                                      (U32USB(OTG_DEV_MISG1) &= (~(BIT14|BIT15)))
#define OTG_DEV_INT_FIFO0_IN_EN()                                       (U32USB(OTG_DEV_MISG1) &= (~(BIT16)))
#define OTG_DEV_INT_FIFO1_IN_EN()                                       (U32USB(OTG_DEV_MISG1) &= (~(BIT17)))
#define OTG_DEV_INT_FIFO2_IN_EN()                                       (U32USB(OTG_DEV_MISG1) &= (~(BIT18)))
#define OTG_DEV_INT_FIFO3_IN_EN()                                       (U32USB(OTG_DEV_MISG1) &= (~(BIT19)))
#define OTG_DEV_INT_FIFO4_IN_EN()                                       (U32USB(OTG_DEV_MISG1) &= (~(BIT20)))
#define OTG_DEV_INT_FIFO5_IN_EN()                                       (U32USB(OTG_DEV_MISG1) &= (~(BIT21)))
#define OTG_DEV_INT_FIFO6_IN_EN()                                       (U32USB(OTG_DEV_MISG1) &= (~(BIT22)))
#define OTG_DEV_INT_FIFO7_IN_EN()                                       (U32USB(OTG_DEV_MISG1) &= (~(BIT23)))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_MISG2_OP OTG_DEV_MISG2 (0x13C) Register Operation
 * @brief Register operation, OTG_DEV_MISG2 (0x13C)
 * @{
 */
#define OTG_DEV_INT_SRC2_MASK_RD()                                      (U32USB(OTG_DEV_MISG2))
#define OTG_DEV_INT_SUSPEND_DIS()                                       (U32USB(OTG_DEV_MISG2) |= ( BIT1 ))
#define OTG_DEV_INT_DMA_CMPLT_DIS()                                     (U32USB(OTG_DEV_MISG2) |= ( BIT7 ))
#define OTG_DEV_INT_DMA_ERR_DIS()                                       (U32USB(OTG_DEV_MISG2) |= ( BIT8 ))
#define OTG_DEV_INT_IDLE_DIS()                                          (U32USB(OTG_DEV_MISG2) |= ( BIT9 ))
#define OTG_DEV_INT_WAKEUP_BY_VBUS_DIS()                                (U32USB(OTG_DEV_MISG2) |= ( BIT10))
#define OTG_DEV_INT_SUSPEND_EN()                                        (U32USB(OTG_DEV_MISG2) &= (~BIT1 ))
#define OTG_DEV_INT_DMA_CMPLT_EN()                                      (U32USB(OTG_DEV_MISG2) &= (~BIT7 ))
#define OTG_DEV_INT_DMA_ERR_EN()                                        (U32USB(OTG_DEV_MISG2) &= (~BIT8 ))
#define OTG_DEV_INT_IDLE_EN()                                           (U32USB(OTG_DEV_MISG2) &= (~BIT9 ))
#define OTG_DEV_INT_WAKEUP_BY_VBUS_EN()                                 (U32USB(OTG_DEV_MISG2) &= (~BIT10))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_IGR_OP OTG_DEV_IGR (0x140) Register Operation
 * @brief Register operation, OTG_DEV_IGR (0x140)
 * @{
 */
#define OTG_DEV_INT_GROUP_RD()                                          (U32USB(OTG_DEV_IGR))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_ISG0_OP OTG_DEV_ISG0 (0x144) Register Operation
 * @brief Register operation, OTG_DEV_ISG0 (0x144)
 * @{
 */
#define OTG_DEV_INT_SRC0_RD()                                           (U32USB(OTG_DEV_ISG0))
#define OTG_DEV_INT_EP0_CMD_ABT_CLR()                                   (U32USB(OTG_DEV_ISG0) |= ( BIT5))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_ISG1_OP OTG_DEV_ISG1 (0x148) Register Operation
 * @brief Register operation, OTG_DEV_ISG1 (0x148)
 * @{
 */
#define OTG_DEV_INT_SRC1_RD()                                           (U32USB(OTG_DEV_ISG1))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_ISG2_OP OTG_DEV_ISG2 (0x14C) Register Operation
 * @brief Register operation, OTG_DEV_ISG2 (0x14C)
 * @{
 */
#define OTG_DEV_INT_SRC2_RD()                                           (U32USB(OTG_DEV_ISG2))
#define OTG_DEV_INT_BUS_RST_CLR()                                       (U32USB(OTG_DEV_ISG2) = ( BIT0))
#define OTG_DEV_INT_SUSPEND_CLR()                                       (U32USB(OTG_DEV_ISG2) = ( BIT1))
#define OTG_DEV_INT_RESUME_CLR()                                        (U32USB(OTG_DEV_ISG2) = ( BIT2))
#define OTG_DEV_INT_ISO_SEQ_ERR_CLR()                                   (U32USB(OTG_DEV_ISG2) = ( BIT3))
#define OTG_DEV_INT_ISO_SEQ_ABT_CLR()                                   (U32USB(OTG_DEV_ISG2) = ( BIT4))
#define OTG_DEV_INT_ISO_SEQ_ERR_ABT_CLR()                               (U32USB(OTG_DEV_ISG2) = ( (BIT3|BIT4)))
#define OTG_DEV_INT_TX0BYTE_CLR()                                       (U32USB(OTG_DEV_ISG2) = ( BIT5))
#define OTG_DEV_INT_RX0BYTE_CLR()                                       (U32USB(OTG_DEV_ISG2) = ( BIT6))
#define OTG_DEV_INT_DMA_CMPLT_CLR()                                     (U32USB(OTG_DEV_ISG2) = ( BIT7))
#define OTG_DEV_INT_DMA_ERR_CLR()                                       (U32USB(OTG_DEV_ISG2) = ( BIT8))
#define OTG_DEV_INT_DMA_CMPLT_CHECK()                                   (U32USB(OTG_DEV_ISG2) & ( BIT7))
#define OTG_DEV_INT_DMA_ERR_CHECK()                                     (U32USB(OTG_DEV_ISG2) & ( BIT8))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_RXZ_OP OTG_DEV_RXZ (0x150) Register Operation
 * @brief Register operation, OTG_DEV_RXZ (0x150)
 * @{
 */
#define OTG_DEV_INT_RX0BYTE_EP_CLR_ALL()                                (U32USB(OTG_DEV_RXZ) = 0xFF)
#define OTG_DEV_INT_RX0BYTE_EP_RD()                                     (U32USB(OTG_DEV_RXZ) & 0xFF)
#define OTG_DEV_INT_RX0BYTE_EP_CLR(val)                                 (U32USB(OTG_DEV_RXZ) |= ((val) & 0xFF))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_TXZ_OP OTG_DEV_TXZ (0x154) Register Operation
 * @brief Register operation, OTG_DEV_TXZ (0x154)
 * @{
 */
#define OTG_DEV_INT_TX0BYTE_EP_CLR_ALL()                                (U32USB(OTG_DEV_TXZ) = 0xFF)
#define OTG_DEV_INT_TX0BYTE_EP_RD()                                     (U32USB(OTG_DEV_TXZ) & 0xFF)
#define OTG_DEV_INT_TX0BYTE_RP_CLR(val)                                 (U32USB(OTG_DEV_TXZ) |= ((val) & 0xFF))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_ISE_OP OTG_DEV_ISE (0x158) Register Operation
 * @brief Register operation, OTG_DEV_ISE (0x158)
 * @{
 */
#define OTG_DEV_INT_ISO_SEQ_ERR_ABT_EP_CLR()                            (U32USB(OTG_DEV_ISE) = 0)
#define OTG_DEV_INT_ISO_SEQ_ABT_EP_RD()                                 ((U32USB(OTG_DEV_ISE) >> 0) & 0xFF)
#define OTG_DEV_INT_ISO_SEQ_ERR_EP_RD()                                 ((U32USB(OTG_DEV_ISE) >> 16) & 0xFF)
#define OTG_DEV_INT_ISO_SEQ_ABT_EP_CLR(val)                             (U32USB(OTG_DEV_ISE) &= (~(((uint32_t)val) << 0)))
#define OTG_DEV_INT_ISO_SEQ_ERR_EP_CLR(val)                             (U32USB(OTG_DEV_ISE) &= (~(((uint32_t)val) << 16)))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_INMPS_OP OTG_DEV_INMPS (0x160) Register Operation
 * @brief Register operation, OTG_DEV_INMPS (0x160)
 * @{
 */
#define OTG_DEV_EP_IN_TRANS2_PER_FRAME(EPn)                             \
do {                                                                    \
    U32USB((OTG_DEV_INMPS+(((EPn)-1)<<2))) &= (~(3U<<13));              \
    U32USB((OTG_DEV_INMPS+(((EPn)-1)<<2))) |= (2U<<13);                 \
} while (0)
#define OTG_DEV_EP_IN_TRANS3_PER_FRAME(EPn)                             \
do {                                                                    \
    U32USB((OTG_DEV_INMPS+(((EPn)-1)<<2))) &= (~(3U<<13));              \
    U32USB((OTG_DEV_INMPS+(((EPn)-1)<<2))) |= (3U<<13);                 \
} while (0)
#define OTG_DEV_EP_IN_MAX_PACKET_SET(EPn, size)                         (U32USB((OTG_DEV_INMPS+(((EPn)-1)<<2))) = ((size)&0x7FF))
#define OTG_DEV_EP_IN_MAX_PACKET_RD(EPn)                                (U32USB((OTG_DEV_INMPS+(((EPn)-1)<<2))) & 0x7FF)
#define OTG_DEV_EP_IN_STALL_CHECK(EPn)                                  ((U32USB((OTG_DEV_INMPS+(((EPn)-1)<<2)))>>11) & 0x1)
#define OTG_DEV_EP_IN_STALL_SET(EPn)                                    (U32USB((OTG_DEV_INMPS+(((EPn)-1)<<2))) |= ( BIT11))
#define OTG_DEV_EP_IN_RSTG_SET(EPn)                                     (U32USB((OTG_DEV_INMPS+(((EPn)-1)<<2))) |= ( BIT12))
#define OTG_DEV_EP_IN_STALL_CLR(EPn)                                    (U32USB((OTG_DEV_INMPS+(((EPn)-1)<<2))) &= (~BIT11))
#define OTG_DEV_EP_IN_RSTG_CLR(EPn)                                     (U32USB((OTG_DEV_INMPS+(((EPn)-1)<<2))) &= (~BIT12))
#define OTG_DEV_EP_IN_TX0_EN(EPn)                                       (U32USB((OTG_DEV_INMPS+(((EPn)-1)<<2))) |= ( BIT15))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_OUTMPS_OP OTG_DEV_OUTMPS (0x180) Register Operation
 * @brief Register operation, OTG_DEV_OUTMPS (0x180)
 * @{
 */
#define OTG_DEV_EP_OUT_MAX_PACKET_SET(EPn, size)                        (U32USB((OTG_DEV_OUTMPS+(((EPn)-1)<<2))) = ((size)&0x7FF))
#define OTG_DEV_EP_OUT_MAX_PACKET_RD(EPn)                               (U32USB((OTG_DEV_OUTMPS+(((EPn)-1)<<2))) & 0x7FF)
#define OTG_DEV_EP_OUT_STALL_CHECK(EPn)                                 ((U32USB((OTG_DEV_OUTMPS+(((EPn)-1)<<2)))>>11) & 0x1)
#define OTG_DEV_EP_OUT_STALL_SET(EPn)                                   (U32USB((OTG_DEV_OUTMPS+(((EPn)-1)<<2))) |= ( BIT11))
#define OTG_DEV_EP_OUT_STALL_CLR(EPn)                                   (U32USB((OTG_DEV_OUTMPS+(((EPn)-1)<<2))) &= (~BIT11))
#define OTG_DEV_EP_OUT_RSTG_SET(EPn)                                    (U32USB((OTG_DEV_OUTMPS+(((EPn)-1)<<2))) |= ( BIT12))
#define OTG_DEV_EP_OUT_RSTG_CLR(EPn)                                    (U32USB((OTG_DEV_OUTMPS+(((EPn)-1)<<2))) &= (~BIT12))
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_EPMAP_OP OTG_DEV_EPMAP0/OTG_DEV_EPMAP1 (0x180/0x1A4) Register Operation
 * @brief Register operation, OTG_DEV_EPMAP0/OTG_DEV_EPMAP1 (0x180/0x1A4)
 * @{
 */
#define OTG_DEV_EP_MAP_SET(EPn, MAP)                                    (U8USB((OTG_DEV_EPMAP0+((EPn)-1))) = (MAP))
#define OTG_DEV_EP_MAP_RD(EPn)                                          (U8USB((OTG_DEV_EPMAP0+((EPn)-1))))
#define OTG_DEV_EP_MAP_ALL_CLR()                                        \
do {                                                                    \
    U32USB(OTG_DEV_EPMAP0) = 0;                                         \
    U32USB(OTG_DEV_EPMAP1) = 0;                                         \
} while (0)
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_FMAP_OP OTG_DEV_FMAP1/OTG_DEV_FMAP2 (0x1A8/0x1D8) Register Operation
 * @brief Register operation, OTG_DEV_FMAP1/OTG_DEV_FMAP2 (0x1A8/0x1D8)
 * @{
 */
#define OTG_DEV_FIFO_MAP_SET(FIFOn, MAP)                                (U8USB((OTG_DEV_FMAP1+(0x30*((FIFOn)>>2))+((FIFOn)&0x3))) = (MAP))
#define OTG_DEV_FIFO_MAP_RD(FIFOn)                                      (U8USB((OTG_DEV_FMAP1+(0x30*((FIFOn)>>2))+((FIFOn)&0x3))))
#define OTG_DEV_FIFO_MAP_ALL_CLR()                                      \
do{                                                                     \
    U32USB(OTG_DEV_FMAP1) = 0;                                          \
    U32USB(OTG_DEV_FMAP2) = 0;                                          \
} while (0)
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_FCFG_OP OTG_DEV_FCFG1/OTG_DEV_FCFG2 (0x1AC/0x1DC) Register Operation
 * @brief Register operation, OTG_DEV_FCFG1/OTG_DEV_FCFG2 (0x1AC/0x1DC)
 * @{
 */
#define OTG_DEV_FIFO_CFG_SET(FIFOn, CFG)                                (U8USB((OTG_DEV_FCFG1+(0x30*((FIFOn)>>2))+((FIFOn)&0x3))) = (CFG))
#define OTG_DEV_FIFO_CFG_RD(FIFOn)                                      (U8USB((OTG_DEV_FCFG1+(0x30*((FIFOn)>>2))+((FIFOn)&0x3))))
#define OTG_DEV_FIFO_CFG_ENABLE_CHECK(FIFOn)                            (OTG_DEV_FIFO_CFG_RD(FIFOn) & (1 << 5))
#define OTG_DEV_FIFO_CFG_ALL_CLR()                                      \
do {                                                                    \
    U32USB(OTG_DEV_FCFG1) = 0;                                          \
    U32USB(OTG_DEV_FCFG2) = 0;                                          \
} while (0)
/**
 * @}
 */
/**
 * @defgroup OTG_DEV_FIBC_OP OTG_DEV_FIBC (0x1B0~0x1BC(FIFO0~FIFO3), 0x1F0~0x1FC(FIFO4~FIFO7)) Register Operation
 * @brief Register operation, OTG_DEV_FIBC (0x1B0~0x1BC(FIFO0~FIFO3), 0x1F0~0x1FC(FIFO4~FIFO7))
 * @{
 */
#define OTG_DEV_FIFO_OUT_BYTE_CNT(FIFOn)                                (U32USB((OTG_DEV_FIBC+(0x40*((FIFOn)>>2))+(0x4*((FIFOn)&0x3)))) & 0x7FF)
#define OTG_DEV_FIFO_RESET(FIFOn)                                       (U32USB((OTG_DEV_FIBC+(0x40*((FIFOn)>>2))+(0x4*((FIFOn)&0x3)))) |= (1U<<12))
/**
 * @}
 */
/**
 * @defgroup OTG_DMA_TFN_OP OTG_DMA_TFN (0x1C0) Register Operation
 * @brief Register operation, OTG_DMA_TFN (0x1C0)
 * @{
 */
#define OTG_DEV_DMA_TO_FIFO_NONE()                                      (U32USB(OTG_DMA_TFN) = 0)
#define OTG_DEV_DMA_TO_FIFO_CX_SEL()                                    (U32USB(OTG_DMA_TFN) = (1U<<4))
#define OTG_DEV_DMA_TO_FIFO_SEL(FIFOn)                                  (U32USB(OTG_DMA_TFN) = (1U<<(((FIFOn)>>2)+(FIFOn))))
/**
 * @}
 */
/**
 * @defgroup OTG_DMA_CPS1_OP OTG_DMA_CPS1 (0x1C8) Register Operation
 * @brief Register operation, OTG_DMA_CPS1 (0x1C8)
 * @{
 */
#define OTG_DEV_DMA_CFG_MEM2FIFO(len)                                   (U32USB(OTG_DMA_CPS1) = (((uint32_t)len) << 8) | (1<<1))
#define OTG_DEV_DMA_CFG_FIFO2MEM(len)                                   (U32USB(OTG_DMA_CPS1) = (((uint32_t)len) << 8) | (0<<1))
#define OTG_DEV_DMA_START()                                             (U32USB(OTG_DMA_CPS1) |= ( BIT0))
#define OTG_DEV_DMA_STOP()                                              (U32USB(OTG_DMA_CPS1) &= (~BIT0))
#define OTG_DEV_DMA_RESET()                                             \
do {                                                                    \
    U32USB(OTG_DMA_CPS1) = 0;                                           \
    U32USB(OTG_DMA_CPS1) = BIT3;                                        \
} while (0)
/**
 * @}
 */
/**
 * @defgroup OTG_DMA_CPS2_OP OTG_DMA_CPS2 (0x1CC) Register Operation
 * @brief Register operation, OTG_DMA_CPS2 (0x1CC)
 * @{
 */
#define OTG_DEV_DMA_CFG_MEM(addr)                                       (U32USB(OTG_DMA_CPS2) = (addr))
/**
 * @}
 */
/**
 * @defgroup OTG_DMA_CPS3_OP OTG_DMA_CPS3 (0x1D0) Register Operation
 * @brief Register operation, OTG_DMA_CPS3 (0x1D0)
 * @{
 */
#define OTG_DEV_EP0_CMD_DATA_RD()                                       (U32USB(OTG_DMA_CPS3))
/**
 * @}
 */
/**
 * @}
 */
/**
 * @defgroup GLOBAL_REG Global Register Operation
 * @{
 */
/**
 * @}
 */
/**
 * @}
 */

/* Exported functions -------------------------------------------------------*/
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
