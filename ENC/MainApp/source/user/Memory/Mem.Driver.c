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
**	文件名	: Mem.Driver.c
**
**	程序员	:  
**
**	日　期  : 2021年12月23日
**
**	文件描述: 这是Memory读写驱动层。也可以认为是数据存储的中间层。应用层为顶层，本文件为中间层，而
**            各存储设备（片内Flash，片外Flash，片外EEPROM等）是底层。
**
**            这里要完成的功能是: 
**            1. 对保存的数据加“时间戳”，对多个Bank依次存储
**            2. 使用多个Bank进行存储，目的是延长存储设备的寿命。
**            3. 对数据分块保存，每块分配一个校验和，增加数据的安全性。
**
****************************************************************************************************
*/
/*
+--------------------------------------------------------------------------------------------------+
|   说明                                                                                           |
|                                                                                                  |
+-----+----------------+---------------------------------------------------------------------------+
|  1  |  TS            |  时间戳，uint32类型，4个字节。                                            |
|     |                |                                                                           |
|	  |                |  写入时，找未存过的Bank或最小时间戳对应的Bank来存储。                     |
|	  |                |  读取时，找最大时间戳对应的Bank来读取。                                   |
+-----+----------------+---------------------------------------------------------------------------+
|  2  |  CRC           |  块数据校验和，一般使用MCU的CRC32，所以是uint32类型，4个字节。	           |
|	  |                |  MCU自带的CRC32计算效率是很高的，如果自定义CRC，计算效率反而低了。        |
+-----+----------------+---------------------------------------------------------------------------+
|  3  |  NumOfBank     |  一个Bank就可以存储完用户数据。使用多个Bank来存储，目的是延长存储寿命     |
|	  |                |                                                                           |
+-----+----------------+---------------------------------------------------------------------------+
|  4  |  Size of Bank  |  一个Bank所占的字节数目。它由用户保存的总数据大小来决定。可以是半个       |
|     |                |  扇区，也可以是一个扇区，甚至多个扇区。                                   |
|	  |				   |                                                                           |
|	  |                |  1 Bank = 用户数据，时间戳，块校验和                                      |
+-----+----------------+---------------------------------------------------------------------------+
|  5  |  Size of Data  |  用户要保存的数据总大小，它小于Bank的大小，即只存部分数据，多余的空       |
|	  |                |  间空着。                                                                 |
+-----+----------------+---------------------------------------------------------------------------+
|  6  |  数据补充      |  当用户保存数据大小，未满最后一个Block时，约定使用FF来填充。即该Block     |
|	  |                |  的CRC，也会计算补充的FF。                                                |
+-----+----------------+---------------------------------------------------------------------------+
|  7  |  双备份        |  在NumOfBank >= 2的情况下，可以实现双备份。如果读取当前ts的Bank不成功，   |
|	  |                |  那么可以继续读取上一个ts的Bank的数据。如果都不成功，那么意味着读取失败。 |
+-----+----------------+---------------------------------------------------------------------------+
*/
#include <string.h>
#include "Iap.h"
#include "Crc.h"
#include "Mem.Driver.h"



/* 函数声明 */
void   MemWriteInit(MEM_DRIVER_HANDLE *mh);
uint32 MemWriteWithCRC(uint32 ts, uint32 bankIdx, MEM_DRIVER_HANDLE *mh);


/*
****************************************************************************************************
**
**                                        CRC_CALC()
**
**
**	功能描述: 数据校验和计算，可以使用MCU自带的硬件CRC外设来做
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
#define CRC_CALC	Crc32.Calc


/*
****************************************************************************************************
**
**                                        FlashWrite()
**
**
**	功能描述: 存储设备底层驱动，片内Flash，片外Flash，片外EEPROM等
**
**	参  数  : 
**
**	返回值  : 1 正确，0 错误
**
****************************************************************************************************
*/
uint32 FlashWrite(uint32 addr, uint8 *data, uint32 len_bytes)
{
	uint32 ret;
	
	

	/* 做地址重映射，可以把所有的存储设备合并到一起 */
//	if ((addr & 0xF0000000) == 0x00000000)		// 片内Flash
//	{
		ret = Iap.Write(addr, (uint32*)data, len_bytes);
//	}
//	else if ((addr & 0xF0000000) == 0x10000000)	// 片外Flash
//	{
//	}
//	else if ((addr & 0xF0000000) == 0x20000000)	// 片外EEPROM
//	{
//	}
	
	return ret;
}


/*
****************************************************************************************************
**
**                                        FlashRead()
**
**
**	功能描述: 存储设备底层驱动，片内Flash，片外Flash，片外EEPROM等
**
**	参  数  : 
**
**	返回值  : 1 正确，0 错误
**
****************************************************************************************************
*/
uint32 FlashRead(uint32 addr, uint8 *data, uint32 len_bytes)
{
	uint32 ret;
	
	
	/* 做地址重映射，可以把所有的存储设备合并到一起 */
//	if ((addr & 0xF0000000) == 0x00000000)		// 片内Flash
//	{
		ret = Iap.Read(addr, (uint32*)data, len_bytes);
//	}
//	else if ((addr & 0xF0000000) == 0x10000000)	// 片外Flash
//	{
//	}
//	else if ((addr & 0xF0000000) == 0x20000000)	// 片外EEPROM
//	{
//	}
	
	return ret;
}


/*
****************************************************************************************************
**
**                                   MemWriteWithCRC()
**
**
**	功能描述: 数据保存到指定的内存，加了校验和。
**
**	参  数  : 
**
**	返回值  : TRUE / FALSE
**
****************************************************************************************************
*/
uint32 MemWriteWithCRC(uint32 ts, uint32 bankIdx, MEM_DRIVER_HANDLE *mh)
{
	uint32 addr  = mh->Address;
	uint8  *data = mh->pData;
	uint32 len   = mh->Length;
	
	uint8  buf[SIZE_OF_BLK];
	uint32 i;
	uint32 xLen;
	uint8  *pBuf;
	uint32 ret = 0;
	uint32 crc = 0;
	uint32 flashAddr;
	
	
	
//	if (len % 4)	// 长度必须是4的整数倍，保证4字节对齐
//	{
//		return 0;
//	}
	
	flashAddr = addr + bankIdx * mh->SizeOfBank;
	
	if (len < (SIZE_OF_BLK - 8))
	{
		xLen = len;
	}
	else
	{
		xLen = (SIZE_OF_BLK - 8);
	}
	
	// 第1个Block保存
	{
		memcpy(buf, &ts, sizeof(uint32)); 	// 新的ts写入
		pBuf = buf + 4;
		for (i=4; i<(xLen+4); i++)
		{
			*pBuf++ = *data++;
		}
		for (; i<(SIZE_OF_BLK-4); i++)
		{
			*pBuf++ = 0xFF;
		}
		
		crc = CRC_CALC((uint32*)buf, (SIZE_OF_BLK-4)/sizeof(uint32));
		memcpy(pBuf, &crc, sizeof(uint32));
		
		ret = FlashWrite(flashAddr, buf, SIZE_OF_BLK);		
		if (!ret)
		{
			return false;
		}
		flashAddr += SIZE_OF_BLK;
		len -= xLen;
	}
	
	
	// 第2到n-1个Block保存
	while (len > (SIZE_OF_BLK-4))
	{
		pBuf = buf;
		for (i=0; i<(SIZE_OF_BLK-4); i++)
		{		
			*pBuf++ = *data++;
		}
		
		crc = CRC_CALC((uint32*)buf, (SIZE_OF_BLK-4)/sizeof(uint32));
		memcpy(pBuf, &crc, sizeof(uint32));
		
		ret = FlashWrite(flashAddr, buf, SIZE_OF_BLK);		
		if (!ret)
		{
			return false;
		}
		flashAddr += SIZE_OF_BLK;
		len -= (SIZE_OF_BLK-4);
	}
	
	// 第n个Block保存
	if (len)
	{
		pBuf = buf;
		for (i=0; i<len; i++)
		{
			*pBuf++ = *data++;
		}
		for (; i<(SIZE_OF_BLK-4); i++)
		{
			*pBuf++ = 0xFF;
		}
		
		crc = CRC_CALC((uint32*)buf, (SIZE_OF_BLK-4)/sizeof(uint32));
		memcpy(pBuf, &crc, sizeof(uint32));
		
		ret = FlashWrite(flashAddr, buf, SIZE_OF_BLK);		
		if (!ret)
		{
			return false;
		}
	}
	
	return true;
}


/*
****************************************************************************************************
**
**                                        MemWriteInit()
**
**
**	功能描述: 数据保存到指定的内存。这是初始化，由于多个Bank对应不同的时间戳，需要做一次初始化。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void MemWriteInit(MEM_DRIVER_HANDLE *mh)
{
	uint32 i;
	uint32 ret;
	
										// 把所有Bank都存一次，初始化ts。当ts达到最大时，
	for (i=0; i<mh->NumOfBank; i++)		// 也可以清除更大的ts 
	{
		ret = MemWriteWithCRC(i, i, mh);
		
		if (!ret)
		{
			while (1);			// 初始化都不成功，选择死循环，告知用户
		}
	}
}


/*
****************************************************************************************************
**
**                                        MemWrite()
**
**
**	功能描述: 数据保存到指定的内存。目前只允许整个数据全部保存。像EEPROM随意偏移位置写，不支持。
**
**	参  数  : 
**
**	返回值  : TRUE / FALSE
**
****************************************************************************************************
*/
uint32 MemWrite(MEM_DRIVER_HANDLE *mh)
{
	uint32 addr = mh->Address;
	
	uint32 i;
	uint32 ret = 0;
	uint32 errCnt = 0;
	uint32 ts[MAX_BANKS];
	uint32 flashAddr;
	uint32 tsMax, bankIdx;
	
	
	
	if (mh->NumOfBank > MAX_BANKS || 
		mh->SizeOfData > (mh->SizeOfBank - (mh->SizeOfBank/SIZE_OF_BLK) * 4 - 4))
	{
		return 0;
	}
	
MEM_REWRITE:
	if (errCnt > (mh->NumOfBank-1))
	{
		return 0;
	}
	
	for (i=0; i<mh->NumOfBank; i++)
	{
		flashAddr = addr + i * mh->SizeOfBank;
		FlashRead(flashAddr, (uint8*)&ts[i], 4);
	}
	
	bankIdx = 0;
	tsMax   = 0;
	for (i=0; i<mh->NumOfBank; i++)		// 从第1个Bank开始查找，找最大的ts
	{
		if (ts[i] == 0xFFFFFFFF)	// 遇到ts = 0xFFFFFFFF，就是未存储过的Bank，则可以存储
		{
			bankIdx = i;
			break;
		}
		else if (tsMax > ts[i])		// 不是0xFFFFFFFF，那么遇到比tsMax小的，就是可以存储的Bank
		{
			bankIdx = i;
			break;
		}
		else
		{
			tsMax = ts[i];
		}
	}
								// 时间戳最大值是0xFFFF0000，方便与0xFFFFFFFF做区别
	if (++tsMax == 0xFFFF0000)	// 新写入的时间戳找最迟的加1
	{
		MemWriteInit(mh);
		return 1;
	}
	
	ret = MemWriteWithCRC(tsMax, bankIdx, mh);
	if (!ret)
	{
		errCnt++;	// 如果写入发生错误（可能扇区部分损坏），可以尝试存储到下一个Bank
		goto MEM_REWRITE;
	}
	
	return ret;
}


/*
****************************************************************************************************
**
**                                        MemRead()
**
**
**	功能描述: 从指定的内存地址，读出数据
**
**	参  数  : doubleBackup   是否支持双备份存储。1 是，0 否
**
**	返回值  : TRUE / FALSE
**
****************************************************************************************************
*/
uint32 MemRead(MEM_DRIVER_HANDLE *mh, uint32 doubleBackup)
{
	uint32 addr   = mh->Address;
	uint32 offset = mh->Offset;
	uint8  *data  = mh->pData;
	uint32 len    = mh->Length;
	
	uint8  buf[SIZE_OF_BLK];
	uint32 i, tmp;
	uint32 xLen;
	uint8  *pBuf;
	uint32 crc = 0;
	uint32 crcMem;
	uint32 ts[MAX_BANKS];
	uint32 flashAddr;
	uint32 tsMax, bankIdx;
	uint32 reReadCnt;
	uint32 bankIdx2nd;			// 备份Bank的序号
	uint32 bankIdx2ndValid;		// 备份Bank是否有效
	
	
	
	if (mh->NumOfBank > MAX_BANKS || 
	   (mh->Offset + mh->Length) > mh->SizeOfData)
	{
		return false;
	}

	for (i=0; i<mh->NumOfBank; i++)
	{
		flashAddr = addr + i * mh->SizeOfBank;
		FlashRead(flashAddr, (uint8*)&ts[i], 4);
	}
		
	reReadCnt       = 0;
	bankIdx         = 0;
	bankIdx2nd      = 0;
	bankIdx2ndValid = 0;	// 默认备份Bank是无效的
	
	tsMax   = 0;	// 读取，找最迟时间戳。tsMax只是帮助查找，bankIdx是查找想要的值
	for (i=0; i<mh->NumOfBank; i++)
	{
		if (ts[i] == 0xFFFFFFFF)
		{
			if (i == 0)
			{
				return false;	// 这种情况是发性在一次都没有存储过
			}
			break;
		}
		else if (tsMax > ts[i])
		{
			break;
		}
		else
		{
			
			bankIdx2nd = bankIdx;	// 先取出来，再更新tsMax, bankIdx
			bankIdx2ndValid = 1;
			
			tsMax   = ts[i];
			bankIdx = i;
		}
	}
	
MEM_REREAD:	
	// 如果当前Bank数据读取有误，可以从上一个Bank读取。相当于双备份的功能。
	if (doubleBackup && mh->NumOfBank > 1)
	{
		if (reReadCnt == 1)
		{
			if (bankIdx2ndValid)
				bankIdx = bankIdx2nd;
			else
				return false;
		}	
		else if (reReadCnt > 1)	// 如果从备份的Bank中读取还是错误，则结束读取，返回错误。
		{
			return false;
		}
	}
	else
	{
		if (reReadCnt > 0)
		{
			return false;
		}
	}
	reReadCnt++;
	
	
	flashAddr = addr + bankIdx * mh->SizeOfBank;	// 当前Bank的Flash地址
	if (offset < (SIZE_OF_BLK - 8))			// 偏移地址从第1个Block算起
	{
		offset += 4;						// 到这里，offset已经是block中的偏移了
	}
	else
	{
		offset -= (SIZE_OF_BLK - 8);
		flashAddr += (offset / (SIZE_OF_BLK-4) + 1) * SIZE_OF_BLK;
		
		offset = offset % (SIZE_OF_BLK-4);	// 到这里，offset已经是block中的偏移了
	}	
	
	// 读取第1个Block
	{
		FlashRead(flashAddr, buf, SIZE_OF_BLK);
	
		crc = CRC_CALC((uint32*)buf, (SIZE_OF_BLK-4)/sizeof(uint32));
		memcpy(&crcMem, buf + SIZE_OF_BLK-4, sizeof(uint32));
		
		if (crc != crcMem)
		{
			goto MEM_REREAD;
		}
	
		pBuf = buf + offset;
	
		tmp = SIZE_OF_BLK-4 - offset;
		if (len <= tmp) xLen = len;
		else            xLen = tmp;
		
		for (i=0; i<xLen; i++)
		{
			*data++ = *pBuf++;
		}
		len -= xLen;
		flashAddr += SIZE_OF_BLK;
	}	
	
	// 读取第2到n-1个Block
	while (len > (SIZE_OF_BLK-4))
	{
		FlashRead(flashAddr, buf, SIZE_OF_BLK);
	
		crc = CRC_CALC((uint32*)buf, (SIZE_OF_BLK-4)/sizeof(uint32));
		memcpy(&crcMem, buf + SIZE_OF_BLK-4, sizeof(uint32));
		
		if (crc != crcMem)
		{
			goto MEM_REREAD;
		}
	
		pBuf = buf;
		for (i=0; i<(SIZE_OF_BLK-4); i++)
		{
			*data++ = *pBuf++;
		}
		len -= (SIZE_OF_BLK-4);
		flashAddr += SIZE_OF_BLK;
	}
	
	// 读取第n个Block
	if (len)
	{
		FlashRead(flashAddr, buf, SIZE_OF_BLK);
	
		crc = CRC_CALC((uint32*)buf, (SIZE_OF_BLK-4)/sizeof(uint32));
		memcpy(&crcMem, buf + SIZE_OF_BLK-4, sizeof(uint32));
		
		if (crc != crcMem)
		{
			goto MEM_REREAD;
		}
	
		pBuf = buf;
		for (i=0; i<len; i++)
		{
			*data++ = *pBuf++;
		}
	}
	
	return true;
}




