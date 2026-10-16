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
**	文件名	: Iap.h
**
**	程序员	: 
**
**	日　期  : 2023年1月12日
**
**	文件描述: MCU片内Flash读写应用
**
****************************************************************************************************
*/
#ifndef _IAP_H
#define _IAP_H
#include "UserRetype.h"



/* GD32F35x扇区大小分布 */
#define MCU_FLASH_PAGE_BYTES_SIZE		(4096)
#define MCU_FLASH_PAGE_WORDS_SIZE		(MCU_FLASH_PAGE_BYTES_SIZE/4)


typedef struct
{
	uint32 (*PageErase)(uint32 dst);
	
	uint32 (*Read )(uint32 dst, uint32 *src, uint32 nBytes);
	uint32 (*Write)(uint32 dst, uint32 *src, uint32 nBytes);
} IAP_HANDLE;

extern IAP_HANDLE Iap;

#endif




