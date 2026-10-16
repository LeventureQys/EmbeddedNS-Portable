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
**  日　期  : 2022年12月1日
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

/* Spi.c 里的 SPI2 部分：GPIO0.28(AF4=SPI2_MOSI) 上的 4 颗 WS2812 走硬件 SPI。
   位时序由 SPI2 外设产生，CPU 只负责把数据塞进 FIFO，所以不受主循环/中断里
   长代码造成的取指、总线停顿影响（软件位翻转就会被这种停顿拉长某一位而误码）。
   Spi2Init() 由 Ui.Led.c 的 UiLedInit() 调用；Spi2LedTx() 由 _RgbStripFlush() 调用。 */
void Spi2Init(void);
void Spi2LedTx(uint8 *data);

#endif




