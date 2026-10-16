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
**	文件名	: Mem.Driver.h
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
#ifndef _MEM_DRIVER_H
#define _MEM_DRIVER_H
#include "UserRetype.h"



#define MAX_BANKS			(8)		// 数据存储允许的最大Bank数目，用于代码判断

									// 数据安全性。对Bank进行分块，每块有一个CRC32的检验和
#define SIZE_OF_BLK  		(32)	// 块大小N，(N-4)个有效数据，4个字节是检验和


typedef struct
{
	uint32 Offset;		// 偏移地址，用户保存数据的偏移地址。目前只允许整个数据全部保存。像EEPROM随意偏移位置写，不支持。
	uint8  *pData;		// 本次操作（读/写）的数据地址
	uint32 Length;		// 本次操作（读/写）的数据长度
	
	uint32 Address;		// 起始地址，Flash的真实地址。（第1个Bank的存储芯片的物理地址）
	
	uint32 SizeOfData;	/* 用户要保存的数据总大小，它小于Bank的大小，即只存部分数据，多余的空间空着。
						   当然，实际上由于Bank除了用户保存的数据，还有时间戳和校验和，所以用户能
						   保存的数据要比Bank大小还要更小一些。*/
	
	uint32 SizeOfBank;	/* 一个Bank所占的字节大小，用户要存储的数据大小（SizeOfData）小于SizeOfBank
						*/
	
	uint32 NumOfBank;	/* 需要用多少个Bank来存储数据。数目越大，存储寿命越长。所以用户数据存储的物
						   理空间大小是 (Banks * SizeOfBank)  */
} MEM_DRIVER_HANDLE;

#endif




