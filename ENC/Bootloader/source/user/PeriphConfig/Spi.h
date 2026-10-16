/*
*****************************************Copyright (c)**********************************************
**
**
**
**
**
**
**
**	文件名	: Spi.c
**	
**	程序员	: 
**
**  日　期  : 2023年1月12日
**
**	文件描述: SPI总线来控制RGB灯
**
****************************************************************************************************
*/
#ifndef _SPI_H
#define _SPI_H
#include "UserRetype.h"



typedef struct
{
	void (*Init)(void);
	void (*LedReflesh)(uint8 *data);

} SPIx_HANDLE;

extern SPIx_HANDLE Spi;

#endif




