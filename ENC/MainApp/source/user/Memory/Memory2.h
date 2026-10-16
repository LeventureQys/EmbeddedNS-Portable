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
**	文件名	: Memory2.h
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: Memory模块内部使用头文件
**
****************************************************************************************************
*/
#ifndef _MEMERY2_H
#define _MEMERY2_H
#include "UserRetype.h"



/* Memory */
//uint32 MemParamRandWrite(uint16 *addr, uint16 *data);
//uint32 MemParamRandRead (uint16 *addr, uint16 *data);
//void   MemParamSequRead (uint16 *addr, uint8 *u8buf, uint32 len);


/* Mem.SaveLoad */
void   MemParamsInit(void);
void   MemParamsInit4Dfu(void);

uint32 MemDeviceInfoWrite(void);
uint32 MemDeviceInfoRead (void);
uint32 MemReadModelCode  (uint16 *mc);

uint32 MemConfigDataWrite(void);
uint32 MemConfigDataRead (void);

uint32 MemPresetDataWrite    (uint32 presetIdx);
uint32 MemPresetDataRead     (uint32 presetIdx);
uint32 MemPresetDataReadBytes(uint32 presetIdx, uint32 addrOffset, uint8 *buf, uint32 regNum);
uint32 MemPresetLoad         (uint32 idx);


/* Mem.Driver */
uint32 MemWrite(MEM_DRIVER_HANDLE *mh);
uint32 MemRead (MEM_DRIVER_HANDLE *mh, uint32 doubleBackup);


#endif




