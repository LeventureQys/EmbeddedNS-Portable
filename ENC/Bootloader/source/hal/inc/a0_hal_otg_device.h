/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_otg_device.h
 * @brief   Header file of OTG Device HAL module.
 * @version V1.0.8
 * @date    2023/06/02
 * @author  huangzheng
 ******************************************************************************
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __A0_HAL_OTG_DEVICE_H
#define __A0_HAL_OTG_DEVICE_H

/* Includes -----------------------------------------------------------------*/
#include "a0_hal_otg.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup OTG_DEV
 * @brief OTG module for device
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/**
 * @defgroup OTG_DEV_Exported_Types OTG Device Exported Types
 * @{
 */
/**
 * @brief  Device paramter structure definition
 */
typedef struct {
    uint8_t             isFullSpeed;            /* 0, default HS(480M), 1, config as FS(12M) */

    uint8_t             epNumInUse;
    otg_ep_fifo_cfg_t*  pEpFifoCfg;

    void*               pDevInfo;
    uint8_t             devState;
    uint8_t             devStateSuspendBackup;
    setup_packet_t      devSetupPacket;

    uint16_t            sizeData;
    uint8_t*            pData;
}otg_dev_para_t;

/**
 * @brief  Device visiable states
 */
typedef enum {
    USB_DEV_STATE_ATTACHED = 0,             /*!< Device is in Attached State */
    USB_DEV_STATE_POWERED = 1,              /*!< Device is in Powered State */
    USB_DEV_STATE_DEFAULT = 2,              /*!< Device is in Default State */
    USB_DEV_STATE_ADDRESSED = 3,            /*!< Device is in Addressed State */
    USB_DEV_STATE_CONFIGURED = 4,           /*!< Device is in Configured State */
    USB_DEV_STATE_SUSPENDED = 5,            /*!< Device is in Suspended State */
}USB_DEV_STATE;
/**
 * @}
 */

/* Exported constants -------------------------------------------------------*/
/* Exported macros ----------------------------------------------------------*/
/* Exported functions -------------------------------------------------------*/
/**
 * @addtogroup OTG_DEV_Exported_Functions OTG Device Exported Functions
 * @{
 */
/**
 * @brief  device write to data fifo
 * @param  pDataMem        pointer to src data in memory which need to write to usb fifo
 * @param  size            byte size of write data to fifo
 * @param  fifoIdx         fifo index number, 0~7
 * @return NONE
 */
void HalOtgDevDxFifoWrite(uint8_t* pDataMem, size_t size, uint8_t fifoIdx);
/**
 * @brief  device read from data fifo
 * @param  pDataMem        pointer to dst memory which need to read from usb fifo and write
 * @param  size            byte size of read data to fifo
 * @param  fifoIdx         fifo index number, 0~7
 * @return NONE
 */
void HalOtgDevDxFifoRead(uint8_t* pDataMem, size_t size, uint8_t fifoIdx);
/**
 * @brief  device write to control fifo, for endpoint0
 * @param  pDataMem        pointer to src data in memory which need to write to usb ctrl fifo
 * @param  size            byte size of write data to fifo, max is 64
 * @return NONE
 */
void HalOtgDevCxFifoWrite(uint8_t* pDataMem, size_t size);
/**
 * @brief  device read from control fifo, for endpoint0
 * @param  pDataMem        pointer to dst memory which need to read from usb ctrl fifo and write
 * @param  size            byte size of read data to fifo, max is 64
 * @return NONE
 */
void HalOtgDevCxFifoRead(uint8_t* pDataMem, size_t size);
/**
 * @brief  device endpoint0 tx data to host
 * @param  pPara           pointer to structure @ref otg_dev_para_t
 * @return NONE
 */
void HalOtgDevEp0TxData(otg_dev_para_t *pPara);
/**
 * @brief  device endpoint0 rx data from host
 * @param  pPara           pointer to structure @ref otg_dev_para_t
 * @return NONE
 */
void HalOtgDevEp0RxData(otg_dev_para_t *pPara);
/**
 * @brief  device initial, need to be called before use usb device
 * @param  pPara           pointer to structure @ref otg_dev_para_t
 * @return NONE
 */
void HalOtgDevInit(otg_dev_para_t *pPara);
/**
 * @brief  device irq process, need to be called in IRQ
 * @param  pPara           pointer to structure @ref otg_dev_para_t
 * @return NONE
 */
void HalOtgDevIrq(otg_dev_para_t *pPara);

/* for code compatible */
#define hal_otg_device_dx_fifo_write            HalOtgDevDxFifoWrite
#define hal_otg_device_dx_fifo_read             HalOtgDevDxFifoRead
#define hal_otg_device_cx_fifo_write            HalOtgDevCxFifoWrite
#define hal_otg_device_cx_fifo_read             HalOtgDevCxFifoRead
#define hal_otg_device_ep0_tx_data              HalOtgDevEp0TxData
#define hal_otg_device_ep0_rx_data              HalOtgDevEp0RxData
#define hal_otg_device_init                     HalOtgDevInit
#define hal_otg_device_irq                      HalOtgDevIrq
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
