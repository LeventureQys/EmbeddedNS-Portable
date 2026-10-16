/*
*****************************************Copyright (c)**********************************************
**
**
**                                  深圳摩罗志远科技有限公司
**                             Shenzhen Maono Technology Co.,Ltd
**
**                                        www.maono.cn
**
**
**	文件名	: Hid.c
**
**	程序员	: 
**
**	日　期  : 2023年1月12日
**
**	文件描述: USB HID应用
**
****************************************************************************************************
*/
#include "OsTick.h"
#include "Hid.h"
#include "lib_drv_uac10.h"


/* 宏定义 */
#define USB_RX_FIFO_SIZE	(4096)	// 环形缓冲区大小，没有必要设置太大


/* 函数声明 */
void   HidInit(void);
void   HidSendBytes  (uint8 *data, uint32 len);
void   HidRxFifoWrite(uint8 *data, uint32 len);
uint32 HidRxFifoRead (uint8 *data);

extern void AppUsbHidSend(uint8 *data, uint32 len);


/* 变量定义和声明 */
HID_HANDLE Hid = 
{
	.Init        = HidInit,
	.SendBytes   = HidSendBytes,
    .RxFiFoWrite = HidRxFifoWrite,
    .RxFiFoRead  = HidRxFifoRead,
};

uint8  UsbHidRxFifo[USB_RX_FIFO_SIZE];
uint8  UsbHidTxBuf[64] = {0};
uint32 pRxFifoRd = 0;
uint32 pRxFifoWr = 0;


/*
****************************************************************************************************
**
**                                     HidInit()
**
**
**	功能描述: USB设备初始化 - USB HID
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void HidInit(void)
{
}


/*
****************************************************************************************************
**
**                                         HidSendBytes()
**
**
**	功能描述: USB HID，发送字节
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void HidSendBytesCheckBusyStatus(uint8 *data, uint32 len)
{
	int32  timeout = 100;
	uint32 busy = 1;
	
	
	while (busy)
	{
		busy = LibDrvUac10HidInSend(data, len);
		
		if (!busy)
		{
			break;
		}		
		
		if (--timeout <= 0)
		{
			break;
		}
		OsTick.DelayUs(20);
	}
}


/*
****************************************************************************************************
**
**                                         HidSendBytes()
**
**
**	功能描述: USB HID，发送字节
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void HidSendBytes(uint8 *data, uint32 len)
{
	uint32 i;
	
	
	while (len > 63)
	{
		UsbHidTxBuf[0] = USB_HID_IN_ID1;
		for (i=1; i<=63; i++)
		{
			UsbHidTxBuf[i] = *data++;
		}
		len -= 63;
		
		HidSendBytesCheckBusyStatus(UsbHidTxBuf, 64);
	}
	
	if (len)
	{
		UsbHidTxBuf[0] = USB_HID_IN_ID1;
		for (i=1; i<=len; i++)
		{
			UsbHidTxBuf[i] = *data++;
		}
		for (; i<=63; i++)
		{
			UsbHidTxBuf[i] = 0;
		}
		
		HidSendBytesCheckBusyStatus(UsbHidTxBuf, 64);
	}
}


/*
****************************************************************************************************
**
**                                         HidRxFifoWrite()
**
**
**	功能描述: 串口环形--接收缓冲区--写入数据
**
**	参  数  : wrDat   写入数据
**
**	返回值  : 
**
****************************************************************************************************
*/
void HidRxFifoWrite(uint8 *wrDat, uint32 len)
{
	uint32 pt;
	uint32 i;


	for (i=0; i<len; i++)
	{
		pt = (pRxFifoWr + 1) % USB_RX_FIFO_SIZE;

		if (pt != pRxFifoRd)
		{
			UsbHidRxFifo[pRxFifoWr] = *wrDat++;
			pRxFifoWr = pt;
		}
	}
}


/*
****************************************************************************************************
**
**                                         HidRxFifoRead()
**
**
**	功能描述: 串口环形--接收缓冲区--读出一字节数据
**
**	参  数  : rdDat   待存放数据的内存空间地址
**
**	返回值  : 成功返回TRUE，失败返回FALSE
**
****************************************************************************************************
*/
uint32 HidRxFifoRead(uint8 *rdDat)
{
	if (pRxFifoWr == pRxFifoRd)
    {
		return false;				// 读空返回0
    }
    else
    {
        *rdDat = UsbHidRxFifo[pRxFifoRd];
        //UsbRxFifo[pRxFifoRd] = 0;	// 读完清0

        pRxFifoRd = (pRxFifoRd + 1) % USB_RX_FIFO_SIZE;

        return true;
    }
}




