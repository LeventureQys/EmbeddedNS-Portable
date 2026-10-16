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
**	文件名	: Com.DataProcess.h
**
**	程序员	: 
**
**	日　期  : 2023年1月12日
**
**	文件描述: Communication Data Process 通讯数据处理。应用于PC电脑软件控制。
**            一般是USB、串口等，它们共用处理功能。
**
****************************************************************************************************
*/
#ifndef _COM_DATA_PROCESS_H
#define _COM_DATA_PROCESS_H
#include "UserRetype.h"



// DFU_CMD, 通讯帧的类型
typedef enum
{
	DFU_CMD_CURR_STATUS = 0x01,
	DFU_CMD_NEW_FW_INFO = 0x02,
	DFU_CMD_CODE_BLOCK  = 0x03,
} DFU_CMD;

// DFU当前状态
typedef enum
{
	DFU_STATUS_INFO_ERR         = 0x01,	// 固件信息错误
	DFU_STATUS_CODE_ERR         = 0x02,	// 固件代码错误
	DFU_STATUS_UPGRADE_CONTINUE = 0x03,	// 升级继续
	DFU_STATUS_UPGRADE_FAIL     = 0x04,	// 升级失败
	DFU_STATUS_UPGRADE_SUCCESS  = 0x05,	// 升级成功
} DFU_STATUS;


// 通讯帧错误响应
typedef enum
{
	CP_ACK_NO_ERR        = 0x0000,		// 无错误（成功）
	CP_ACK_DEV_ADDR_ERR  = 0x0001,		// 设备地址错误，设备地址与当前设备地址不匹配
	CP_ACK_LEN_ERR       = 0x0002,		// 长度错误，一般是超过一帧可容纳的最大长度
	CP_ACK_CHECKSUM_ERR  = 0x0003,		// 校验和错误
	CP_ACK_READ_ONLY_ERR = 0x0004,		// 参数只读，如果写只读参数会提示错误
	CP_ACK_DATA_ERR      = 0x0005,		// 参数值错误，一般是写某个参数时，值不合理或超范围，会提示错误
} CP_ACK;

// 通讯帧CMD定义
typedef enum
{
	CC_ACK = 0x02,				// 响应帧命令
	CC_DFU = 0x0B,				// 固件升级命令
					   
	CC_PARAM_RAND_WR = 0x03,	// 随机写参数
	CC_PARAM_RAND_RD = 0x04,	// 随机读参数
	CC_PARAM_SEQU_WR = 0x05,	// 连续写参数
	CC_PARAM_SEQU_RD = 0x06,	// 连续读参数
} CP_CMD;

#endif




