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
**	文件名	: Com.DataProcess.c
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: Communication Data Process 通讯数据处理。应用于PC电脑软件控制。
**            一般是USB、串口等，它们共用处理功能。
**
****************************************************************************************************
*/
#include "UserRetype.h"
#include "OsTick.h"
#include "Hid.h"
#include "Iap.h"
#include "Dsp.h"
#include "Com.h"
#include "ParamTable.h"
#include "Memory.h"
#include "Fw.Upgrade.h"
#include "Fw.Info.h"
#include "Fw.Version.h"



/* 函数声明 */
void ComSavePreset(void);
void ComLoadPreset(void);
void ComDeletePreset(void);
void ComConfigParamProc(uint16 *paramAddr, uint32 *dspUpdate);
void ComDevInfoParamProc(uint16 *paramAddr);
void ComTxAck(uint32 err);


/* 变量定义和声明 */
#define CP_DATA_TX_MAX		(1*512)
uint8 TxPacket[CP_DATA_TX_MAX + 7 + 1] = {0};

uint32 SyncFlagTimer = 0;


/*
****************************************************************************************************
**
**                                         ComSendBytes()
**
**
**	功能描述: 底层数据发送函数
**
**	参  数  : len  字节长度
**
**	返回值  : 
**
****************************************************************************************************
*/
void ComSendBytes(uint8 *data, uint32 len)
{
	Hid.SendBytes(data, len);
}


/*
****************************************************************************************************
**
**                                         ComTxPacket()
**
**
**	功能描述: 发送一帧数据
**
**	参  数  : cmd   	帧命令字节
**            data		发送数据地址
**            len		发送数据长度
**
**	返回值  : 
**
****************************************************************************************************
*/
void ComTxPacket(uint8 cmd, uint8 *data, uint32 len)
{
	uint32 i, idx = 0;
	uint32 nCRC = 0;



	TxPacket[idx++] = SB_CP_START;
	TxPacket[idx++] = ((len + 7)     ) & 0xFF;
	TxPacket[idx++] = ((len + 7) >> 8) & 0xFF;
	TxPacket[idx++] = ParamTbl.Config->DeviceAddr;
	TxPacket[idx++] = cmd;

	for (i=0; i<len; i++)
	{
		TxPacket[idx++] = data[i];
	}

	// 计算SigmaCRC
	for (i=0; i<idx; i++)
	{
		nCRC += TxPacket[i];
	}
	nCRC = ~nCRC + 1;
	TxPacket[idx++] = (nCRC   ) & 0xFF;
	TxPacket[idx++] = (nCRC>>8) & 0xFF;

	ComSendBytes(TxPacket, idx);
}


/*
****************************************************************************************************
**
**                                         ComTxPacket2()
**
**
**	功能描述: 发送一帧数据。调用本函数前，已经对TxPacket进行赋值了。
**            本函数是针对大包数据帧而设计的
**
**	参  数  : cmd   	帧命令字节
**            data		发送数据地址
**            len		发送数据长度
**
**	返回值  : 
**
****************************************************************************************************
*/
void ComTxPacket2(uint8 cmd, uint32 len)
{
	uint32 i, idx = 0;
	uint32 nCRC = 0;


	TxPacket[idx++] = SB_CP_START;
	TxPacket[idx++] = ((len + 7)     ) & 0xFF;
	TxPacket[idx++] = ((len + 7) >> 8) & 0xFF;
	TxPacket[idx++] = ParamTbl.Config->DeviceAddr;
	TxPacket[idx++] = cmd;

	idx += len;

	// 计算SigmaCRC
	for (i=0; i<idx; i++)
	{
		nCRC += TxPacket[i];
	}
	nCRC = ~nCRC + 1;
	TxPacket[idx++] = (nCRC   ) & 0xFF;
	TxPacket[idx++] = (nCRC>>8) & 0xFF;

	ComSendBytes(TxPacket, idx);
}


/*
****************************************************************************************************
**
**                                   ComTxParamRandWrite()
**
**
**	功能描述: 下位机往上位机发送参数 - 随机写参数，data是参数地址+参数数据的组合
**
**	参  数  : data		参数地址与参数值的组合
**			  len		地址和参数都包括
**
**	返回值  : 
**
****************************************************************************************************
*/
void ComTxParamRandWrite(uint16 *data, uint32 len)
{
	ComTxPacket(CC_PARAM_RAND_WR, (uint8*)data, len * sizeof(uint16));
}


/*
****************************************************************************************************
**
**                                        ComTxAck()
**
**
**	功能描述: 主机发送的通讯帧有误，从机响应相应的错误代码，返回给主机
**
**	参  数  : err		
**
**
****************************************************************************************************
*/
void ComTxAck(uint32 err)
{
	uint8 txBuf[2];
	
	
	txBuf[0] = (err     ) & 0xFF;
	txBuf[1] = (err >> 8) & 0xFF;
	
	ComTxPacket(CC_ACK, txBuf, sizeof(txBuf));
}


/*
****************************************************************************************************
**
**                                   ComParamRandWrite()
**
**
**	功能描述: 上位机设置下位数据 - 随机写参数，data是参数地址+参数数据的组合
**
**	参  数  : data		接收到的数据
**			  len		数据长度（不包括cmd, checksum）
**
**	返回值  : 
**
****************************************************************************************************
*/
void ComParamRandWrite(uint8 cmd, uint8 *data, uint32 len)
{
	uint32 paramType = 0;
	uint16 paramAddr;
	uint16 ParamData;
	uint32 dspUpdate = 0;
	uint32 i;
	uint32 lp = len/4;
	uint16 *pData = (uint16*)data;



	if (len % 4) return;	// 在参数随机写的时候，地址+数据组合，CP_DATA的数据长度必须是4的倍数

	for (i=0; i<lp; i++)
	{
		paramAddr = *pData++;
		ParamData = *pData++;
		
		paramType = Mem.ParamWrite.Rand(&paramAddr, &ParamData);
		
		if (paramType == IS_CFG_DATA)
		{
			ComConfigParamProc(&paramAddr, &dspUpdate);
//			if (dspUpdate)
//			{
//				Dsp.Program->AMSet.RandParam(data, len);
//			}
		}
		else if (paramType == IS_DEV_INFO)
		{
			ComDevInfoParamProc(&paramAddr);
		}
		else if (paramType == 0)
		{
			break;
		}
	}

	if (paramType == IS_CURR_PRESET)
	{
		Dsp.Program->AMSet.RandParam(data, len);
		ComTxAck(CP_ACK_NO_ERR);			// 返回无错误响应
		
		Mem.PresetParamAutoSavePreview();	// 自动保存预处理
	}
	else if (paramType == IS_DEV_INFO || paramType == IS_CFG_DATA)
	{
		ComTxAck(CP_ACK_NO_ERR);
	}
}


/*
****************************************************************************************************
**
**                                   ComParamSequWrite()
**
**
**	功能描述: 上位机设置下位数据 - 连续写参数，data是参数首地址+参数数目+参数数据
**
**	参  数  : data		接收到的数据
**			  len		数据长度（不包括cmd, checksum）
**
**	返回值  : 
**
****************************************************************************************************
*/
void ComParamSequWrite(uint8 cmd, uint8 *data, uint32 len)
{
	uint32 paramType = 0;
	uint16 paramAddr;
	uint16 ParamData;
	uint32 dspUpdate = 0;
	uint32 ParamNum;
	uint32 i;
	uint16 *pData = (uint16*)data;


	
	paramAddr = *pData++;
	ParamNum  = *pData++;
	
	if ((ParamNum*2+4) != len)
		return;
	
	for (i=0; i<ParamNum; i++)
	{
		ParamData = *pData++;
		
		paramType = Mem.ParamWrite.Rand(&paramAddr, &ParamData);
		
		if (paramType == IS_CFG_DATA)
		{
			ComConfigParamProc(&paramAddr, &dspUpdate);
//			if (dspUpdate)
//			{
//				Dsp.Program->AMSet.RandParam(data, len);
//			}
		}
		else if (paramType == IS_DEV_INFO)
		{
			ComDevInfoParamProc(&paramAddr);
		}
		else if (paramType == 0)
		{
			break;
		}
		
		paramAddr++;
	}

	if (paramType == IS_CURR_PRESET)
	{
		paramAddr = data[1]<<8 | data[0];
		
		Dsp.Program->AMSet.SequParam(&paramAddr, ParamNum);
		ComTxAck(CP_ACK_NO_ERR);			// 返回无错误响应
		
		Mem.PresetParamAutoSavePreview();	// 自动保存预处理
	}	
	else if (paramType == IS_DEV_INFO || paramType == IS_CFG_DATA)
	{
		ComTxAck(CP_ACK_NO_ERR);
	}
}


/*
****************************************************************************************************
**
**                                    ComParamRandRead()
**
**
**	功能描述: 上位机读取下位数据 - 随机读参数，data是参数地址序列
**
**	参  数  : data		接收到的数据
**			  len		数据长度（不包括cmd, checksum）
**
**	返回值  : 
**
****************************************************************************************************
*/
void ComParamRandRead(uint8 cmd, uint8 *data, uint32 len)
{
	uint32 paramType;
	uint16 paramAddr;
	uint16 paramData;
	uint32 paramNum;
	uint32 i;
	uint8  *txBuf = TxPacket + 5; // 直接指向TxPacket中的CP_DATA位置
	uint16 *pData = (uint16*)data;



	/* 地址队列，CP_DATA的数据长度必须是2的倍数。返回帧中是地址和数据，所以帧允许的最大字节数：
	   (len / 2) * 4
	*/
	if ((len % 2) || (len*2 > CP_DATA_TX_MAX))
	{
		return;
	}

	paramNum = len / 2;
	
	for (i=0; i<paramNum; i++)
	{
		paramAddr = *pData++;

		paramType = Mem.ParamRead.Rand(&paramAddr, &paramData);
		
		if (paramType)
		{
			*txBuf++ = data[2*i  ];
			*txBuf++ = data[2*i+1];
			*txBuf++ = (paramData   ) & 0xFF;
			*txBuf++ = (paramData>>8) & 0xFF;
		}
		else
		{
			break;
		}
	}

	if (paramType)
	{
		txBuf = TxPacket + 5;
		ComTxPacket(cmd, txBuf, paramNum*4);
	}
}


/*
****************************************************************************************************
**
**                                    ComParamSequRead()
**
**
**	功能描述: 上位机读取下位数据 - 随机读参数，data是参数首地址+参数数目
**
**	参  数  : data		接收到的数据
**			  len		数据长度（不包括cmd, checksum）
**
**	返回值  : 
**
****************************************************************************************************
*/
void ComParamSequRead(uint8 cmd, uint8 *data, uint32 len)
{
	uint16 paramAddr;
	uint32 paramNum;
	uint32 memBlockType;
	uint32 addrOffset;
	uint8  *txBuf = TxPacket + 5; // 直接指向TxPacket中的CP_DATA位置
	uint16 *pData = (uint16*)data;


	
	paramAddr = *pData++;	// 首地址
	paramNum  = *pData++;	// 参数数目	
	
	/* 地址队列，CP_DATA的数据长度必须是2的倍数。返回帧中是地址和数据，所以帧允许的最大字节数：
	   paramNum * 2 + 4
	*/
	if ((len % 2) || ((paramNum * 2 + 4) > CP_DATA_TX_MAX))
	{
		return;
	}
	
	// 读的范围不能超出有效的范围
	memBlockType = paramAddr & MEM_ADDR_TYPE_MASK;
	addrOffset   = paramAddr & MEM_OFFSET_ADDR_MASK;
	
	if (memBlockType == MEM_DEV_INFO_BASE)
	{
		if (addrOffset + paramNum > DEV_INFO_TBL_SIZE)
			return;
	}
	else if (memBlockType == MEM_CONFIG_DATA_BASE)
	{
		if (addrOffset + paramNum > CONFIG_TBL_SIZE)
			return;
	}
	else
	{
		if (addrOffset + paramNum > PRESET_TBL_SIZE)
			return;
	}
	
	// 读并发送数据
	{
		*txBuf++ = (paramAddr   ) & 0xFF;
		*txBuf++ = (paramAddr>>8) & 0xFF;
		*txBuf++ = (paramNum    ) & 0xFF;
		*txBuf++ = (paramNum>>8 ) & 0xFF;
		
		Mem.ParamRead.Sequ(&paramAddr, txBuf, paramNum);
		ComTxPacket2(cmd, paramNum * 2 + 4);
	}
}


/*
****************************************************************************************************
**
**                                   ComDevInfoParamProc()
**
**
**	功能描述: 按指定的参数地址，执行DevInfo参数表中相应的处理
**
**	参  数  : paramAddr		参数头地址
**
**	返回值  : 
**
****************************************************************************************************
*/
void ComDevInfoParamProc(uint16 *paramAddr)
{
	if (*paramAddr == PARAM_SN_END)		// SN要发完所有32个参数，才会保存
	{
		FwInfoSNSave();
	}
}


/*
****************************************************************************************************
**
**                                   ComConfigParamProc()
**
**
**	功能描述: 按指定的参数地址，执行Config参数表中相应的处理
**
**	参  数  : paramAddr		参数头地址
**
**	返回值  : 
**
****************************************************************************************************
*/
void ComConfigParamProc(uint16 *paramAddr, uint32 *dspUpdate)
{
	uint32 saveOk = 1;
	
	
	*dspUpdate = 0;
	
	if (*paramAddr == PARAM_DFU)				// DFU
	{
		if (ParamTbl.Config->DFU)
		{
			Fwu.Start();
		}
		saveOk = 0;
	}
	// Liang
	else if (*paramAddr == PARAM_REC_FACTORY)	// Restore factory settings
	{
		Mem.ResFactory();
		saveOk = 0;
	}
	
	if (saveOk)
	{
		Mem.CfgParamAutoSavePreview();
	}
}


/*
****************************************************************************************************
**
**                                        ComFuncEmpty()
**
**
**	功能描述: 
**
**	参  数  : data		数据头地址
**			  len		数据长度
**
**
****************************************************************************************************
*/
void ComFuncEmpty(uint8 cmd, uint8 *data, uint32 len)
{
	// 保留一个空函数
}


/*
****************************************************************************************************
**
**                                         ComFuncEntry()
**
**
**	功能描述: 通讯功能处理入口函数
**
**	参  数  : cmd		命令字节
**			  data		数据头地址
**			  len		数据长度
**
**	返回值  : 
**
**  备  注  : 
**
****************************************************************************************************
*/
typedef void (*COM_FUNC_PF)(uint8 cmd, uint8 *data, uint32 len);

const COM_FUNC_PF ComFuncTable[7] =
{
	ComFuncEmpty,			// 0x00
	ComFuncEmpty,			// 0x01
	ComFuncEmpty,			// 0x02
	ComParamRandWrite,		// 0x03
	ComParamRandRead,		// 0x04
	ComParamSequWrite,		// 0x05
	ComParamSequRead,		// 0x06
};

void ComFuncEntry(uint8 cmd, uint8 *data, uint32 len)
{
	if (cmd < SIZE_OF(ComFuncTable))
	{
		ComFuncTable[cmd](cmd, data, len);
	}
}




