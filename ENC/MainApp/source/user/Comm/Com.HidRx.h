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
**	文件名	: Com.HidRx.h
**
**	程序员	: 
**
**  日　期  : 2021年2月1日
**
**	文件描述: HID接收解析，使用环形缓冲区来接收。
**
**
****************************************************************************************************
*/
#ifndef _COM_HID_RX_H
#define _COM_HID_RX_H
#include "UserRetype.h"



#define	SB_CP_START				(0xC4)		// SigmaBus协议


// 数据接收状态机的各个状态定义
typedef	enum
{
	RX_STATE_BEGIN = 0,
	RX_STATE_LEN_LOW,
	RX_STATE_LEN_HIGH,
	RX_STATE_ADDR,
	RX_STATE_CMD,
	RX_STATE_DATA_CRC,
} RX_STATE;


//数据帧结构
typedef	struct
{
	uint8 Begin;
	uint8 LenLow;
	uint8 LenHigh;
	uint8 Addr;			// Device Addr
	uint8 Cmd;
	uint8 *Data; 		// Cmd也包含在Data内
	uint8 CrcLow;		// CRC校验
	uint8 CrcHigh;
	
	uint16 Len;			// 16位的长度
} CP_DATA_PKT_CLASS;	// Dfu Communication Protocol Data Packet


//接收时，各阶段状态和标志
typedef	struct
{
	uint32 	 Timeout;

	RX_STATE State;		// 接收帧结构状态
	uint32	 RxCnt;		// 有效数据个数
	uint32 	 Completed;	// 完整接收一帧数据
} CP_DATA_PKT_RX_SM;	// 接收状态机 State Machine

#endif




