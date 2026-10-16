/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_otg_device.c
 * @brief   OTG Device HAL module.
 * @version V1.0.8
 * @date    2023/06/02
 * @author  huangzheng
 ******************************************************************************
 */

/**
 * 版本日志：
 * V1.0.8, 2023/06/02
 *      1. 初始化增加对系统控制寄存器中USB的SQ位配置，解决USB的高速和全速下出现通信不稳定情况，HalOtgDevInit中添加第一行。
 * V1.0.7, 2022/12/26
 *      1. HalOtgDevEp0RxData收完数据后增加callback函数调用，并增加弱函数HalOtgDevEp0RxDataCallback定义
 * V1.0.6, 2022/04/07
 *      1. 按照最终版本芯片对应修改
 *      2. 函数命名按照大驼峰方式命名，头文件增加原函数名的define，为了兼容原代码，后续希望渐渐过渡到统一风格书写规范
 *      3. 整理头文件中依赖关系，使用jat_typedef.h作为数据类型的头文件，并把include放到extern "C" 之外
 *      4. 将一些配置选项移到a0_hal_conf.h，统一进行配置
 * V1.0.5, 2022/03/10
 *      1. hal_otg_device_ep_fifo_config函数调整，支持FIFO的BLOCK配置，可以更灵活地配置输入输出的FIFO数量，
 *         如：使用2个FIFO拼成1024使用，或者使用4个FIFO拼成2个1024，进行每微帧2次（或3次）传输，但是此时在ISO数据流传输时，需要对应配置好EP的最大包SIZE
 * V1.0.4, 2021/12/09
 *      1. otg_dev_para_t结构体中增加usb状态的备份，在suspend之后进行状态备份，resume还原回到原来的状态阶段
 * V1.0.3, 2021/10/27
 *      1. 初始化时，增加状态将状态赋值成USB_DEV_STATE_DEFAULT
 * V1.0.2, 2021/10/14
 *      1. IRQ中增加对Group0中断的BIT3处理，解决由于这位触发IRQ中断，导致一致不停进中断的问题
 * V1.0.1, 2021/10/11,
 *      1. 优化调整
 * V1.0.0, 2021/07/06,
 *      1. 在A1基础上，整理Device部分代码
 */
/* Includes -----------------------------------------------------------------*/
#include "a0_hal_conf.h"
/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup OTG_DEV
 * @{
 */
#ifdef HAL_OTG_DEV_MODULE_ENABLED
#include "a0_hal_otg_device.h"
#include "a0_hal_otg.h"
#include "a0_hal_sys_ctrl.h"
#include "a0_hal_def.h"

/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
#ifndef HAL_OTG_PRINT_EN
#define USB_PRINT(...)
#define USB_SCAN(...)
#else
#define USB_PRINT(...)  DBG_PRINT(__VA_ARGS__)
#define USB_SCAN(...)   DBG_SCAN(__VA_ARGS__)
#endif

#ifdef USB_MEM_ADDR_CHECK_EN
#define USB_MEM_ADDR_CHECK(addr)                                            \
    (((addr >= 0x10000000) && (addr < 0x10020000)) ? (0) :                  \
    (((addr >= 0x20000000) && (addr < 0x20008000)) ? (0) : (1)))
#else
#define USB_MEM_ADDR_CHECK(addr)                                            (0)
#endif

/* Private variables --------------------------------------------------------*/
__RAM1 static uint32_t halOtgDevCxBuff[16];         //64个字节，用于CX的DMA访问，固定定义在RAM1

/* Private function prototypes ----------------------------------------------*/
static int32_t HalOtgDevCxFifoAccess(uint32_t memAddr)
{
    OTG_DEV_DMA_TO_FIFO_CX_SEL();
    OTG_DEV_DMA_CFG_MEM(OTG_MEM_ADDR_ADJ(memAddr));
    OTG_DEV_DMA_START();
    int32_t result;
    while (1)
    {
        uint32_t intState = OTG_DEV_INT_SRC2_RD();
        if (intState & (1U<<8))
        {
            /* DMA Error Interrupt */
            OTG_DEV_CX_FIFO_CLR();
            OTG_DEV_INT_DMA_ERR_CLR();
            result = -1;
            USB_PRINT("Error, Line%d, %s, %s()\n", __LINE__, __FILE__, __func__);
            break;
        }
        if (intState & (1U<<7))
        {
            /* DMA Completion Interrupt */
            OTG_DEV_INT_DMA_CMPLT_CLR();
            result = 0;
            break;
        }
        if (intState & (7U<<0))
        {
            /* Resume/Suspend/Reset Interrupt */
            OTG_DEV_DMA_RESET();
            OTG_DEV_CX_FIFO_CLR();
            OTG_DEV_INT_DMA_CMPLT_CLR();
            result = 1;
            USB_PRINT("Error, Line%d, %s, %s()\n", __LINE__, __FILE__, __func__);
            break;
        }
    }
    OTG_DEV_DMA_TO_FIFO_NONE();
    return result;
}

__RAM_CODE static int32_t HalOtgDevDxFifoAccess(uint32_t memAddr, uint32_t fifoIdx)
{
    OTG_DEV_DMA_TO_FIFO_SEL(fifoIdx);
    OTG_DEV_DMA_CFG_MEM(OTG_MEM_ADDR_ADJ(memAddr));
    OTG_DEV_DMA_START();
    int32_t result;
    while (1)
    {
        uint32_t intState = OTG_DEV_INT_SRC2_RD();
        if (intState & (1U<<8))
        {
            /* DMA Error Interrupt */
            OTG_DEV_FIFO_RESET(fifoIdx);
            OTG_DEV_INT_DMA_ERR_CLR();
            result = -1;
            USB_PRINT("Error, Line%d, %s, %s()\n", __LINE__, __FILE__, __func__);
            break;
        }
        if (intState & (1U<<7))
        {
            /* DMA Completion Interrupt */
            OTG_DEV_INT_DMA_CMPLT_CLR();
            result = 0;
            break;
        }
        if (intState & (7U<<0))
        {
            /* Resume/Suspend/Reset Interrupt */
            OTG_DEV_DMA_RESET();
            OTG_DEV_FIFO_RESET(fifoIdx);
            OTG_DEV_INT_DMA_CMPLT_CLR();
            result = 1;
            USB_PRINT("Error, Line%d, %s, %s()\n", __LINE__, __FILE__, __func__);
            break;
        }
    }
    OTG_DEV_DMA_TO_FIFO_NONE();
    return result;
}

static void HalOtgDevEpFifoConfig(otg_dev_para_t *pPara)
{
    otg_ep_fifo_cfg_t *pCfg = pPara->pEpFifoCfg;

    for (uint8_t i = 0; i < pPara->epNumInUse; i++, pCfg++)
    {
        uint8_t fifoCfg = ((pCfg->fifo1024 << 4) | (pCfg->fifoBlockNum << 2) | (pCfg->transType << 0));
        uint8_t epMap = 0;
        if (pCfg->fifoInIdx < OTG_MAX_FIFO_NUM)
        {
            uint8_t fifoMap = (0x10 | pCfg->epIdx);
            epMap |= pCfg->fifoInIdx;
            if (pCfg->fifo1024 != 0)
            {
                uint8_t fifoIdx = pCfg->fifoInIdx;
                OTG_DEV_FIFO_MAP_SET(fifoIdx, fifoMap);
                while (OTG_DEV_FIFO_MAP_RD(fifoIdx) != fifoMap);
                OTG_DEV_FIFO_CFG_SET(fifoIdx, (fifoCfg | (1 << 5)));
                while (OTG_DEV_FIFO_CFG_RD(fifoIdx) != (fifoCfg | (1 << 5)));

                fifoIdx++;
                OTG_DEV_FIFO_MAP_SET(fifoIdx, fifoMap);
                while (OTG_DEV_FIFO_MAP_RD(fifoIdx) != fifoMap);
                OTG_DEV_FIFO_CFG_SET(fifoIdx, (fifoCfg | (0 << 5)));
                while (OTG_DEV_FIFO_CFG_RD(fifoIdx) != (fifoCfg | (0 << 5)));

                for (uint8_t j = 0; j < (pCfg->fifoBlockNum * 2); j++)
                {
                    fifoIdx++;
                    OTG_DEV_FIFO_MAP_SET((fifoIdx), fifoMap);
                    while (OTG_DEV_FIFO_MAP_RD(fifoIdx) != fifoMap);
                    OTG_DEV_FIFO_CFG_SET((fifoIdx), (fifoCfg | (0 << 5)));
                    while (OTG_DEV_FIFO_CFG_RD(fifoIdx) != (fifoCfg | (0 << 5)));
                }
                /*
                 * 暂时按照1024的大小配置，实际传输中，在切换AltSetting或者采样率改变的时候（涉及到需要进行两次或三次传输的，需要调整大小和描述符中的断点包大小一致）
                 * 1. 调用OTG_DEV_EP_IN_MAX_PACKET_SET(EPn, size)，配置每次传输包大小
                 * 2. 调用OTG_DEV_EP_IN_TRANS2_PER_FRAME(EPn)或OTG_DEV_EP_IN_TRANS3_PER_FRAME(EPn)，配置每个微帧进行2次还是3次传输（如果只需要1次传输，配置完包大小即可，此函数无需调用）
                 */
                OTG_DEV_EP_IN_MAX_PACKET_SET(pCfg->epIdx, 0x400);
                while (OTG_DEV_EP_IN_MAX_PACKET_RD(pCfg->epIdx) != 0x400);
            }
            else
            {
                uint8_t fifoIdx = pCfg->fifoInIdx;
                OTG_DEV_FIFO_MAP_SET(fifoIdx, fifoMap);
                while (OTG_DEV_FIFO_MAP_RD(fifoIdx) != fifoMap);
                OTG_DEV_FIFO_CFG_SET(fifoIdx, (fifoCfg | (1 << 5)));
                while (OTG_DEV_FIFO_CFG_RD(fifoIdx) != (fifoCfg | (1 << 5)));

                for (uint8_t j = 0; j < (pCfg->fifoBlockNum * 1); j++)
                {
                    fifoIdx++;
                    OTG_DEV_FIFO_MAP_SET((fifoIdx), fifoMap);
                    while (OTG_DEV_FIFO_MAP_RD(fifoIdx) != fifoMap);
                    OTG_DEV_FIFO_CFG_SET((fifoIdx), (fifoCfg | (0 << 5)));
                    while (OTG_DEV_FIFO_CFG_RD(fifoIdx) != (fifoCfg | (0 << 5)));
                }
                /*
                 * 暂时按照512的大小配置，实际传输中，在切换AltSetting或者采样率改变的时候（涉及到需要进行两次或三次传输的，需要调整大小和描述符中的断点包大小一致）
                 * 1. 调用OTG_DEV_EP_IN_MAX_PACKET_SET(EPn, size)，配置每次传输包大小
                 * 2. 调用OTG_DEV_EP_IN_TRANS2_PER_FRAME(EPn)或OTG_DEV_EP_IN_TRANS3_PER_FRAME(EPn)，配置每个微帧进行2次还是3次传输（如果只需要1次传输，配置完包大小即可，此函数无需调用）
                 */
                OTG_DEV_EP_IN_MAX_PACKET_SET(pCfg->epIdx, 0x200);
                while (OTG_DEV_EP_IN_MAX_PACKET_RD(pCfg->epIdx) != 0x200);
            }
        }
        if (pCfg->fifoOutIdx < OTG_MAX_FIFO_NUM)
        {
            uint8_t fifoMap = (0x00 | pCfg->epIdx);
            epMap |= (pCfg->fifoOutIdx << 4);
            if (pCfg->fifo1024 != 0)
            {
                uint8_t fifoIdx = pCfg->fifoOutIdx;
                OTG_DEV_FIFO_MAP_SET(fifoIdx, fifoMap);
                while (OTG_DEV_FIFO_MAP_RD(fifoIdx) != fifoMap);
                OTG_DEV_FIFO_CFG_SET(fifoIdx, (fifoCfg | (1 << 5)));
                while (OTG_DEV_FIFO_CFG_RD(fifoIdx) != (fifoCfg | (1 << 5)));

                fifoIdx++;
                OTG_DEV_FIFO_MAP_SET(fifoIdx, fifoMap);
                while (OTG_DEV_FIFO_MAP_RD(fifoIdx) != fifoMap);
                OTG_DEV_FIFO_CFG_SET(fifoIdx, (fifoCfg | (0 << 5)));
                while (OTG_DEV_FIFO_CFG_RD(fifoIdx) != (fifoCfg | (0 << 5)));

                for (uint8_t j = 0; j < (pCfg->fifoBlockNum * 2); j++)
                {
                    fifoIdx++;
                    OTG_DEV_FIFO_MAP_SET((fifoIdx), fifoMap);
                    while (OTG_DEV_FIFO_MAP_RD(fifoIdx) != fifoMap);
                    OTG_DEV_FIFO_CFG_SET((fifoIdx), (fifoCfg | (0 << 5)));
                    while (OTG_DEV_FIFO_CFG_RD(fifoIdx) != (fifoCfg | (0 << 5)));
                }
                /*
                 * 暂时按照1024的大小配置，实际传输中，在切换AltSetting或者采样率改变的时候（涉及到需要进行两次或三次传输的，需要调整大小和描述符中的断点包大小一致）
                 * 1. 调用OTG_DEV_EP_OUT_MAX_PACKET_SET(EPn, size)，配置每次传输包大小
                 */
                OTG_DEV_EP_OUT_MAX_PACKET_SET(pCfg->epIdx, 0x400);
                while (OTG_DEV_EP_OUT_MAX_PACKET_RD(pCfg->epIdx) != 0x400);
            }
            else
            {
                uint8_t fifoIdx = pCfg->fifoOutIdx;
                OTG_DEV_FIFO_MAP_SET(fifoIdx, fifoMap);
                while (OTG_DEV_FIFO_MAP_RD(fifoIdx) != fifoMap);
                OTG_DEV_FIFO_CFG_SET(fifoIdx, (fifoCfg | (1 << 5)));
                while (OTG_DEV_FIFO_CFG_RD(fifoIdx) != (fifoCfg | (1 << 5)));

                for (uint8_t j = 0; j < (pCfg->fifoBlockNum * 1); j++)
                {
                    fifoIdx++;
                    OTG_DEV_FIFO_MAP_SET((fifoIdx), fifoMap);
                    while (OTG_DEV_FIFO_MAP_RD(fifoIdx) != fifoMap);
                    OTG_DEV_FIFO_CFG_SET((fifoIdx), (fifoCfg | (0 << 5)));
                    while (OTG_DEV_FIFO_CFG_RD(fifoIdx) != (fifoCfg | (0 << 5)));
                }
                /*
                 * 暂时按照512的大小配置，实际传输中，在切换AltSetting或者采样率改变的时候（涉及到需要进行两次或三次传输的，需要调整大小和描述符中的断点包大小一致）
                 * 1. 调用OTG_DEV_EP_OUT_MAX_PACKET_SET(EPn, size)，配置每次传输包大小
                 */
                OTG_DEV_EP_OUT_MAX_PACKET_SET(pCfg->epIdx, 0x200);
                while (OTG_DEV_EP_OUT_MAX_PACKET_RD(pCfg->epIdx) != 0x200);
            }
            OTG_DEV_INT_FIFO_OUT_EN(pCfg->fifoOutIdx);
        }
        OTG_DEV_EP_MAP_SET(pCfg->epIdx, epMap);
        while (OTG_DEV_EP_MAP_RD(pCfg->epIdx) != epMap);
    }
    return;
}

/* Exported functions -------------------------------------------------------*/
__WEAK void HalOtgDevReset(void)
{
    /* add user code to handle after reset */
    return;
}

__WEAK void HalOtgDevSuspend(void)
{
    /* add user code to stop usb-mic and usb-spk */
    return;
}

__WEAK void HalOtgDevResume(void)
{
    /* clear remote wakeup signal */
    HAL_SYS_USB_REMOTE_WAKEUP_CLR();
    return;
}

__WEAK void HalOtgDevEp0Setup(otg_dev_para_t *pPara)
{
    /* user can rewrite this function in DRIVER */
    /* rewrite start */
    OTG_DEV_EP0_STALL_SET();
    /* rewrite end */
    return;
}

__WEAK __RAM_CODE void HalOtgDevEp0RxDataCallback(void)
{
    return;    
}

__WEAK void HalOtgDevFifoCallback(uint8_t fifoIdx, uint32_t in, uint32_t out)
{
    return;
}

void HalOtgDevCxFifoWrite(uint8_t* pDataMem, size_t size)
{
    /* write data to cx fifo */
    if (size != 0)
    {
        memcpy(halOtgDevCxBuff, pDataMem, size);
        OTG_DEV_DMA_CFG_MEM2FIFO(size);
        HalOtgDevCxFifoAccess((uint32_t)&halOtgDevCxBuff[0]);
    }
    return;
}

void HalOtgDevCxFifoRead(uint8_t* pDataMem, size_t size)
{
    /* read data from cx fifo */
    if (size != 0)
    {
        OTG_DEV_DMA_CFG_FIFO2MEM(size);
        HalOtgDevCxFifoAccess((uint32_t)&halOtgDevCxBuff[0]);
        memcpy(pDataMem, halOtgDevCxBuff, size);
    }
    return;
}

__RAM_CODE void HalOtgDevDxFifoWrite(uint8_t* pDataMem, size_t size, uint8_t fifoIdx)
{
    if (size != 0)
    {
        uint32_t memAddr = (uint32_t)pDataMem;
        if (USB_MEM_ADDR_CHECK(memAddr) == 0)
        {
            OTG_DEV_DMA_CFG_MEM2FIFO(size);
            HalOtgDevDxFifoAccess(memAddr, fifoIdx);
        }
        else
        {
            USB_PRINT("Error, Line%d, %s, %s()\n", __LINE__, __FILE__, __func__);
            //DX访问的Memory地址不在OTG可访问范围内
            __breakpoint(0);
        }
    }
    return;
}
__RAM_CODE void HalOtgDevDxFifoRead(uint8_t* pDataMem, size_t size, uint8_t fifoIdx)
{
    if (size != 0)
    {
        uint32_t memAddr = (uint32_t)pDataMem;
        if (USB_MEM_ADDR_CHECK(memAddr) == 0)
        {
            OTG_DEV_DMA_CFG_FIFO2MEM(size);
            HalOtgDevDxFifoAccess(memAddr, fifoIdx);
        }
        else
        {
            USB_PRINT("Error, Line%d, %s, %s()\n", __LINE__, __FILE__, __func__);
            //DX访问的Memory地址不在OTG可访问范围内
            __breakpoint(0);
        }
    }
    return;
}

void HalOtgDevEp0TxData(otg_dev_para_t *pPara)
{
    if (pPara->sizeData != 0)
    {
        uint8_t size;
        if (pPara->sizeData < OTG_MAX_EP0_PACK_SIZE)
        {
            size = pPara->sizeData;
        }
        else
        {
            size = OTG_MAX_EP0_PACK_SIZE;
        }
        pPara->sizeData -= size;
        HalOtgDevCxFifoWrite(pPara->pData, size);
        pPara->pData += size;

        if (pPara->sizeData == 0)
        {
            OTG_DEV_EP0_DONE_SET();
        }
    }
    else
    {
        OTG_DEV_EP0_STALL_SET();
    }
    return;
}

void HalOtgDevEp0RxData(otg_dev_para_t *pPara)
{
    if (pPara->sizeData != 0)
    {
        uint8_t size;
        if (pPara->sizeData < OTG_MAX_EP0_PACK_SIZE)
        {
            size = pPara->sizeData;
        }
        else
        {
            size = OTG_MAX_EP0_PACK_SIZE;
        }
        pPara->sizeData -= size;
        HalOtgDevCxFifoRead(pPara->pData, size);
        pPara->pData += size;

        if (pPara->sizeData == 0)
        {
            OTG_DEV_EP0_DONE_SET();
        }
        HalOtgDevEp0RxDataCallback();
    }
    else
    {
        OTG_DEV_EP0_STALL_SET();
    }
    return;
}

void HalOtgDevInit(otg_dev_para_t *pPara)
{
    SYS_CTRL->SYS_USB_CFG1 = 0x1;

    pPara->devState = USB_DEV_STATE_DEFAULT;

    OTG_DEV_UNPLUG_SET();

    /* disable system irq first */
    OTG_IRQ_DISABLE();

    /* write 1 to clear interrupt flag */
    OTGC_INT_STATE_CLR(OTGC_INT_STATE_RD());
    /* disable all interrupt */
    OTGC_INT_EN_CLR(0xFFFFFFFF);
    /* enable the interrupt of usb as device mode */
    OTGC_INT_EN_SET(OTGC_INT_B_TYPE);
    /* config usb device as HS or FS */
    if (pPara->isFullSpeed != 0)
    {
        OTG_DEV_FS_SET();
    }
    else
    {
        OTG_DEV_HS_SET();
    }
    /* config suspend delay time, 0~7ms */
    OTG_DEV_IDLE_CNT_SET(7);
    /* Clear interrupt */
    OTG_DEV_INT_BUS_RST_CLR();
    OTG_DEV_INT_SUSPEND_CLR();
    OTG_DEV_INT_RESUME_CLR();
    /* Disable all fifo interrupt */
    OTG_DEV_INT_FIFO0_7OUT_DIS();
    OTG_DEV_INT_FIFO0_7IN_DIS();
    /* Clear all fifo, write 1 to clear */
    OTG_DEV_CLR_ALL_FIFO();
    /* disable the interrupt below */
    OTG_DEV_INT_WAKEUP_BY_VBUS_DIS();
    OTG_DEV_INT_IDLE_DIS();
    /* enable the function below */
    OTG_DEV_REMOTE_WAKEUP_SET();
    OTG_DEV_GLOBAL_INT_EN();
    OTG_DEV_CHIP_EN();
    /* software reset and wait reset complete */
    OTG_DEV_SOFT_RST();
    while (OTG_DEV_SOFT_RST_CHECK() != 0);

    HalOtgDevTickDelay(10);

    HalOtgDevEpFifoConfig(pPara);
    /* enable system irq at last */
    OTG_IRQ_ENABLE();

    /* disable force unplug setting */
    OTG_DEV_UNPLUG_CLR();

    return;
}

__RAM_CODE void HalOtgDevIrq(otg_dev_para_t *pPara)
{
    /* clear otg ctrl int */
    OTGC_INT_STATE_CLR(OTGC_INT_STATE_RD());

    /* check whether gloabl interrupt is enabled */
    if (OTG_DEV_GLOBAL_INT_EN_CHECK() != 0)
    {
        uint32_t intLevel1 = (OTG_DEV_INT_GROUP_RD() & (~(OTG_DEV_INT_GROUP_MASK_RD())));
        if (intLevel1 != 0)
        {
            OTG_IRQ_DISABLE();
            uint32_t intLevel2;
            if (intLevel1 & (1U << 2))                      /* Group Source 2, Device */
            {
                intLevel2 = (OTG_DEV_INT_SRC2_RD() & (~(OTG_DEV_INT_SRC2_MASK_RD())));
                if (intLevel2 & (1U << 0))                  /* Reset */
                {
                    HalOtgDevReset();
                    /* all fifo in interrupt disable */
                    OTG_DEV_INT_FIFO0_7IN_DIS();
                    /* interrupt clear of bus reset */
                    OTG_DEV_INT_BUS_RST_CLR();
                    /* all fifo clear */
                    OTG_DEV_CLR_ALL_FIFO();
                    /* USB Stage Change*/
                    pPara->devState = USB_DEV_STATE_DEFAULT;
                }
                if (intLevel2 & (1U << 1))                  /* Suspend */
                {
                    HalOtgDevSuspend();
                    /* all fifo in interrupt disable */
                    OTG_DEV_INT_FIFO0_7IN_DIS();
                    /* interrupt clear of suspend */
                    OTG_DEV_INT_SUSPEND_CLR();
                    /* ctrl PHY to enter suspend mode */
                    OTG_DEV_GO_SUSPEND();
                    /* USB Stage Change*/
                    pPara->devStateSuspendBackup = pPara->devState;
                    pPara->devState = USB_DEV_STATE_SUSPENDED;
                }
                if (intLevel2 & (1U << 2))                  /* Resume */
                {
                    HalOtgDevResume();
                    /* interrupt clear of resume */
                    OTG_DEV_INT_RESUME_CLR();
                    /* USB Stage Change*/
                    pPara->devState = pPara->devStateSuspendBackup;
                }
                if (intLevel2 & (3U << 3))                  /* Isochronous Sequential Error/Abort */
                {
                    OTG_DEV_INT_ISO_SEQ_ERR_ABT_CLR();
                    OTG_DEV_INT_ISO_SEQ_ERR_ABT_EP_CLR();
                }
                if (intLevel2 & (1U << 5))                  /* Transferred Zero-length Data Packet */
                {
                    OTG_DEV_INT_TX0BYTE_CLR();
                    OTG_DEV_INT_TX0BYTE_EP_CLR_ALL();
                }
                if (intLevel2 & (1U << 6))                  /* Received Zero-length Data Packet */
                {
                    OTG_DEV_INT_RX0BYTE_CLR();
                    OTG_DEV_INT_RX0BYTE_EP_CLR_ALL();
                }
                if (intLevel2 & (1U << 7))                  /* DMA Completion */
                {
                    OTG_DEV_INT_DMA_CMPLT_CLR();
                }
                if (intLevel2 & (1U << 8))                  /* DMA Error */
                {
                    OTG_DEV_INT_DMA_ERR_CLR();
                }
            }


            if (intLevel1 & (1U << 1))                      /* Group Source 1 */
            {
                intLevel2 = (OTG_DEV_INT_SRC1_RD() & (~(OTG_DEV_INT_SRC1_MASK_RD())));
                uint32_t maskIn = (1U << 16);
                uint32_t maskOut = (3U << 0);
                for (uint8_t i = 0; i < OTG_MAX_FIFO_NUM; i++)
                {
                    HalOtgDevFifoCallback(i, (intLevel2 & maskIn), (intLevel2 & maskOut));
                    maskIn <<= 1;
                    maskOut <<= 2;
                }
            }		


			if (intLevel1 & (1U << 0))                      /* Group Source 0, Cx */
            {
                intLevel2 = (OTG_DEV_INT_SRC0_RD() & (~(OTG_DEV_INT_SRC0_MASK_RD())));
                if (intLevel2 & (1U << 5))                  /* Cx(EP0) Cmd Abort */
                {
                    OTG_DEV_INT_EP0_CMD_ABT_CLR();
                }
                if (intLevel2 & (1U << 0))                  /* Cx(EP0) Setup */
                {
                    HalOtgDevEp0Setup(pPara);
                }
                if (intLevel2 & (1U << 1))                  /* Cx(EP0) In */
                {
                    HalOtgDevEp0TxData(pPara);
                }
                if (intLevel2 & (1U << 2))                  /* Cx(EP0) Out */
                {
                    HalOtgDevEp0RxData(pPara);
                }
//                if (intLevel2 & (1U << 3))
//                {
//                    OTG_DEV_EP0_DONE_SET();
//                }
//                if (intLevel2 & (1U << 4))                  /* Cx(EP0) */
//                {
//                    OTG_DEV_EP0_STALL_SET();
//                }
            }
			
//            if (intLevel1 & (1U << 1))                      /* Group Source 1 */
//            {
//                intLevel2 = (OTG_DEV_INT_SRC1_RD() & (~(OTG_DEV_INT_SRC1_MASK_RD())));
//                uint32_t maskIn = (1U << 16);
//                uint32_t maskOut = (3U << 0);
//                for (uint8_t i = 0; i < OTG_MAX_FIFO_NUM; i++)
//                {
//                    HalOtgDevFifoCallback(i, (intLevel2 & maskIn), (intLevel2 & maskOut));
//                    maskIn <<= 1;
//                    maskOut <<= 2;
//                }
//            }

            OTG_IRQ_ENABLE();
        }
    }
    return;
}

#endif
/**
 * @}
 */
/**
 * @}
 */

