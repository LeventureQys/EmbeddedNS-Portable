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
**	文件名	: ParamTable.DevInfo.h
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: 设备信息参数表
**
****************************************************************************************************
*/
#ifndef _DEVICE_INFO_H
#define _DEVICE_INFO_H
#include "UserRetype.h"



// 设备信息数据结构，协议中是Big-Endian格式
typedef struct
{
	uint16 ModelCode[3];	// Model Code是一个字符串，共6个字符
	uint16 ModelName[8];	// Device Name（设备名称）是一个字符串，共16个字符
	
	struct
	{
		uint16 X;			// 固件程序版本号，X.Y.Z.Build
		uint16 Y;
		uint16 Z;
		uint16 Build;
		uint16 Type;		// 版本类型，0 Demo, 1 Release
	} FirmwareVer;
	
	struct
	{
		uint16 X;			// DSP1程序版本号，X.Y.Z
		uint16 Y;
		uint16 Z;
	} Dsp1Ver;
	
	struct
	{
		uint16 X;			// DSP2程序版本号，X.Y.Z
		uint16 Y;
		uint16 Z;
	} Dsp2Ver;
	
	struct
	{
		uint16 VID;
		uint16 PID;
	} UsbInfo;
	
	struct
	{
		uint16 Year;		// 设备生产日期
		uint16 Month;
		uint16 Day;
	} Date;
	
	struct
	{
		uint16 Hour;		// 时间
		uint16 Minute;
		uint16 Second;
	} Time;
	
	uint16 SN[32];			// 设备生产序列号
} DEV_INFO_PARAM;

#endif




