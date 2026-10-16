
#include "drv_uac10_user_config.h"

#if (UAC_IF_ID_HID >= 0)
#include "lib_drv_uac10.h"

#if USB_HID_EXT_FLASH_UPGRADE_EN
#include "a0_hal_spi_flash.h"
#endif

/* Private define ------------------------------------------------------------*/
// 定义本文件中需要使用到的Tick计时函数，最小单位ms
#define UsbHidTickGet                   HAL_TickGet
#define UsbHidTickDelay                 HAL_TickDelay
// 定义HID-IN发送超时判断时间
#define USB_HID_IN_TIME_OUT             (4)                 // 4ms
// 定义HID-OUT数据包在未处理之前允许最大接收的包数
#define USB_HID_OUT_PACKET_MAX          (4)                 // 最大支持4包
// 定义测试HID给PC发送数据测测试总数据
#define USB_HID_SEND_CTRL_NUM           (20)

/* Private types -------------------------------------------------------------*/
typedef struct {
    uint8_t     hidPacketBuff[USB_HID_OUT_PACKET_MAX][64];  // 从PC接收的数据包缓存
    uint8_t     hidPacketBuffSize[USB_HID_OUT_PACKET_MAX];  // 从PC接收的数据包大小
    uint8_t     hidPacketSend[64];                          // 往PC发送的数据包缓存
    uint32_t    hidParaBuff[17];                            // 预处理之后，用于后续解析数据的缓存
#if USB_HID_EXT_FLASH_UPGRADE_EN
    uint32_t    hidPacketIdx;
#endif
    uint8_t     hidPacketWrIdx;                             // HID-OUT数据包，中断接收的包数
    uint8_t     hidPacketRdIdx;                             // HID-OUT数据包，处理接收的包数
    uint8_t     hidLedState;                                // 记录LED状态值
    uint8_t     aspIdx;                                     // 1, 唤出Teams窗口
    FUNCPP      pHidUserParaHandlerCb;                      // HID解码回调函数，需要用户配置
    FUNCPP      pHidVenderHandlerCb;                        // HIDVenderID的解码回调函数，用于UC通信时的16字节解码处理
} drv_uac10_para_hid_t;


/* Private function declarations ---------------------------------------------*/
static void UsbHidSendExample(void);
static void UsbHidPreDecode(uint8_t *pHidOut, uint8_t *pHidIn, uint32_t *pParaBuff);

/* Public variables ----------------------------------------------------------*/
#if (USB_HID_SEND_TEST_EN != 0)
uint32_t g_testUsbHidSendCtrlFlag;                          //直接在watch窗口修改全局变量值观测发码响应情况，发码内容对应usbHidSendCtrlTb表格内数据
uint32_t g_testUsbHidSendKeyboard;
#endif

/* Private variables ---------------------------------------------------------*/
static drv_uac10_para_hid_t drvUac10ParaHid;

/* Private constants ---------------------------------------------------------*/
#if (USB_HID_SEND_TEST_EN)
static uint16_t usbHidSendCtrlTb[USB_HID_SEND_CTRL_NUM] =
{
    0x0000,
    0x0201, 0x0202, 0x0204, 0x0208, 0x0210, 0x0220, 0x0240, 0x0280,
    0x0301, 0x0302, 0x0304, 0x0308, 0x0310, 0x0320, 0x0340, 0x0380,
    0x0501, 0x9B01,
};
#endif

/* Weak functions ------------------------------------------------------------*/
// 开放给用户自行实现HID的周期发码处理，可参考UsbHidSendExample来实现
__WEAK void AppUsbHidSendHandler(void)
{
    return;
}

/* Weak functions rewrite ----------------------------------------------------*/
// LIB库内定义的弱函数，此处重写，用于中断直接接收HID-OUT数据（PC下发的）
// pDataMem:    接收的数据源指针
// size:        字节数量，如果size=0，表示空包，特殊应用时需要额外处理，通常情况下不用管
void UsbHidOutIrqHandler(uint8_t* pDataMem, size_t size)
{
    if (size != 0)
    {
        LibDrvMemcpy(drvUac10ParaHid.hidPacketBuff[drvUac10ParaHid.hidPacketWrIdx], pDataMem, size);
        drvUac10ParaHid.hidPacketBuffSize[drvUac10ParaHid.hidPacketWrIdx] = size;//Niko
        drvUac10ParaHid.hidPacketWrIdx++;
        if (drvUac10ParaHid.hidPacketWrIdx == USB_HID_OUT_PACKET_MAX)
        {
            drvUac10ParaHid.hidPacketWrIdx = 0;
        }
    }
    return;
}

/* Exported functions --------------------------------------------------------*/
// 提供给UC通信耳机功能获取LED状态的接口函数
uint32_t DrvUsbHidLedStateGet(void)
{
    uint32_t ledState = drvUac10ParaHid.hidLedState;
    return (ledState);
}

// 用于UC通信、播放器控制等进行HID-IN的控制码发送
// ctrlData: 待发送数据，如：0x0206，则表示发送 ID=0x02，数据=0x06
// return:   0, 发送成功
//           1, 发送失败
int32_t DrvUsbHidSend2Byte(uint32_t ctrlData)
{
    uint8_t hidSendData[2] = {
        (uint8_t)(ctrlData >> 8),
        (uint8_t)(ctrlData >> 0),
    };
    LibDrvUac10HidInSend(hidSendData, 2);
    uint32_t curTick = UsbHidTickGet();
    while ((LibDrvUac10StateGet() & BIT3) != 0)
    {
        if ((UsbHidTickGet() - curTick) > USB_HID_IN_TIME_OUT)
        {   // 等了USB_HID_IN_TIME_OUT 毫秒，仍然未发送成功，则直接复位并返回错误，让用于决定下次是否重发
            LibDrvUac10HidInSendReset();
            return 1;
        }
    }
    return 0;
}

int32_t DrvUsbHidSend3Byte(uint32_t ctrlData)
{
    uint8_t hidSendData[3] = {
        (uint8_t)(ctrlData >> 16),
        (uint8_t)(ctrlData >> 8),
        (uint8_t)(ctrlData >> 0),
    };
    LibDrvUac10HidInSend(hidSendData, 3);
    uint32_t curTick = UsbHidTickGet();
    while ((LibDrvUac10StateGet() & BIT3) != 0)
    {
        if ((UsbHidTickGet() - curTick) > USB_HID_IN_TIME_OUT)
        {   // 等了USB_HID_IN_TIME_OUT 毫秒，仍然未发送成功，则直接复位并返回错误，让用于决定下次是否重发
            LibDrvUac10HidInSendReset();
            return 1;
        }
    }
    return 0;
}

int32_t DrvUsbHidSend2ByteForHeadset(void *pCtrlData)
{
    return (DrvUsbHidSend2Byte(*((uint32_t *)pCtrlData)));
}

void DrvUsbHidUserParaHandlerCallbackInstall(FUNCPP cb)
{
    drvUac10ParaHid.pHidUserParaHandlerCb = cb;
    return;
}

void DrvUsbHidUserParaHandlerCallbackUnInstall(void)
{
    drvUac10ParaHid.pHidUserParaHandlerCb = NULL;
    return;
}

void DrvUsbHidVenderTelephonyCallbackInstall(FUNCPP cb)
{
    drvUac10ParaHid.pHidVenderHandlerCb = cb;
    return;
}

void DrvUsbHidVenderTelephonyCallbackUnInstall(void)
{
    drvUac10ParaHid.pHidVenderHandlerCb = NULL;
    return;
}

#include "Hid.h"

// HID解码处理
void DrvUac10HidHandler(void)
{
#if USB_HID_SEND_TEST_EN
    UsbHidSendExample();
#endif

    AppUsbHidSendHandler();

    while (drvUac10ParaHid.hidPacketWrIdx != drvUac10ParaHid.hidPacketRdIdx)
    {
        uint8_t *pHidOutData = &drvUac10ParaHid.hidPacketBuff[drvUac10ParaHid.hidPacketRdIdx][0];
        uint32_t *pHidPara = &drvUac10ParaHid.hidParaBuff[0];
        uint8_t hidID = pHidOutData[0];
#if (USB_HID_COL_USER_TRANS_EN != 0)
        if (hidID == USB_HID_OUT_ID1)
        {   // 自定义端口进行数据通信，用于PC调音软件等解析处理
            // by Niko 获取HID接收的数据
            // if(pHidOutData[1] == 0xC4)//Maono 帧头
            {
                uint16_t size = drvUac10ParaHid.hidPacketBuffSize[drvUac10ParaHid.hidPacketRdIdx];
                Hid.RxFiFoWrite(&pHidOutData[1], size-1);
            }

            // 解析预处理，并立刻给PC应答
            #if(EFFECT_DEBUG == 1)
            uint8_t *pHidInData = &drvUac10ParaHid.hidPacketSend[0];
            UsbHidPreDecode(pHidOutData, pHidInData, pHidPara);
            #endif
            // 解析处理，这里可能耗时较长
            if (drvUac10ParaHid.pHidUserParaHandlerCb != NULL)
            {
                drvUac10ParaHid.pHidUserParaHandlerCb((void *)pHidPara);
            }
        }
        #if 0   // 目前暂时用不到，如有需要，自行添加处理代码并打开条件编译
        else if (hidID == USB_HID_OUT_ID0)
        {

        }
        #endif
#else
        if (0)
        {
            ;
        }
#endif

#if (USB_HID_COL_UC_CTRL_EN)
        else if (hidID == USB_HID_OUT_LED_ID)
        {
            drvUac10ParaHid.hidLedState = pHidOutData[1];
        }
        else if (hidID == USB_HID_OUT_ASP_ID)
        {
            LibDrvUsbHidDecode(pHidOutData, (uint32_t)drvUac10ParaHid.aspIdx);
        }
        else if (hidID == USB_HID_TEL_VENDER_ID)
        {
            drvUac10ParaHid.pHidVenderHandlerCb(pHidOutData);
        }
#endif
        drvUac10ParaHid.hidPacketRdIdx++;
        if (drvUac10ParaHid.hidPacketRdIdx == USB_HID_OUT_PACKET_MAX)
        {
            drvUac10ParaHid.hidPacketRdIdx = 0;
        }
    }
}

/* Private function prototypes -----------------------------------------------*/
#if (USB_HID_SEND_TEST_EN != 0)
static void UsbHidSendExample(void)
{
    if (g_testUsbHidSendCtrlFlag != 0)
    {
        if (g_testUsbHidSendCtrlFlag < USB_HID_SEND_CTRL_NUM)
        {
            uint32_t ctrlCode = usbHidSendCtrlTb[g_testUsbHidSendCtrlFlag];
            DrvUsbHidSend2Byte(ctrlCode);
            UsbHidTickDelay(10);
            ctrlCode &= 0xFF00;
            DrvUsbHidSend2Byte(ctrlCode);
        }
        g_testUsbHidSendCtrlFlag = 0;
    }
    if (g_testUsbHidSendKeyboard != 0)
    {
        // PageUp, g_testUsbHidSendKeyboard = 0x004B;
        // PageDown, g_testUsbHidSendKeyboard = 0x004E;
        UsbHidTickDelay(1000);
        g_testUsbHidSendKeyboard |= 0x080000;
        DrvUsbHidSend3Byte(g_testUsbHidSendKeyboard);
        UsbHidTickDelay(10);
        g_testUsbHidSendKeyboard = 0x080000;
        DrvUsbHidSend3Byte(g_testUsbHidSendKeyboard);
        g_testUsbHidSendKeyboard = 0;
    }
}
#endif

#if USB_HID_EXT_FLASH_UPGRADE_EN
static void UsbHidFlashWriteHandle(uint8_t *pByteRecv)
{
    static uint32_t cacheAddr = USB_HID_EXT_FLASH_CACHE_ADDR;
    static uint32_t writeAddr = USB_HID_EXT_FLASH_UPGRADE_ADDR;
    static uint32_t readIndex = 0;
    static uint32_t packetIdx = 0;
    static uint32_t isFlashWriteIn = 0; // 0：初始化 0x3E: flash传输中 0x3F: 传输完毕且无错误  按自己需求使用
    
    uint32_t time = HAL_TickGet();
    
    uint32_t cacheSize = USB_HID_EXT_FLASH_CACHE_SIZE;

    uint32_t *pUintRecv = (uint32_t *)pByteRecv;
    uint32_t addr = pByteRecv[2] | (pByteRecv[3] << 8);
    DBG_PRINT("Protocol addr 0x%x.\n", addr);
    
    pByteRecv[0] = 0x11;
    if(addr == 0x4010)      //接收擦除参数，执行flash擦除任务
    {
        uint32_t flashAddr, blkEraseSize, blkEraseType;
        flashAddr = pUintRecv[1];
        blkEraseType = pUintRecv[2];
        blkEraseSize = pUintRecv[3];
        
        DBG_PRINT("Erase flash addr %x\n", flashAddr);
        
        isFlashWriteIn = 0x3E;
        cacheAddr = USB_HID_EXT_FLASH_CACHE_ADDR;
        writeAddr = USB_HID_EXT_FLASH_UPGRADE_ADDR;
        readIndex = 0;
        packetIdx = 0;
        memset((uint8_t *)writeAddr, 0xff, cacheSize);
        
        uint32_t arg[3];
        arg[0] = flashAddr;
        arg[1] = blkEraseSize;
        if(blkEraseType == 4)
        {
            arg[2] = 1;
        }
        else if(blkEraseType == 32)
        {
            arg[2] = 2;
        }
        else if(blkEraseType == 64)
        {
            arg[2] = 3;
        }
        DBG_PRINT("Flash erase type %d.\n", arg[2]);
        
        if(hal_flash_erase(arg) == 0)
        {
            pByteRecv[1] = 1;
            DBG_PRINT("HID Erase success.\n");
        }
        else
        {
            pByteRecv[1] = 0;
            DBG_PRINT("HID Erase fail.\n");
        }
    }
    else if(addr == 0x4011) //写入命令
    {
        uint32_t packetLen = pByteRecv[1];
        uint8_t* pData = (uint8_t *)&pUintRecv[1];
        
        pByteRecv[1] = 1;
        //如果这一包数据不能填充满缓存，直接复制到缓存里
        if(readIndex + packetLen < cacheSize)
        {
            memcpy((uint8_t *)cacheAddr, pData, packetLen);
            cacheAddr += packetLen;
            readIndex += packetLen;
            packetIdx++;
            DBG_PRINT("cache %d byte data.\n", packetLen);
        }
        //如果能填充满或者溢出，就先填满然后全部写入，再将剩余数据填入缓存
        else
        {
            int len = cacheSize - readIndex;
            memcpy((uint8_t *)cacheAddr, pData, len);
            readIndex += len;
            DBG_PRINT("cache %d byte data.\n", len);
            uint32_t arg[3];
            arg[0] = writeAddr;
            arg[1] = readIndex;
            arg[2] = USB_HID_EXT_FLASH_CACHE_ADDR;
            DBG_PRINT("cache fully, ");
            
            if (hal_flash_write(arg) == 0)
            {
                DBG_PRINT("program %d byte data.\n", cacheSize);
                memset((void *)USB_HID_EXT_FLASH_CACHE_ADDR, 0xFF, cacheSize);
                writeAddr += readIndex;
                cacheAddr = USB_HID_EXT_FLASH_CACHE_ADDR;
                readIndex = 0;
                packetIdx++;
                DBG_PRINT("flash write block success.\n");
            }
            else
            {
                pByteRecv[1] = 0;
            }
            memcpy((uint8_t *)cacheAddr, pData + len, packetLen - len);
            cacheAddr += (packetLen - len);
            readIndex += (packetLen - len);
            DBG_PRINT("cache %d byte data.\n", packetLen - len);
        }
        
        uint32_t checksum = pByteRecv[60] | (pByteRecv[61] << 8) | (pByteRecv[62] << 16) | (pByteRecv[63] << 24);
        uint32_t sum = 0;
        for(size_t i = 0; i < packetLen; i++)
        {
            sum += pData[i];
        }

        pByteRecv[2] = packetIdx & 0xFF;
        pByteRecv[3] = (packetIdx >> 8) & 0xff;

        pUintRecv[1] = checksum;
        pByteRecv[1] = (checksum == sum)? 1 : 0;        
    }
    else if(addr == 0x4012)
    {
        uint32_t arg[3];
        arg[0] = writeAddr;
        arg[1] = readIndex;
        arg[2] = USB_HID_EXT_FLASH_CACHE_ADDR;

        if (hal_flash_write(arg) == 0)
        {
            pByteRecv[1] = 1;
            DBG_PRINT("flash write block success.\n");
        }
        else
        {
            pByteRecv[1] = 0;
        }
    }
    else if(addr == 0x4013)
    {        
        uint32_t flashAddr = pUintRecv[1];
        uint32_t packetLen = pUintRecv[2];
        uint32_t checkSum  = pUintRecv[3];
        
        uint8_t* pData = (uint8_t *)flashAddr;
        

        DBG_PRINT("Checksum addr 0x%x, len %d.\n", flashAddr, packetLen);
        uint32_t check = 0;
        for(size_t i = 0; i < packetLen; i++)
        {
            check += pData[i];
        }
        pUintRecv[1] = check;
        
        if(check == checkSum)
        {
            isFlashWriteIn = 0x3F;
            pByteRecv[1] = 1;
        }
        else
        {
            pByteRecv[1] = 0;
        }
        DBG_PRINT("Device checksum %d, host checksum %d.\n", check, checkSum);
    }
    DBG_PRINT("Once packet time：%d ms.\n, systick:%d.\n", HAL_TickGet() - time, HAL_TickGet());
  
    
    uint32_t curTick = UsbHidTickGet();
    LibDrvUac10HidInSend(pByteRecv, 64);
    
    while ((LibDrvUac10StateGet() & BIT3) != 0)
    {
        if ((UsbHidTickGet() - curTick) > USB_HID_IN_TIME_OUT)
        {   
            // HID超时后仍然未发送成功，则复位重发
            LibDrvUac10HidInSendReset();
            curTick = UsbHidTickGet();
            LibDrvUac10HidInSend(pByteRecv, 64);
        }
    }
}
#endif

// 预解码，将有效数据保存，并回复应答包数据
// pHidOut:     指向HID-OUT数据包的指针（从PC接收的）
// pHidIn:      指向HID-IN数据包的指针（给PC发送的）
// pParaBuff:   指向参数解析缓存，用于后续解析处理参数
static void UsbHidPreDecode(uint8_t *pHidOut, uint8_t *pHidIn, uint32_t *pParaBuff)
{
    uint8_t length = pHidOut[1];
    uint32_t addr = (uint32_t)pHidOut[2] + ((uint32_t)pHidOut[3] << 8);
#if USB_HID_EXT_FLASH_UPGRADE_EN
    if ((addr >= 0x4010) && (addr < 0x4020))
    {
        UsbHidFlashWriteHandle(pHidOut);
        return;
    }
#endif
    pHidIn[0] = USB_HID_IN_ID1;
    if ((length > 0) && (length < 16))
    {
        if (addr == 0x4000)
        {
            pHidIn[1] = (uint8_t)UsbUserInfoHandler(length, (uint32_t *)&pHidOut[4]);
            LibDrvMemcpy(&pHidIn[8], &pHidOut[8], 4);
        }
        else
        {
            if (pParaBuff[0] == 0)
            {   // 上一包待处理的数据已经处理完成
                pParaBuff[0] = (uint32_t)length;
                pParaBuff[1] = (uint32_t)addr;
                LibDrvMemcpy((UINT8 *)&pParaBuff[2], &pHidOut[4], length*4);
                pHidIn[1] = 0x00;
            }
            else
            {   // 上一包待处理的数据未处理完成，返回值
                pHidIn[1] = (uint8_t)pParaBuff[0];
            }
        }
    }
    else
    {
        pHidIn[1] = 0xFF;
    }
    LibDrvUac10HidInSend(pHidIn, 64);
    uint32_t curTick = UsbHidTickGet();
    while ((LibDrvUac10StateGet() & BIT3) != 0)
    {
        if ((UsbHidTickGet() - curTick) > USB_HID_IN_TIME_OUT)
        {   // 等了USB_HID_IN_TIME_OUT 毫秒，仍然未发送成功，则直接复位
            LibDrvUac10HidInSendReset();
            break;
        }
    }
    return;
}


#endif


