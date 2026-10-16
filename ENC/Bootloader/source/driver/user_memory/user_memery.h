#ifndef _USER_MEMERY_H
#define _USER_MEMERY_H


#include "Main.h"
#include "a0_hal_spi_flash.h"

extern UINT8 write_flash_cnt;

void ReadFlash(void);
void WriteFlash(void);
void  write_flash_pro(uint32_t TimeDelay);
void *mmset(void *src, int ch, size_t len);/*移动函数*/
void *mmcpy(void *src,  void *dst, size_t len);/*复制函数*/
void WritrFlashTest(uint32_t TimeDelay);
#endif
