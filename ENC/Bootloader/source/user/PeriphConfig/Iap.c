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
**	文件名	: Iap.c
**
**	程序员	: 
**
**	日　期  : 2023年1月12日
**
**	文件描述: MCU片内Flash读写应用
**
****************************************************************************************************
*/
#include "Iap.h"
#include "a0.h"
#include "a0_hal_spi_flash.h"



/* 函数声明 */
uint32 IapPageErase(uint32 dst);
uint32 IapWrite(uint32 dst, uint32* src, uint32 nBytes);
uint32 IapRead (uint32 dst, uint32* src, uint32 nBytes);


/* 变量定义和声明 */
IAP_HANDLE Iap = 
{
	.PageErase = IapPageErase,
	.Read      = IapRead,
	.Write     = IapWrite,
};


/*
****************************************************************************************************
**
**                                         IapPageErase()
**
**
**	功能描述: 擦除Flash扇区
**
**	参  数  : dst: 	Flash的目标地址
**
**	返回值  : 
**
****************************************************************************************************
*/
uint32 IapPageErase(uint32 dst)
{
	uint32 argv[3];
	
	
	argv[0] = dst;            					// 擦除的地址
    argv[1] = MCU_FLASH_PAGE_BYTES_SIZE;		// 擦除的长度
    argv[2] = 1;             					// 0 for page(256Byte), 1 for sector(4KByte), 2 for block 32KByte, 3 for block 64KByte, 4 for chip
    
	if (hal_flash_erase(argv) != 0)
    {
		return false;
    }
	return true;
}


/*
****************************************************************************************************
**
**                                         IapRead()
**
**
**	功能描述: 读Flash
**
**	参  数  : dst 		写出数据的Flash地址
**            src 		要写出的数据的地首指针
**            nBytes 	写入的长度(以字节为单位)
**
**	返回值  : 正确返回 TRUE，错误返回 FALSE
**
**  备  注  : 
**
****************************************************************************************************
*/
uint32 IapRead(uint32 dst, uint32 *src, uint32 nBytes)
{
	uint32 i;
	uint32 ret = true;


	for (i=0; i<nBytes; i++)
	{
		*((uint8*)src+i) = *((uint8*)dst+i);
	}

    return ret;
}


/*
****************************************************************************************************
**
**                                         IapWrite()
**
**
**	功能描述: 写Flash
**
**	参  数  : dst 		写入Flash的地址
**            src 		要写入的数据的地首指针
**            nBytes 	写入的长度(以字节为单位)
**
**	返回值  : 正确返回 TRUE，错误返回 FALSE
**
**  备  注  : 
**
****************************************************************************************************
*/
uint32 IapPageWrCheck(uint32 dst, uint32* src, uint32 nBytes)
{
	uint32 i,j;
	uint32 nWords;
	uint32 erasePageEnable = 0;
	uint32 ret;


	if (nBytes > MCU_FLASH_PAGE_BYTES_SIZE)
	{
		return false;
	}

	if (dst % MCU_FLASH_PAGE_BYTES_SIZE == 0)
	{
		erasePageEnable = 1;
	}
	
	nWords = nBytes%4 ? nBytes/4+1 : nBytes/4;	
	for (j=0; j<3; j++)
	{
		ret = true;

		if (erasePageEnable)
		{
			IapPageErase(dst);
		}
		
		{
			uint32 argv[3];							/* 开始连续向 FLASH 写入数据 */
			argv[0] = dst;
			argv[1] = nBytes;
			argv[2] = (uint32)src;
			hal_flash_write(argv);				
		}
		
		for (i=0; i<nWords; i++)					/* Check the corectness of written data */
		{
			if(*((uint32*)dst+i) != *(src+i))
			{
				ret = false;
				break;
			}
		}

		if (ret == true)
		{
			break;
		}
	}

    return ret;
}


uint32 IapWrite(uint32 dst, uint32 *src, uint32 nBytes)
{
	uint32 i;
	uint32 l = 0;
	uint32 r = 0;
	uint32 ret = false;


	l = nBytes / MCU_FLASH_PAGE_BYTES_SIZE;
	r = nBytes % MCU_FLASH_PAGE_BYTES_SIZE;

	for (i=0; i<l; i++)
	{
		ret = IapPageWrCheck(dst + i*MCU_FLASH_PAGE_BYTES_SIZE,
							 src + i*MCU_FLASH_PAGE_WORDS_SIZE,
							 MCU_FLASH_PAGE_BYTES_SIZE);
		if (!ret)
		{
			return ret;
		}
	}

	if (r > 0)
	{
		ret = IapPageWrCheck(dst + i*MCU_FLASH_PAGE_BYTES_SIZE,
							 src + i*MCU_FLASH_PAGE_WORDS_SIZE,
							 r);
		if (!ret)
		{
			return ret;
		}
	}

	return ret;
}




