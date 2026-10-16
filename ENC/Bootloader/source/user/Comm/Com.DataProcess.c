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
**	日　期  : 2023年1月12日
**
**	文件描述: Communication Data Process 通讯数据处理。应用于PC电脑软件控制。
**            一般是USB、串口等，它们共用处理功能。
**
****************************************************************************************************
*/
#include "main.h"



/* 函数声明 */
void ComErrorAck(uint32 err);
void ComTxCurrDfuStatus(uint32 status);


/* 变量定义和声明 */
#define CP_DATA_TX_MAX		(256)
uint8 TxPacket[CP_DATA_TX_MAX] = {0};


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
	TxPacket[idx++] = 0x00;
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

	Hid.SendBytes(TxPacket, idx);
}


/*
****************************************************************************************************
**
**                                        ComErrorAck()
**
**
**	功能描述: 主机发送的通讯帧有误，从机响应相应的错误代码，返回给主机
**
**	参  数  : err		
**
**
****************************************************************************************************
*/
void ComErrorAck(uint32 err)
{
	uint8 txBuf[2];
	
	
	txBuf[0] = (err     ) & 0xFF;
	txBuf[1] = (err >> 8) & 0xFF;
	
	ComTxPacket(CC_ACK, txBuf, sizeof(txBuf));
}


/*
****************************************************************************************************
**
**                                        ComErrorAck()
**
**
**	功能描述: 主机发送的通讯帧有误，从机响应相应的错误代码，返回给主机
**
**	参  数  : err		
**
**
****************************************************************************************************
*/
void ComTxCurrDfuStatus(uint32 status)
{
	uint8 txBuf[3];
	
	
	txBuf[0] = DFU_CMD_CURR_STATUS;
	txBuf[1] = CHIP_ID;
	txBuf[2] = status;
	
	ComTxPacket(CC_DFU, txBuf, sizeof(txBuf));
}


/*
****************************************************************************************************
**
**                                   ComRxFirmwareInfo()
**
**
**	功能描述：接收到升级固件的信息
**
**	参  数  ：data		接收到的数据
**			  len		
**
**	返回值  ：
**
****************************************************************************************************
*/
void ComRxFirmwareInfo(uint8 *data, uint32 len)
{
	uint32 status;


	status = Dfu.Program.ValidCheck(data, len);
	ComTxCurrDfuStatus(status);
}


/*
****************************************************************************************************
**
**                                   ComRxFirmwareCode()
**
**
**	功能描述：接收到升级固件的数据
**
**	参  数  ：data		接收到的数据
**			  len		
**
**	返回值  ：
**
****************************************************************************************************
*/
void ComRxFirmwareCode(uint8 *data, uint32 len)
{
	uint32 status;


	status = Dfu.Program.WriteFlash(data, len);
	ComTxCurrDfuStatus(status);
}

void ComTxDfuStatus(uint32 status)
{
    ComTxCurrDfuStatus(status);
}


#if 1
/* 正常来说，“读写”参数值是在MainApp固件中的。但存在一种情况，升级失败。
   MCU已经无法进入MainApp了，MCU上电就进入DFU模式。

   所以，在xxx_Dfu工程中，也要添加适当的参数“读写”操作。
*/
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
	uint16 paramAddr;
	uint16 *pData = (uint16*)data;


	
	paramAddr = *pData++;
	
	if (paramAddr == 0x1008)
	{
		ComErrorAck(CP_ACK_NO_ERR);
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
	uint16 paramAddr;
	uint32 paramNum;
	uint16 *pData = (uint16*)data;


	
	paramAddr = *pData++;	// 首地址
	paramNum  = *pData++;	// 参数数目	
	
	if (paramAddr == 0x1008 && paramNum == 1)
	{
		ComErrorAck(CP_ACK_NO_ERR);
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
	uint16 paramAddr;
	uint16 paramData;
	uint32 paramNum;
	uint32 i;
	uint8  txBuf[64];
	uint32 cnt = 0;
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
		
		if (paramAddr < 0x000B)
		{
			paramData = ParamTbl.DevInfo[paramAddr];
			
			txBuf[cnt++] = (paramAddr     ) & 0xFF;
			txBuf[cnt++] = (paramAddr >> 8) & 0xFF;
			txBuf[cnt++] = (paramData     ) & 0xFF;
			txBuf[cnt++] = (paramData >> 8) & 0xFF;
		}
		else if (paramAddr == 0x1008)
		{
			paramData = 0x0001;
			
			txBuf[cnt++] = (paramAddr     ) & 0xFF;
			txBuf[cnt++] = (paramAddr >> 8) & 0xFF;
			txBuf[cnt++] = (paramData     ) & 0xFF;
			txBuf[cnt++] = (paramData >> 8) & 0xFF;
		}
	}
	
	ComTxPacket(CC_PARAM_RAND_RD, txBuf, paramNum*4);
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
	uint16 paramData;
	uint32 paramNum;
	uint32 i;
	uint8  txBuf[64];
	uint32 cnt = 0;
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
	
	if (paramAddr < 0x000B)
	{
		txBuf[cnt++] = (paramAddr     ) & 0xFF;
		txBuf[cnt++] = (paramAddr >> 8) & 0xFF;
		txBuf[cnt++] = (paramNum      ) & 0xFF;
		txBuf[cnt++] = (paramNum  >> 8) & 0xFF;
		
		for (i=0; i<paramNum; i++)
		{
			paramData = ParamTbl.DevInfo[paramAddr + i];

			txBuf[cnt++] = (paramData     ) & 0xFF;
			txBuf[cnt++] = (paramData >> 8) & 0xFF;
		}
	}
	else if (paramAddr == 0x1008)
	{
		txBuf[cnt++] = (paramAddr     ) & 0xFF;
		txBuf[cnt++] = (paramAddr >> 8) & 0xFF;
		txBuf[cnt++] = (paramNum      ) & 0xFF;
		txBuf[cnt++] = (paramNum  >> 8) & 0xFF;
		
		paramData = 0x0001;
		txBuf[cnt++] = (paramData     ) & 0xFF;
		txBuf[cnt++] = (paramData >> 8) & 0xFF;
	}

	ComTxPacket(CC_PARAM_SEQU_RD, txBuf, paramNum * 2 + 4);
}
#endif


/*
****************************************************************************************************
**
**                                        ComDfuEntry()
**
**
**	功能描述: 与Dfu相关的操作
**
**	参  数  : data		数据头地址
**			  len		数据长度
**
**
****************************************************************************************************
*/
void ComDfuEntry(uint8 cmd, uint8 *data, uint32 len)
{
	uint8 dfuCmd = *data;


	data += 2;		// 跳过两个字节(DFU_CMD, CHIP_ID)
	len  -= 2;

	switch (dfuCmd)
	{
		case DFU_CMD_NEW_FW_INFO:
			ComRxFirmwareInfo(data, len);
			break;
		
		case DFU_CMD_CODE_BLOCK:
			ComRxFirmwareCode(data, len);
			break;
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

const COM_FUNC_PF ComFuncTable[12] =
{
	ComFuncEmpty,			// 0x00
	ComFuncEmpty,			// 0x01
	ComFuncEmpty,			// 0x02
	ComParamRandWrite,		// 0x03
	ComParamRandRead,		// 0x04
	ComParamSequWrite,		// 0x05
	ComParamSequRead,		// 0x06
	ComFuncEmpty,			// 0x07
	ComFuncEmpty,			// 0x08
	ComFuncEmpty,			// 0x09
	ComFuncEmpty,			// 0x0A
	ComDfuEntry,			// 0x0B
};

void ComFuncEntry(uint8 cmd, uint8 *data, uint32 len)
{
	if (cmd < SIZE_OF(ComFuncTable))
	{
		ComFuncTable[cmd](cmd, data, len);
	}
}




