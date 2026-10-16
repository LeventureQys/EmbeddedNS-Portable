
#include "drv_uac10_user_config.h"

#if (UAC_IF_ID_STREAMING_MIDI > 0)
#include "lib_drv_uac10.h"

/* Private define ------------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/

/* Private function declarations ---------------------------------------------*/
static void UsbMidiSendExample(void);

/* Public variables ----------------------------------------------------------*/
#if (USB_MIDI_SEND_TEST_EN != 0)
// 用于测试MIDI上传控制
uint32_t g_testUsbMidiSendCtrlFlag;         // 写1给PC发送g_testMidiSendNoteOnOff表格内的midi码进行测试
uint32_t g_testUsbMidiThroughEn;            // 1, 直接把MIDI-OUT收到的数据，通过MIDI-IN会发给PC
uint8_t g_testMidiBuff[64];                 // 接收到的一包MIDI-OUT数据
#endif

/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
#if (USB_MIDI_SEND_TEST_EN != 0)
const uint8_t g_testMidiSendNoteOnOff[] = {
    0x09, 0x90, 0x3C, 0x7F,
    0x09, 0x90, 0x3C, 0x00,
};
#endif

/* Weak functions rewrite ----------------------------------------------------*/
/* Weak functions rewrite ----------------------------------------------------*/
// LIB库内定义的弱函数，此处重写，用于中断直接接收MIDI-OUT数据（PC下发的）
// pDataMem:    接收的数据源指针
// size:        字节数量，如果size=0，表示空包，特殊应用时需要额外处理，通常情况下不用管
#if 0       // 默认无需中断接收，如果主循环查询得方式太慢或可能丢码的话，这里添加适当得代码进行MIDI码的接收
__RAM_CODE void UsbMidiOutIrqHandler(uint8_t* pDataMem, size_t size)
{
    return;
}
#endif
/* Exported functions --------------------------------------------------------*/

void DrvUac10MidiHandler(void)
{
#if (USB_MIDI_SEND_TEST_EN != 0)
    UsbMidiSendExample();
#endif
    /*
        配合中断接收数据，进行解码处理，添加对应代码
    */
}

/* Private function prototypes -----------------------------------------------*/
#if (USB_MIDI_SEND_TEST_EN != 0)
static void UsbMidiSendExample(void)
{
    if (g_testUsbMidiSendCtrlFlag != 0)
    {
        LibDrvUac10MidiInSend(g_testMidiSendNoteOnOff, sizeof(g_testMidiSendNoteOnOff));
        while ((LibDrvUac10StateGet() & BIT6) != 0) {;}
    }
    if (g_testUsbMidiThroughEn == 0)
    {
        int32_t sizeRx = LibDrvUac10MidiOutReceive(g_testMidiBuff);
        if (sizeRx > 0)
        {
            LibDrvUac10MidiInSend(g_testMidiBuff, sizeRx);
            while ((LibDrvUac10StateGet() & BIT6) != 0) {;}
        }
    }
}
#endif

#endif
