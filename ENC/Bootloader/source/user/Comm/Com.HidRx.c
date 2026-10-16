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
**	文件名	: Com.HidRx.c
**
**	程序员	: 
**
**	日　期  : 2023年1月12日
**
**	文件描述: HID接收解析，使用环形缓冲区来接收。
**
****************************************************************************************************
*/
#include "main.h"



/* 宏定义 */


/* 函数声明 */
extern void ComFuncEntry(uint8 cmd, uint8 *pData, uint32 len);
extern void ComErrorAck(uint8 err);
extern void ComTxDfuStatus(uint32 status);

/* 变量定义和声明 */
#define COM_CP_BUF_SIZE	(1024)
static uint8 CpDataBuf[COM_CP_BUF_SIZE] = {0};

static CP_DATA_PKT_CLASS  Packet =
{
	.Begin   = 0,
	.LenLow  = 0,
	.LenHigh = 0,
	.Addr    = 0,
	.Cmd     = 0,
	.Data    = CpDataBuf,
	.CrcLow  = 0,
	.CrcHigh = 0,
	
	.Len     = 0,
};

static CP_DATA_PKT_RX_SM  StaMac =
{
	.Timeout    = 0,
	.State      = RX_STATE_BEGIN,
	.RxCnt      = 0,
	.Completed  = 0,
};


/*
****************************************************************************************************
**
**                                           HidRxStaMac()
**
**
**	功能描述: 帧处理函数
**
**	参  数  : 接收到的数据
**
**	返回值  : 
**
**	备  注	: 对接收内容进行帧分析，不正确的帧、不完整的帧、校验和错误等，丢掉。重新等待串口数
**			  据接收。
**
****************************************************************************************************
*/
void HidRxStaMac(void)
{
	uint8  rdData;
	uint32 crcRcv;
	static uint32 crcCalc = 0;



	if ( !Hid.RxFiFoRead(&rdData) )
	{
		return;
	}

	StaMac.Timeout = 300;

	switch(StaMac.State)
	{
		case RX_STATE_BEGIN:
			if (rdData == SB_CP_START)
			{
				Packet.Begin = rdData;
				crcCalc = rdData;
				StaMac.State++;
			}
			else
			{
				StaMac.State = RX_STATE_BEGIN;
			}
			break;

		case RX_STATE_LEN_LOW:
			Packet.LenLow = rdData;
			StaMac.State++;
			break;
			
		case RX_STATE_LEN_HIGH:
			Packet.LenHigh = rdData;
			Packet.Len = ((Packet.LenHigh << 8) | Packet.LenLow) & 0xFFFF;

			if(Packet.Len < (COM_CP_BUF_SIZE-1))
			{
				StaMac.State++;
				StaMac.RxCnt = 0;
			}
			else
			{
				StaMac.State = RX_STATE_BEGIN;
			}
			break;

		case RX_STATE_ADDR:
			if(rdData == 0x00)
			{
				Packet.Addr = rdData;
				StaMac.State++;
			}
			else
			{
				ComErrorAck(CP_ACK_DEV_ADDR_ERR);
				StaMac.State = RX_STATE_BEGIN;
			}
			break;

		case RX_STATE_CMD:
			Packet.Cmd = rdData;
			crcCalc = Packet.Begin + Packet.LenLow + Packet.LenHigh + Packet.Addr + Packet.Cmd;
			StaMac.State++;
			break;
			
		case RX_STATE_DATA_CRC:
			if (StaMac.RxCnt < Packet.Len-7)
			{
				Packet.Data[StaMac.RxCnt] = rdData;
				StaMac.RxCnt++;
				crcCalc += rdData;
			}
			else if (StaMac.RxCnt == Packet.Len-7)		// CRC Low
			{
				StaMac.RxCnt++;
				Packet.CrcLow = rdData;
			}
			else										// CRC High
			{
				Packet.CrcHigh = rdData;
				crcRcv = (Packet.CrcHigh<<8) | Packet.CrcLow;
				
				crcCalc = (~crcCalc + 1) & 0xFFFF;

				if ( crcRcv == crcCalc )
				{
					Packet.Cmd &= 0x7F;
					StaMac.Completed = true;
				}
				else
				{
					ComErrorAck(CP_ACK_CHECKSUM_ERR);
				}

				StaMac.State = RX_STATE_BEGIN;
				StaMac.RxCnt = 0;
			}
			break;
			
		default:
			break;
	}
}


/*
****************************************************************************************************
**
**                                         HidRxProcess()
**
**
**	功能描述: HID接收处理任务
**
**	参  数  : 
**
**	返回值  : 
**
**	备  注	: 对接收到的正确帧进行解析，然后把数据送往相应的处理函数
**
****************************************************************************************************
*/
void HidRxProcess(void)
{
	HidRxStaMac();											// HID接收状态机

	if(StaMac.Completed)
	{
		StaMac.Completed = false;
		ComFuncEntry(Packet.Cmd, Packet.Data, Packet.Len-7);
	}

    if (OsTick.TimerMs(&StaMac.Timeout, TFB_HID_RX_TO))     // HID接收超时
    {
        StaMac.State = RX_STATE_BEGIN;
        StaMac.RxCnt = 0;
    }
    
    if(Dfu.Progress.Value > 0)
    {
        if (Dfu.Program.GetCompletedFlag())
        {
            Dfu.Progress.DfuTimeOut = 0;
        }
    }
    
    if (OsTick.TimerMs(&Dfu.Progress.DfuTimeOut, TFB_DFU_TIME_OUT))//正在更新过程中，超时了，反馈给上位机代码错误
    {
        ComTxDfuStatus(DFU_STATUS_CODE_ERR);
        StaMac.State = RX_STATE_BEGIN;
        StaMac.RxCnt = 0;
        StaMac.Completed = false;
        
    }

}




