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
**	文件名	: Fw.Info.h
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: 固件信息
**
****************************************************************************************************
*/
#include "main.h"



#define MODEL_CODE          "P0JY04"
#define MODEL_NAME          "PD100UGen2"

#define MODEL_CODE_PARAM1   (MODEL_CODE[0] | (MODEL_CODE[1] << 8))
#define MODEL_CODE_PARAM2   (MODEL_CODE[2] | (MODEL_CODE[3] << 8))
#define MODEL_CODE_PARAM3   (MODEL_CODE[4] | (MODEL_CODE[5] << 8))

#define MODEL_NAME_PARAM1   (MODEL_NAME[0] | (MODEL_NAME[1] << 8))
#define MODEL_NAME_PARAM2   (MODEL_NAME[2] | (MODEL_NAME[3] << 8))
#define MODEL_NAME_PARAM3   (MODEL_NAME[4] | (MODEL_NAME[5] << 8))
#define MODEL_NAME_PARAM4   (MODEL_NAME[6] | (MODEL_NAME[7] << 8))
#define MODEL_NAME_PARAM5   (MODEL_NAME[8] | (MODEL_NAME[9] << 8))
#define MODEL_NAME_PARAM6   (0x0000)
#define MODEL_NAME_PARAM7   (0x0000)
#define MODEL_NAME_PARAM8   (0x0000)


/* 固件信息，主要存放
	Model Code
	Model Name
	Device SN
   
   固件信息，DFU工程也可以访问。
*/
#ifdef __DEBUG		// 只有调试才开启
const uint8 FwInfoArray[88]__attribute__((at(FLASH_FW_INFO_ADDR))) = 
{
	(MODEL_CODE_PARAM1 & 0xFF), ((MODEL_CODE_PARAM1 >> 8) & 0xFF),
	(MODEL_CODE_PARAM2 & 0xFF), ((MODEL_CODE_PARAM2 >> 8) & 0xFF),
	(MODEL_CODE_PARAM3 & 0xFF), ((MODEL_CODE_PARAM3 >> 8) & 0xFF),
	0xFF, 0xFF,							// 补两字节，用于4字节对齐
	
	(MODEL_NAME_PARAM1 & 0xFF), ((MODEL_NAME_PARAM1 >> 8) & 0xFF),
	(MODEL_NAME_PARAM2 & 0xFF), ((MODEL_NAME_PARAM2 >> 8) & 0xFF),
	(MODEL_NAME_PARAM3 & 0xFF), ((MODEL_NAME_PARAM3 >> 8) & 0xFF),
	(MODEL_NAME_PARAM4 & 0xFF), ((MODEL_NAME_PARAM4 >> 8) & 0xFF),
	(MODEL_NAME_PARAM5 & 0xFF), ((MODEL_NAME_PARAM5 >> 8) & 0xFF),
	(MODEL_NAME_PARAM6 & 0xFF), ((MODEL_NAME_PARAM6 >> 8) & 0xFF),
	(MODEL_NAME_PARAM7 & 0xFF), ((MODEL_NAME_PARAM7 >> 8) & 0xFF),
	(MODEL_NAME_PARAM8 & 0xFF), ((MODEL_NAME_PARAM8 >> 8) & 0xFF),
	
	0xFF, 0xFF, 0xFF, 0xFF, 			// Flash地址为：0x08022000 + 24 = 0x08022018
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 
};
#endif




