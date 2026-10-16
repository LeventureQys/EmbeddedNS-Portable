/**
 ******************************************************************************
 *
 *         Copyright (C) 2021 JinAudio Technology (shanghai) Co.,LTD.
 *
 ******************************************************************************
 * @file    a0_hal_otg_host.h
 * @brief   Header file of OTG Host HAL module.
 * @version V1.0.3
 * @date    2022/05/27
 * @author  huangzheng
 ******************************************************************************
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __A0_HAL_OTG_HOST_H
#define __A0_HAL_OTG_HOST_H

/* Includes -----------------------------------------------------------------*/
#include "a0_hal_otg.h"

/**
 * @addtogroup A0_HAL_Driver
 * @{
 */
/**
 * @addtogroup OTG_HOST
 * @brief OTG module for host
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types -----------------------------------------------------------*/
/**
 * @defgroup OTG_HOST_Exported_Types OTG Host Exported Types
 * @{
 */
/**
 * @brief  qHD structure definition
 * @note   the base address[31:5] of this struct will write to 0x28(offset) register, so here must aligend to 32 bytes
 */
/*
    该结构体实际需要48个字节，但是由于地址需要写入0x28寄存器，地址低5位无效，实际上是要32字节对齐，
    但是声明结构体数组的时候，要保证每个结构体元素首地址都是32字节对齐，因此这里补齐到64字节，
*/
typedef struct __ALIGNED(32)
{
    //<1>.Next_qHD_Pointer Word
    uint32_t    bTerminate:1;
    uint32_t    bType:2;
    uint32_t    bReserve_1:2;
    uint32_t    bNextQHDPointer:27;
    //<2>.qHD_2 Word
    uint32_t    bDeviceAddress:7;
    uint32_t    bInactiveOnNextTransaction:1;
    uint32_t    bEdNumber:4;
    uint32_t    bEdSpeed:2;
    uint32_t    bDataToggleControl:1;
    uint32_t    bHeadOfReclamationListFlag:1;
    uint32_t    bMaxPacketSize:11;
    uint32_t    bControlEdFlag:1;
    uint32_t    bNakCounter:4;
    //<3>.qHD_3 Word
    uint32_t    bInterruptScheduleMask:8;
    uint32_t    bSplitTransactionMask:8;
    uint32_t    bHubAddr:7;
    uint32_t    bPortNumber:7;
    uint32_t    bHighBandwidth:2;
    //<4>.Overlay_CurrentqTD
    uint32_t    bOverlay_CurrentqTD;
    //<5>.Overlay_NextqTD
    uint32_t    bOverlay_NextTerminate:1;
    uint32_t    bOverlay_Reserve2:4;
    uint32_t    bOverlay_NextqTD:27;
    //<6>.Overlay_AlternateNextqTD
    uint32_t    bOverlay_AlternateNextTerminate:1;
    uint32_t    bOverlay_NanCnt:4;
    uint32_t    bOverlay_AlternateqTD:27;
    //<7>.Overlay_TotalBytes
    uint32_t    bOverlay_Status:8;
    uint32_t    bOverlay_PID:2;
    uint32_t    bOverlay_ErrorCounter:2;
    uint32_t    bOverlay_C_Page:3;
    uint32_t    bOverlay_InterruptOnComplete:1;
    uint32_t    bOverlay_TotalBytes:15;
    uint32_t    bOverlay_Direction:1;
    //<8>.Overlay_BufferPointer0
    uint32_t    bOverlay_CurrentOffset:12;
    uint32_t    bOverlay_BufferPointer_0:20;
    //<9>.Overlay_BufferPointer1
    uint32_t    bOverlay_C_Prog_Mask:8;
    uint32_t    bOverlay_Reserve3:4;
    uint32_t    bOverlay_BufferPointer_1:20;
    //<10>.Overlay_BufferPointer2
    uint32_t    bOverlay_FrameTag:5;
    uint32_t    bOverlay_S_Bytes:7;
    uint32_t    bOverlay_BufferPointer_2:20;
    //<11>.Overlay_BufferPointer3
    uint32_t    bOverlay_Reserve4:12;
    uint32_t    bOverlay_BufferPointer_3:20;
    //<12>.Overlay_BufferPointer4
    uint32_t    bOverlay_Reserve5:12;
    uint32_t    bOverlay_BufferPointer_4:20;
    //<13-16>
    uint32_t    reserved[4];
}otg_host_qHD_t;

/**
 * @brief  Queue Head and Queue Element Transfer Descriptor (qTD) structure definition
 */
typedef struct __ALIGNED(32)
{
    //<1>.Next_qTD_Pointer Word
    uint32_t    bTerminate:1;
    uint32_t    bReserve_1:4;
    uint32_t    bNextQTDPointer:27;
    //<2>.Alternate Next qTD Word
    uint32_t    bAlternateTerminate:1;
    uint32_t    bReserve_2:4;
    uint32_t    bAlternateQTDPointer:27;
    //<3>.Status Word
    uint32_t    bStatus_PingState:1;
    uint32_t    bStatus_SplitState:1;
    uint32_t    bStatus_MissMicroFrame:1;
    uint32_t    bStatus_Transaction_Err:1;
    uint32_t    bStatus_Babble:1;
    uint32_t    bStatus_Buffer_Err:1;
    uint32_t    bStatus_Halted:1;
    uint32_t    bStatus_Active:1;
    uint32_t    bPID:2;
    uint32_t    bErrorCounter:2;
    uint32_t    CurrentPage:3;
    uint32_t    bInterruptOnComplete:1;
    uint32_t    bTotalBytes:15;
    uint32_t    bDataToggle:1;
    //<4>.Buffer Pointer Word Array
    uint32_t    ArrayBufferPointer_Word[5];
}otg_host_qTD_t;

/**
 * @brief  ISO and Split Transaction ISO Transfer Descriptor (iTD/siTD) structure definition
 */
typedef union __ALIGNED(32)
{
    struct {
        //<1>.Next_Link_Pointer Word
        uint32_t    bTerminate:1;
        uint32_t    bType:2;
        uint32_t    bReserve_1:2;
        uint32_t    bNextLinkPointer:27;
        //<2>.Status Word
        struct {
            uint32_t    bOffset:12;
            uint32_t    bPageSelect:3;
            uint32_t    bInterruptOnComplete:1;
            uint32_t    bLength:12;
            uint32_t    bStatus:4;
        }ArrayStatus_Word[8];
        //<3>.Buffer_Pointer Word
        struct {
            uint32_t    bMultiFunction:12;
            uint32_t    bBufferPointer:20;
        }ArrayBufferPointer_Word[7];
    }iTD;
    struct {
        //<1>.1 Word
        uint32_t    bTerminate:1;
        uint32_t    bType:2;
        uint32_t    bReserve_1:2;
        uint32_t    bNextQHDPointer:27;
        //<2>.2 Word
        uint32_t    bDeviceAddress:7;
        uint32_t    bReserve_21:1;
        uint32_t    bEdNumber:4;
        uint32_t    bReserve_22:4;
        uint32_t    bHubAddr:7;
        uint32_t    bReserve_23:1;
        uint32_t    bPortNumber:7;
        uint32_t    bInOut:1;
        //<3>.3 Word
        uint32_t    bS_Mask:8;
        uint32_t    bC_Mask:8;
        uint32_t    bReserve_31:16;
        //<4>.4 Word
        uint32_t    bStatus:8;
        uint32_t    bC_Prog_Mask:8;
        uint32_t    bTotalBytesToTransfer:10;
        uint32_t    bReserve_41:4;
        uint32_t    bPageSelect:1;
        uint32_t    bInterruptOnComplete:1;
        //<5>.5 Word
        uint32_t    bCurrentOffset:12;
        uint32_t    bBufferPointer_Page0:20;
        //<6>.6 Word
        uint32_t    bT_Count:3;
        uint32_t    bTransactionPosition:2;
        uint32_t    bReserve_61:7;
        uint32_t    bBufferPointer_Page1:20;
        //<6>.7 Word
        uint32_t    bBP_Terminate:1;
        uint32_t    bReserve_71:4;
        uint32_t    bBackPointer:27;
    }siTD;
}otg_host_iTD_t;

/**
 * @brief  frame list element pointer structure definition
 */
typedef struct
{
    uint32_t    bTerminate:1;
    uint32_t    bType:2;
    uint32_t    bReserved:2;
    uint32_t    bNextLinkPointer:27;
}otg_host_frame_list_pointer_t;

/**
 * @brief  periodic frame list structure definition
 * @note   the base address[31:12] of this struct will write to 0x24(offset) register, so here must aligend to 4096 bytes
 *         here defined 1024 cells, if want to enlarge the cell number, (1024*n) is a suitable number (n = 1,2,3~)
 */
typedef struct __ALIGNED(4096)
{
    struct {
        //<1>.Next_Link_Pointer Word
        uint32_t    bTerminal:1;
        uint32_t    bType:2;
        uint32_t    bReserved:2;
        uint32_t    bLinkPointer:27;
    }sCell[1024];
}otg_host_periodic_frame_list_t;

/**
 * @brief  Device Descriptor structure definition
 */
typedef struct
{
    uint8_t     bLength;                    /*!< Size of this Descriptor in Bytes */
    uint8_t     bDescriptorType;            /*!< Descriptor Type (=USB_DSC_DEVICE) */
    uint8_t     bcdUSBLowByte;              /*!< USB Spec Release Number in BCD */
    uint8_t     bcdUSBHighByte;             /*!< USB Spec Release Number in BCD */
    uint8_t     bDeviceClass;               /*!< Device Class Code */
    uint8_t     bDeviceSubClass;            /*!< Device Subclass Code */
    uint8_t     bDeviceProtocol;            /*!< Device Protocol Code */
    uint8_t     bMaxPacketSize0;            /*!< Maximum Packet Size for EP0 */
    uint8_t     idVendorLowByte;            /*!< Vendor ID */
    uint8_t     idVendorHighByte;           /*!< Vendor ID */
    uint8_t     idProductLowByte;           /*!< Product ID */
    uint8_t     idProductHighByte;          /*!< Product ID */
    uint8_t     bcdDeviceLowByte;           /*!< Device Release Number in BCD */
    uint8_t     bcdDeviceHighByte;          /*!< Device Release Number in BCD */
    uint8_t     iManufacturer;              /*!< Index of String Desc for Manufacturer */
    uint8_t     iProduct;                   /*!< Index of String Desc for Product */
    uint8_t     iSerialNumber;              /*!< Index of String Desc for SerNo */
    uint8_t     bNumConfigurations;         /*!< Number of possible Configurations */
}otg_host_dsc_dev_t;

/**
 * @brief  qHD parameter structure definition
 */
typedef struct
{
    uint8_t     bNextType;
    uint8_t     bAddress;
    uint8_t     bHead;
    uint8_t     bEndPt;
    uint32_t    wMaxPacketSize;
}otg_host_qhd_para_t;

/**
 * @brief  CTRL transfer parameter structure definition
 */
typedef struct
{
    uint8_t     bEdNum;
    uint16_t    hwDataSize;
    uint8_t*    pbCmd;
    uint8_t*    pbData;
}otg_host_ctrl_para_t;

/**
 * @brief  ISO transfer parameter structure definition
 */
typedef struct {
    uint32_t            wFrameBufferAddress;            //除了初始化，其他没有用到的地方
    uint32_t            wFrameBufferAddressOffset;      //除了初始化，其他没有用到的地方
    uint32_t            wSize;                          //除了初始化，其他没有用到的地方
    uint32_t            wReceiveCounter;
    uint32_t            CurrentiTDNum;
}otg_host_iso_para_t;

/**
 * @brief  device attached information structure definition
 */
typedef struct
{
    /* basic information */
    uint8_t             bSpeed;                 /*!< 0, fs, 1, ls, 2, hs */
    uint8_t             bConnectStatus;         /*!< 0, disconnected, 1, connected */
    uint8_t             bEnumerated;            /*!< 0, enumerate failed, 1, enumerat succeeded */
    uint8_t             bPortReset;             /*!< 0, reset over, 1, reseting */
    uint8_t             bSuspend;               /*!< 0, normal, 1, suspend */
    uint8_t             bRemoteWakeUpDetected;  /*!< 0, normal, 1, remote wake up singal detected */
    uint8_t             bSendStatusError;       /*!< 0, normal, 1, error */
    uint8_t             bFrameList0Int;
    uint8_t             bAddress;               /*!< device address */
    uint8_t             bClassInfo;             /*!< BIT0, audio class, BIT1, hid class, BIT2, MSD class */
    uint8_t*            pDataBuffer;
    otg_host_qTD_t*     pQTDSendLast;

    /* descriptor information */
    otg_host_dsc_dev_t  deviceDescriptor;
    void*               pDscCfg;
    uint8_t*            pStrLanguage;
    uint8_t*            pStrManufacture;
    uint8_t*            pStrProduct;
    uint8_t*            pStrSerialNum;
    /* iso information */
    uint8_t             bISOTransferEnable;
    uint32_t            wISOiTDAddress[2][16];          //0 for out, 1 for in, 后面确定iTD数量之后，再减小
}otg_host_attach_device_t;

/**
 * @brief  Host parameter structure definition
 */
typedef struct {
    uint8_t                             numQHD;
    uint8_t                             numQTD;
    uint16_t                            numITD;
    uint8_t                             numDataPage;
    uint32_t                            addrDataPage;
    otg_host_qHD_t*                     pQHD;
    otg_host_qTD_t*                     pQTD;
    otg_host_iTD_t*                     pITD;
    otg_host_periodic_frame_list_t*     pPeriFrameList;
    uint8_t*                            pQTDManage;
    uint8_t*                            pITDManage;
    uint8_t*                            pDataPageManage;
    volatile otg_host_attach_device_t*  pAttachDevice;

    otg_host_iso_para_t*                pHostIsoPara;
    otg_host_qHD_t*                     pQHDListControl[3]; //这里为啥是3？
    otg_host_qHD_t*                     pQHDListBulk[3];
    otg_host_qHD_t*                     pQHDListInt[3];

}otg_host_para_t;
/**
 * @}
 */

/* Exported constants -------------------------------------------------------*/
/**
 * @defgroup OTG_HOST_Exported_Constants OTG Host Exported Constants
 * @{
 */
/**
 * @defgroup OTG_HOST_MEM_STATE Otg Host Memory State
 * @brief Memery state in memery manage of qTD, iTD and dataPage
 * @{
 */
#define OTG_HOST_MEM_FREE                   (0x01)
#define OTG_HOST_MEM_USED                   (0x02)
/**
 * @}
 */
/**
 * @defgroup OTG_HOST_MEM_TYPE Otg Host Memory Type
 * @brief Otg host memory type definition
 * @{
 */
#define OTG_HOST_MEM_TYPE_QTD               (0x00)
#define OTG_HOST_MEM_TYPE_ITD               (0x01)
#define OTG_HOST_MEM_TYPE_PAGE4K            (0x02)
#define OTG_HOST_MEM_TYPE_SITD              (0x03)
/**
 * @}
 */
/**
 * @defgroup OTG_HOST_HD_TYPE Otg Host HD Type
 * @brief Otg host HD type definition
 * @{
 */
#define OTG_HOST_HD_Type_ITD                (0x00)
#define OTG_HOST_HD_Type_QH                 (0x01)
#define OTG_HOST_HD_Type_SITD               (0x02)
#define OTG_HOST_HD_Type_FSTN               (0x03)
/**
 * @}
 */
/**
 * @defgroup OTG_HOST_QTD_PID_TYPE Otg Host QTD PID Type
 * @brief Otg host QTD PID type
 * @{
 */
#define OTG_HOST_QTD_PID_OUT                (0x00)
#define OTG_HOST_QTD_PID_IN                 (0x01)
#define OTG_HOST_QTD_PID_SETUP              (0x02)
/**
 * @}
 */
/**
 * @defgroup OTG_HOST_ITD_STATUS Otg Host iTD Status
 * @brief Otg host iTD status
 * @{
 */
#define OTG_HOST_ITD_STATUS_ACTIVE          (0x08)
#define OTG_HOST_ITD_STATUS_DATABUFF_ERR    (0x04)
#define OTG_HOST_ITD_STATUS_BABBLE_DETECT   (0x02)
#define OTG_HOST_ITD_STATUS_TRANSCTION_ERR  (0x01)
/**
 * @}
 */
/**
 * @defgroup OTG_HOST_SITD_POS Otg Host siTD Position
 * @brief Otg host siTD position
 * @{
 */
#define OTG_HOST_SITD_TP_ALL                (0x00)
#define OTG_HOST_SITD_TP_BEGIN              (0x01)
#define OTG_HOST_SITD_TP_MID                (0x02)
#define OTG_HOST_SITD_TP_END                (0x03)
/**
 * @}
 */

/**
 * @}
 */

/* Exported macros ----------------------------------------------------------*/
/* Private macros -----------------------------------------------------------*/
/**
 * @defgroup OTG_HOST_Private_Macros OTG Host Private Macros
 * @{
 */
//return 32bit address from link
#define OTG_S_ADDR_RETURN(addr)                                         \
(((((addr) << 5) & 0x20000000) == 0) ? (((addr) << 5) | 0x10000000) : ((addr) << 5))
//((addr) << 5)

//return HighAddress without address[4:0]
#define OTG_S_ADDR_ADJ(addr)                                            (OTG_MEM_ADDR_ADJ(addr) >> 5)
//return HighAddress without address[11:0]
#define OTG_S_ADDR_ADJ_4K(addr)                                         (OTG_MEM_ADDR_ADJ(addr) >> 12)
/**
 * @}
 */

/* Exported functions -------------------------------------------------------*/
/**
 * @addtogroup OTG_HOST_Exported_Functions OTG Host Exported Functions
 * @{
 */
/**
 * @brief  host irq process, called in irq handler, the code of this function is assigned in sram to run
 * @param  pPara       pointer to structure @ref otg_host_para_t
 * @return NONE
 */
void HalOtgHostIrq(otg_host_para_t *pPara);
/**
 * @brief  host module initial
 * @return NONE
 */
void HalOtgHostInit(void);
/**
 * @brief  host issue control, standard cmd
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @retval == 0            OK
 * @retval == 1            time out error
 * @retval == 2            cmd is not supported
 */
int32_t HalOtgHostConnectedInit(otg_host_para_t *pPara);
/**
 * @brief  host enumerate process
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @param  bAddress        device address
 * @retval == 0            OK
 * @retval != 0            Error
 */
int32_t HalOtgHostEnumerate(otg_host_para_t *pPara, uint8_t bAddress);
/**
 * @brief  host periodic schedule enable or disable control
 * @param  option          'OPTION_DISABLE' for disable, 'OPTION_ENABLE' for enable
 * @return NONE
 */
void HalOtgHostPeriodicScheduleSetting(uint8_t option);
/**
 * @brief  host get structure of qTD, iTD, siTD, or page 4K memory, if error will halt
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @param  type            memory type, @ref OTG_HOST_MEM_TYPE
 * @retval == 0            error with not found
 * @retval != 0            address
 */
uint32_t HalOtgHostStructureGet(otg_host_para_t *pPara, uint8_t type);
/**
 * @brief  host release structure of qTD, iTD, siTD, or page 4K memory
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @param  type            memory type, @ref OTG_HOST_MEM_TYPE
 * @param  address         start address of structure to release
 * @retval == 0            OK
 * @retval == 1            type error
 * @retval == 2            address error
 */
int32_t HalOtgHostStructureRelease(otg_host_para_t *pPara, uint8_t type, uint32_t address);
/**
 * @brief  host allocate new qTD for qHD
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @param  pQHD            pointer to structure, @ref otg_host_qHD_t
 * @param  pQHDPara        pointer to structure, @ref otg_host_qhd_para_t
 * @return None
 */
void HalOtgHostAllocateQHD(otg_host_para_t *pPara, otg_host_qHD_t *pQHD, otg_host_qhd_para_t *pQHDPara);
/**
 * @brief  host issue control base function
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @param  pCtrlPara       pointer to structure, @ref otg_host_ctrl_para_t
 * @param  mode            0, setup/out/in, 1, setup/in/out, 2, setup/in
 * @retval == 0            OK
 * @retval == 1            time out error
 */
int32_t HalOtgHostIssueControlBase(otg_host_para_t *pPara, otg_host_ctrl_para_t *pCtrlPara, uint8_t mode);
/**
 * @brief  host issue control base function
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @param  dir             1, in(OTG_HOST_QTD_PID_IN), 0, out(OTG_HOST_QTD_PID_OUT)
 * @param  pBufferArray    pointer to the buffer array
 * @param  size            byte number
 * @retval == 0            OK
 * @retval == 1            time out error
 */
int32_t HalOtgHostIssueInterrupt(otg_host_para_t *pPara, uint8_t dir, uint32_t *pBufferArray, size_t size);
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
 * @addtogroup HAL_WEAK_Functions_OTG_HOST Weak Functions of OTG Host
 * @brief Weak functions in a0_hal_otg_host.c
 * @{
 */
/**
 * @brief  host structure initial
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @retval == 0            OK
 * @retval != 0            Error
 */
int32_t HalOtgHostStructureInit(otg_host_para_t *pPara);

/**
 * @brief  host configuration descriptor analysis
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @param  pbData
 * @retval == 0            OK
 * @retval != 0            Error
 */
int32_t HalOtgHostAnalysisConfiguration(otg_host_para_t *pPara, uint8_t *pbData);
/**
 * @brief  host enumerate information interface of class respond
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @retval == 0            OK
 * @retval != 0            Error
 */
int32_t HalOtgHostEnumerateInterfaceClass(otg_host_para_t *pPara);
/**
 * @brief  host interrupt isr process, update periodic schedule
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @retval == 0            OK
 * @retval != 0            Error
 */
int32_t HalOtgHostInterruptIsr(otg_host_para_t *pPara);
/**
 * @brief  host iso isr process, update periodic schedule
 * @param  pPara           pointer to structure @ref otg_host_para_t
 * @return NONE
 */
void HalOtgHostIsoIsr(otg_host_para_t *pPara);
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
