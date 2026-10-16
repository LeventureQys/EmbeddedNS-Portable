#include "USB\Device\hid\drv_usb_device_hid.h"

#ifdef DRV_OTG_DEVICE_ENABLE_HID
#include "a0_hal_dsp.h"
#include "simulation.h"
#include "IIS\SynthSchedule\SynthSchedule.h"

/* Private define ------------------------------------------------------------*/

#define USB_HID_MAX_PACK_SIZE_FULLSPEED				(64)
#define USB_HID_MAX_PACK_SIZE_HIGHSPEED				(1024)

#define USB_HID_FIFO_SIZE_FULLSPEED					(512)
#define USB_HID_FIFO_SIZE_HIGHSPEED					(1024)

#define USB_HID_POOL_INTERVAL_FULLSPEED				(0x05)
#define USB_HID_POOL_INTERVAL_HIGHSPEED				(0x02)

#define USB_HID_MAX_BUFF_SIZE_FULLSPEED				USB_HID_MAX_PACK_SIZE_FULLSPEED
#define USB_HID_MAX_BUFF_SIZE_HIGHSPEED				USB_HID_MAX_PACK_SIZE_HIGHSPEED

#ifdef 	DRV_OTG_HID_USE_HIGHSPEED
	#define USB_HID_MAX_PACK_SIZE				USB_HID_MAX_PACK_SIZE_HIGHSPEED
	#define USB_HID_POLL_INTERVAL				USB_HID_POOL_INTERVAL_HIGHSPEED
	#define USB_SPEC_VERSION					USB_SPEC_VERSION2_0
	#define USB_HID_MAX_BUFF_SIZE				USB_HID_MAX_PACK_SIZE_HIGHSPEED
	#define USB_HID_FIFO_SIZE					USB_HID_FIFO_SIZE_HIGHSPEED
#else
	#define USB_HID_MAX_PACK_SIZE				USB_HID_MAX_PACK_SIZE_FULLSPEED
	#define USB_HID_POLL_INTERVAL				USB_HID_POOL_INTERVAL_FULLSPEED
	#define USB_SPEC_VERSION					USB_SPEC_VERSION1_1
	#define USB_HID_MAX_BUFF_SIZE				USB_HID_MAX_PACK_SIZE_FULLSPEED
	#define USB_HID_FIFO_SIZE					USB_HID_FIFO_SIZE_FULLSPEED
#endif

#define USB_DEVICE_FUNC_HID_PID				(0x30A3)

#define FIFO_IDX_HID_IN                     (0) //fifo4 for hid in
#define FIFO_IDX_HID_OUT                    (2) //fifo5 for hid out

#define EP_IDX_HID                          (USB_HID_EP_NUM)    //for hid in/out

#define TRANS_TYPE_HID                      (OTG_FIFO_TYPE_INT)

#define EP_FIFO_CFG_IDX_HID                 (0)
#define EP_FIFO_CFG_IDX_MAX                 (1)


/* Private macro -------------------------------------------------------------*/
#define USB_HID_IN_FIFO_INT_EN()            (OTG_DEV_INT_FIFO_IN_EN(FIFO_IDX_HID_IN))
#define USB_HID_IN_FIFO_INT_DIS()           (OTG_DEV_INT_FIFO_IN_DIS(FIFO_IDX_HID_IN))

/* Descriptors -------------------------------------------------------------*/

#pragma diag_suppress=191           //disable warning #191-D
#pragma diag_suppress=1296          //disable warning #1296-D


#define REPORTID_RECIEVE					(0x51)
#define REPORTID_SEND						(0x11)

#ifdef DRV_OTG_HID_USE_HIGHSPEED
/* usb hid report descriptor const table */
const static uint8_t hidReportDescriptor[] = 
{
    0x06,0x01,0xFF,     //Usage Page
    0x09,0x04,          //Usage
    0xA1,0x01,          //Collection(Application)
        /* Collection */
        0xA1,0x02,          //Collection(Logical)
        0x15,0x80,          //Logical Minimum (-128)
        0x25,0x7F,          //Logical Maximum (127)
        0x75,0x08,          //Report Size (8)
	
        0x85,0x11,          //Report ID(17)
        0x96,0xFF,03,		//Report Count (1023)
        0x09,0x14,          //Usage
        0x81,0x22,          //INPUT (Data,Var,Abs,NWrp,Lin,NPrf,NNul,Bit)

        0x85,0x51,          //Report ID(81)
        0x96,0XFF,03,       //Report Count (1023)
        0x09,0x14,          //Usage
        0x91,0x22,          //OUTPUT (Data,Var,Abs,NWrp,Lin,NPrf,NNul,NVol,Bit)
        0xC0,               //End Collection
    0xC0,               //End Collection
};
#else
/* usb hid report descriptor const table */
const static uint8_t hidReportDescriptor[] = 
{
    0x06,0x01,0xFF,     //Usage Page
    0x09,0x04,          //Usage
    0xA1,0x01,          //Collection(Application)
        /* Collection */
        0xA1,0x02,          //Collection(Logical)
        0x15,0x80,          //Logical Minimum (-128)
        0x25,0x7F,          //Logical Maximum (127)
        0x75,0x08,          //Report Size (8)
        0x85,0x11,          //Report ID(17)
        0x95,0x3F,		 	//Report Count (63)
        0x09,0x14,          //Usage
        0x81,0x22,          //INPUT (Data,Var,Abs,NWrp,Lin,NPrf,NNul,Bit)
        0x85,0x51,          //Report ID(81)
        0x95,0x3F,      	//Report Count (63)
        0x09,0x14,          //Usage
        0x91,0x22,          //OUTPUT (Data,Var,Abs,NWrp,Lin,NPrf,NNul,NVol,Bit)
        0xC0,               //End Collection
    0xC0,               //End Collection
};
#endif

const static usb_dsc_t appHidDeviceDescriptor = 
{
    .bLength 					= sizeof(usb_dsc_t),                  // bLength
    .bDescriptorType			= USB_DSC_DEVICE,                     // bDescriptorType
    .bcdUSB 					= USB_SPEC_VERSION,                   // Specification Version(bcd)
    .bDeviceClass 				= 0x00,                               // bDeviceClass
    .bDeviceSubClass 			= 0x00,                               // bDeviceSubClass
    .bDeviceProtocol 			= 0x00,                               // bDeviceProtocol
    .bMaxPacketSize0 			= OTG_MAX_EP0_PACK_SIZE,              // bMaxPacketSize0
    .idVendor 					= USB_DEFAULT_VID,                    // Vendor ID
    .idProduct 					= USB_DEVICE_FUNC_HID_PID,            // Product ID
    .bcdDevice 					= USB_DEVICE_VERSION,                 // bcdDevice release,Version
    .iManufacturer 				= USB_STR_INDEX_MANUFACTURER,         // iManufacturer
    .iProduct 					= USB_STR_INDEX_PRODUCT,              // iProduct
    .iSerialNumber 				= USB_STR_INDEX_SERIALNUMBER,         // iSerialNumber
    .bNumConfigurations 		= USB_CFG_DESCRIPTORS,            // bNumConfigurations
};

const static usb_dsc_cfg_hid_t appUsbHidConfigDescriptor = 
{
    .dscCfg = 
    {
        .bLength 				= sizeof(usb_dsc_cfg_t),
        .bDescriptorType	 	= USB_DSC_CONFIG,
        .wTotalLength 			= sizeof(usb_dsc_cfg_hid_t),
        .bNumInterfaces 		= HID_IF_NUM,
        .bConfigurationValue 	= 0x01,
        .iConfiguration 		= 0x00,
        .bmAttributes 			= USB_CFG_DEFAULT,//USB_CFG_ATTRIBUTES,
        .bMaxPower 				= (USB_CFG_MAXPOWER/2),
    },
    .dscCsIfHid = 
    {
        .dscIf = 
        {
            .bLength 			= sizeof(usb_dsc_if_t),
            .bDescriptorType 	= USB_DSC_INTERFACE,
            .bInterfaceNumber 	= HID_IF_ID,
            .bAlternateSetting 	= 0x00,
            .bNumEndpoints 		= 0x02,
            .bInterfaceClass 	= HID_CLASS,
            .bInterfaceSubClass = HID_SUBCLASS_NONE,
            .bInterfaceProtocol = HID_PROTOCOL_NONE,
            .iInterface 		= 0x00,
        },
        .dscHid = 
        {
            .bLength 			= sizeof(usb_dsc_class_hid_t),
            .bDescriptorType 	= HID_DSC_HID,
            .bcdHID 			= HID_SPEC_VERSION,
            .bCountryCode 		= 0x00,
            .bNumDescriptors 	= HID_CLASS_DESCRIPTORS_NUM,
            .descriptor = {
                {
                    .bDescriptorType 	= HID_DSC_REPORT,
                    .wDescriptorLength 	= sizeof(hidReportDescriptor),
                },
            },
        },
        .dscEpIn = 
        {
            .bLength 			= sizeof(usb_dsc_ep_t),
            .bDescriptorType 	= USB_DSC_ENDPOINT,
            .bEndpointAddress 	= (USB_HID_EP_NUM|USB_EP_DEV2HOST),
            .bmAttributes 		= USB_EP_TRANSFER_TYPE_INT,
            .wMaxPacketSize 	= USB_HID_MAX_PACK_SIZE,
            .bInterval 			= USB_HID_POLL_INTERVAL,
        },
        .dscEpOut = 
        {
            .bLength 			= sizeof(usb_dsc_ep_t),
            .bDescriptorType 	= USB_DSC_ENDPOINT,
            .bEndpointAddress 	= (USB_HID_EP_NUM|USB_EP_HOST2DEV),
            .bmAttributes 		= USB_EP_TRANSFER_TYPE_INT,
            .wMaxPacketSize 	= USB_HID_MAX_PACK_SIZE,
            .bInterval 			= USB_HID_POLL_INTERVAL,
        },
    },
};
#ifdef DRV_OTG_HID_USE_HIGHSPEED
const static USBDeviceQualifierDescriptor appHidDeviceQualifierDescriptor = {
	.bLength 					= sizeof(USBDeviceQualifierDescriptor),
	.bDescriptorType 			= USB_DSC_DEVICE_QUALIFIER,
	.bcdUSB 					= USB_SPEC_VERSION,
	.bDeviceClass 				= 0,
	.bDeviceSubClass 			= 0,
	.bDeviceProtocol 			= 0,
	.bMaxPacketSize0 			= OTG_MAX_EP0_PACK_SIZE,
	.bNumConfigurations 		= 1,
	.bReserved 					= 0,
};

const static usb_dsc_cfg_hid_t appUsbHidOtherSpeedConfigDescriptor = 
{
    .dscCfg = 
    {
        .bLength 				= sizeof(usb_dsc_cfg_t),
        .bDescriptorType 		= USB_DSC_OTHER_SPEED_CONFIG,
        .wTotalLength 			= sizeof(usb_dsc_cfg_hid_t),
        .bNumInterfaces 		= HID_IF_NUM,
        .bConfigurationValue 	= 0x01,
        .iConfiguration 		= 0x00,
        .bmAttributes 			= USB_CFG_ATTRIBUTES,
        .bMaxPower 				= (USB_CFG_MAXPOWER/2),
    },
    .dscCsIfHid = 
    {
        .dscIf = 
        {
            .bLength 			= sizeof(usb_dsc_if_t),
            .bDescriptorType 	= USB_DSC_INTERFACE,
            .bInterfaceNumber 	= HID_IF_ID,
            .bAlternateSetting 	= 0x00,
            .bNumEndpoints 		= 0x02,
            .bInterfaceClass 	= HID_CLASS,
            .bInterfaceSubClass = HID_SUBCLASS_NONE,
            .bInterfaceProtocol = HID_PROTOCOL_NONE,
            .iInterface 		= 0x00,
        },
        .dscHid = 
        {
            .bLength 			= sizeof(usb_dsc_class_hid_t),
            .bDescriptorType 	= HID_DSC_HID,
            .bcdHID 			= HID_SPEC_VERSION,
            .bCountryCode 		= 0x00,
            .bNumDescriptors 	= HID_CLASS_DESCRIPTORS_NUM,
            .descriptor = {
                {
                    .bDescriptorType 	= HID_DSC_REPORT,
                    .wDescriptorLength 	= sizeof(hidReportDescriptor),
                },
            },
        },
        .dscEpIn = 
        {
            .bLength 			= sizeof(usb_dsc_ep_t),
            .bDescriptorType 	= USB_DSC_ENDPOINT,
            .bEndpointAddress 	= (USB_HID_EP_NUM|USB_EP_DEV2HOST),
            .bmAttributes 		= USB_EP_TRANSFER_TYPE_INT,
            .wMaxPacketSize 	= USB_HID_MAX_PACK_SIZE,
            .bInterval 			= USB_HID_POLL_INTERVAL,
        },
        .dscEpOut = 
        {
            .bLength 			= sizeof(usb_dsc_ep_t),
            .bDescriptorType 	= USB_DSC_ENDPOINT,
            .bEndpointAddress 	= (USB_HID_EP_NUM|USB_EP_HOST2DEV),
            .bmAttributes 		= USB_EP_TRANSFER_TYPE_INT,
            .wMaxPacketSize 	= USB_HID_MAX_PACK_SIZE,
            .bInterval 			= USB_HID_POLL_INTERVAL,
        },
    },
};
const static uint16_t hidOtherSpeedConfigDescriptorSizeTable[USB_CFG_DESCRIPTORS] = {
	sizeof(appUsbHidOtherSpeedConfigDescriptor),
};

const static uint32_t hidOtherSpeedConfigDescriptorAddressTable[USB_CFG_DESCRIPTORS] = {
	(const uint32_t)&appUsbHidOtherSpeedConfigDescriptor,
};
#endif

const static uint16_t hidConfigDescriptorSizeTable[USB_CFG_DESCRIPTORS] = 
{
    sizeof(appUsbHidConfigDescriptor),
};

const static uint16_t hidStringDescriptorSizeTable[USB_STR_DESCRIPTORS] = 
{
    sizeof(appUsbStringDescriptorOfLanguage),
    sizeof(appUsbStringDescriptorOfManufacturer),
    sizeof(appUsbStringDescriptorOfProduct),
    sizeof(appUsbStringDescriptorOfSerialNumber),
};

const static uint32_t hidConfigDescriptorAddressTable[USB_CFG_DESCRIPTORS] = 
{
    (const uint32_t)&appUsbHidConfigDescriptor,
};


const static uint32_t hidStringDescriptorAddressTable[USB_STR_DESCRIPTORS] = 
{
    (const uint32_t)&appUsbStringDescriptorOfLanguage[0],
    (const uint32_t)&appUsbStringDescriptorOfManufacturer[0],
    (const uint32_t)&appUsbStringDescriptorOfProduct[0],
    (const uint32_t)&appUsbStringDescriptorOfSerialNumber[0],
};

/* Private function declarations ---------------------------------------------*/
static void usb_device_hid_in_cb(void);
static void usb_device_hid_out_cb(void);
static void app_fun_null(void);
static int32_t usb_hid_get_report(void);

/* Private variables ---------------------------------------------------------*/
static uint8_t 	usbIfAltSetting[HID_IF_NUM];

static uint8_t 	usbHidInBuffer[USB_HID_MAX_PACK_SIZE];                  //hid发送缓存
static uint8_t 	usbHidInState;                       					//0, idle, ready to send, 1, busy, can't send
static uint32_t usbHidInSize;                        					//byte number to send, max 64
static uint8_t 	usbHidOutBuffer[USB_HID_MAX_PACK_SIZE];                 //hid接收缓冲
static uint8_t 	usbHidOutState;                      					//0, no hid out data, 1, got data, need to decode

static otg_ep_fifo_cfg_t hidEndpointFifoConfig[EP_FIFO_CFG_IDX_MAX] = {
    {
        .epIdx = EP_IDX_HID,
        .fifoInIdx = FIFO_IDX_HID_IN,         
        .fifoOutIdx = FIFO_IDX_HID_OUT,        
        .fifo1024 = (USB_HID_FIFO_SIZE>>10),
        .transType = TRANS_TYPE_HID,
    }
};

static usb_dev_info_t hidDeviceInformation = 
{
    .numIf 				= HID_IF_NUM,
    .numDscCfg 			= USB_CFG_DESCRIPTORS,
    .numDscStr 			= USB_STR_DESCRIPTORS,
    .pIfAltSetting 		= usbIfAltSetting,
    .sizeDscDev 		= sizeof(appHidDeviceDescriptor),
    .pDscDev 			= (uint8_t *)&appHidDeviceDescriptor,
	
#ifdef DRV_OTG_HID_USE_HIGHSPEED
	.pDscDevQualifier 		= (uint8_t*)&appHidDeviceQualifierDescriptor,
	.sizeDscDevQualifier	= sizeof(USBDeviceQualifierDescriptor),	
	.pSizeDscOtherSpeedCfg 	= NULL,
	.pAddrDscOtherSpeedCfg 	= NULL,
#else
	.pDscDevQualifier 		= NULL,
	.sizeDscDevQualifier	= 0,
	.pSizeDscOtherSpeedCfg 	= NULL,
	.pAddrDscOtherSpeedCfg 	= NULL,
#endif
	.sizeDscHidReport 		= sizeof(hidReportDescriptor),
    .pDscHidReport 			= (uint8_t *)&hidReportDescriptor,
    .pSizeDscCfg 			= (uint16_t *)&hidConfigDescriptorSizeTable[0],
    .pSizeDscStr 			= (uint16_t *)&hidStringDescriptorSizeTable[0],
    .pAddrDscCfg 			= (uint32_t *)&hidConfigDescriptorAddressTable[0],
    .pAddrDscStr 			= (uint32_t *)&hidStringDescriptorAddressTable[0],
};

static otg_dev_para_t otgHidDeviceParameter = {
#ifdef DRV_OTG_HID_USE_HIGHSPEED
    .isFullSpeed = 0,
#else
	.isFullSpeed = 1,
#endif
    .epNumInUse = 1,
    .pEpFifoCfg = hidEndpointFifoConfig,
    .pDevInfo = (void *)&hidDeviceInformation,
};

/* Private constants ---------------------------------------------------------*/
/* fifo??μ÷oˉêy±í?? */
static const UsbFifoIrqHandler usbHidFifoIrqHandler[OTG_MAX_FIFO_NUM][2] = 
{
    /* IN, OUT */
    {usb_device_hid_in_cb,  app_fun_null                	},  /* FIFO0 */
    {app_fun_null,              app_fun_null   				},  /* FIFO1 */
    {app_fun_null,              usb_device_hid_out_cb   	},  /* FIFO2 */
    {app_fun_null,              app_fun_null                },  /* FIFO3 */
    {app_fun_null,  			app_fun_null                },  /* FIFO4 */
    {app_fun_null,              app_fun_null   				},  /* FIFO5 */
    {app_fun_null,              app_fun_null                },  /* FIFO6 */
    {app_fun_null,              app_fun_null                },  /* FIFO7 */
};

/* Private function prototypes -----------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////
//  空fifo中断处理函数
////////////////////////////////////////////////////////////////////////////////
static void app_fun_null(void)
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
//  HID IN/OUT端点对应fifo中断的数据处理回调
////////////////////////////////////////////////////////////////////////////////
static void usb_device_hid_in_cb(void)
{
    hal_otg_device_dx_fifo_write(usbHidInBuffer, usbHidInSize, FIFO_IDX_HID_IN);
	USB_HID_IN_FIFO_INT_DIS();
    usbHidInState = 0;
    return;
}
static void usb_device_hid_out_cb(void)
{
    uint8_t fifoIdx = hidEndpointFifoConfig[EP_FIFO_CFG_IDX_HID].fifoOutIdx;
    uint32_t rxByteN = OTG_DEV_FIFO_OUT_BYTE_CNT(fifoIdx);
    hal_otg_device_dx_fifo_read(usbHidOutBuffer, rxByteN, FIFO_IDX_HID_OUT);
	usbHidOutState = 1;
    return;
}

static int32_t usb_hid_get_report(void)
{
    int32_t result;
    otg_dev_para_t *pPara = &otgHidDeviceParameter;
    if ((pPara->devSetupPacket.wValue >> 8) == HID_REPORT_IN)
    {
        usbHidInBuffer[0] = (uint8_t)pPara->devSetupPacket.wValue;   //report ID
        pPara->pData = usbHidInBuffer;
        pPara->sizeData = pPara->devSetupPacket.wLength;
        hal_otg_device_ep0_tx_data(pPara);
        result = -1;
    }
    else
    {
        result = 1;
    }
    return result;
}


/* WEAK function rewrite -----------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////
//  usb device fifo的中断回调函数。
////////////////////////////////////////////////////////////////////////////////
__RAM_CODE void hal_otg_device_fifo_cb(uint8_t fifoIdx, uint32_t in, uint32_t out)
{
    /* fifo?D???ìó| */
    if (in != 0)
    {
        usbHidFifoIrqHandler[fifoIdx][0]();
    }
    if (out != 0)
    {
        usbHidFifoIrqHandler[fifoIdx][1]();
    }
    return;
}

////////////////////////////////////////////////////////////////////////////////
//  替换drv_usb_device_hid.c中的weak函数，用于处理hid class特定请求。
////////////////////////////////////////////////////////////////////////////////

static uint8_t reportRate = 0;
//return 0, set ep0 done
//return 1, set ep0 stall
//return -1, do nothing, prepare to receive
int32_t drv_usb_class_cmd(setup_packet_t *pDevSetupPacket)
{
    int32_t result;
    if (pDevSetupPacket->bmRequestType.b.recipient == USB_SETUP_REQ_RECIPIENT_INTERFACE){
        if (pDevSetupPacket->bRequest == HID_GET_REPORT){
            result = usb_hid_get_report();
        }else if(pDevSetupPacket->bRequest == HID_SET_IDLE){
			reportRate = pDevSetupPacket->wValue >> 8;
			OTG_DEV_EP_IN_STALL_CLR(EP_IDX_HID);
			result = 0;
		}else if(pDevSetupPacket->bRequest == HID_GET_IDLE){
			otgHidDeviceParameter.pData = &reportRate;
			otgHidDeviceParameter.sizeData = sizeof(reportRate);
			hal_otg_device_ep0_tx_data(&otgHidDeviceParameter);
			return 0;
		}else{
            result = 1;
        }
    }else{
        result = 1;
    }
    return result;    
}

static int32_t drv_usb_device_hid_in_real_send(uint8_t *pSendData, size_t byteSize){
	if (usbHidInState == 0){
        usbHidInState = 1;
        usbHidInSize = byteSize;
        memcpy((void *)usbHidInBuffer, pSendData, byteSize);
        USB_HID_IN_FIFO_INT_EN();
        return 0;
    }
    
	return 1;
}

#if defined(DRV_I2S_ENABLE_SYNTH_SCHEDULE) && (DRV_OTG_DEVICE_HID_FUNCTION_SELECT == DRV_OTG_DEVICE_HID_FUNCTION_FOR_SIMULATION)

static void usb_device_hid_out_handler(uint8_t *hidOutBuffer){
	SimulationReport *report = (SimulationReport*)hidOutBuffer;
	
	if(doProcessSimulationReport(report) != 0){
		drv_usb_device_hid_in_real_send((uint8_t*)report,sizeof(*report));
	}
}

static SimulationReport localReport;
static uint8_t dspBreakAckPending = 0;
// dspBreakPointAck只会在DSP_END_IRQ中断调用和hid处理主循环中调用，因此此处不需要再对dspBreakAckPending
// 进行临界区保护。
void dspBreakPointAck(void) {
	dspBreakAckPending = 1;
}
static void dspEndIrqDisable(void){
	NVIC_DisableIRQ(DSP_END_IRQn);
}
static void dspEndIrqEnable(void){
	NVIC_EnableIRQ(DSP_END_IRQn);
}
static void i2sIrqDisable(void){
	NVIC_DisableIRQ(I2S_IRQn);
}
static void i2sIrqEnable(void){
	NVIC_EnableIRQ(I2S_IRQn);
}
static void usb_device_hid_in_handler(void){
	uint8_t pending;
	
	/* 	
		为防止与DSP_END_IRQ中断对dspBreakAckPending的访问产生竞争，
		短暂的禁止该中断，获取并更新dspBreakAckPending的状态后再恢复。
	*/
	dspEndIrqDisable();
	pending = dspBreakAckPending;
	dspBreakAckPending = 0;
	dspEndIrqEnable();
	
	// 当确定DSP已暂停运行(因DSP HOLD指令或被打了断点)，则发送应答。
	if(pending){
		breakAckReport(&localReport);
		drv_usb_device_hid_in_real_send((uint8_t*)&localReport,sizeof(localReport));
	}
}
#else
__WEAK void usb_device_hid_out_handler(uint8_t *hidOutBuffer){
	return;
}
#endif
/* 在主循环中处理接收的数据 */
void drv_usb_device_hid_process(void)
{
    /* 当有数据在缓存中时调用数据处理函数处理 */
    if (usbHidOutState != 0){
        usb_device_hid_out_handler(usbHidOutBuffer);
        usbHidOutState = 0;
    }
#if defined(DRV_I2S_ENABLE_SYNTH_SCHEDULE) &&  (DRV_OTG_DEVICE_HID_FUNCTION_SELECT == DRV_OTG_DEVICE_HID_FUNCTION_FOR_SIMULATION)
	usb_device_hid_in_handler();
#endif
    return;
}

void drv_usb_device_hid_init(void)
{
#if defined(DRV_I2S_ENABLE_SYNTH_SCHEDULE) && (DRV_OTG_DEVICE_HID_FUNCTION_SELECT == DRV_OTG_DEVICE_HID_FUNCTION_FOR_SIMULATION)
	// 初始化DSP系统仿真report id
	simulation_init(REPORTID_NONE,REPORTID_NONE);
	setSynthInitialIdleState(SYNTH_SCHED_STATE_IDLE0);
#endif

    /* vbus config to 1 */
    HAL_SYS_USB_ID_SET();
    HAL_SYS_USB_VBUS_EN();
    
    hal_otg_device_init(&otgHidDeviceParameter);
    
    usb_dev_info_t *pDevInfo = (usb_dev_info_t *)otgHidDeviceParameter.pDevInfo;
    pDevInfo->dscCfg_bmAttributes = appUsbHidConfigDescriptor.dscCfg.bmAttributes;
	
    return;
}

__RAM_CODE void drv_usb_device_irq_handler(void)
{
    hal_otg_device_irq(&otgHidDeviceParameter);
    return;
}



/** @brief  Send hid in data
  * @param  pSendData: Pointer to the data need to be sent
  * @param  byteSize: byte number to be sent
  * @retval == 0, ready to send
  * @retval == 1, busy, need to call this function later
  */
int32_t drv_usb_device_hid_in_send(uint8_t *pSendData, size_t byteSize)
{
	int32_t ret = 0;
	
#if !defined(DRV_I2S_ENABLE_SYNTH_SCHEDULE) || (DRV_OTG_DEVICE_HID_FUNCTION_SELECT != DRV_OTG_DEVICE_HID_FUNCTION_FOR_SIMULATION)
	ret =  drv_usb_device_hid_in_real_send(pSendData,byteSize);
#else
	//如果HID用于DSP 仿真DEBUG那么，hid report被占用，该接口不允许被外部使用。
	__die("hid is reserved for simulation!");
#endif
    
	return ret;
}

int drv_usb_device_hid_register_listener(USBDeviceListener *listener){
	__die("not support!");
	return -1;
}

USB_DEV_STATE drv_usb_device_get_devstate_func_hid(void)
{
    return otgHidDeviceParameter.devState;
}


#endif
