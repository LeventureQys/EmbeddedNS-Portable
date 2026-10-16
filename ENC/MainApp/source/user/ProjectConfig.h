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
**	文件名	: ProjectConfig.h
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: 定义全局宏，类似Keil工程中的全局宏，但这是以代码的形式出现。在GIT比较差异时，更好用。
**
****************************************************************************************************
*/
#ifndef	_PROJECT_CFG_H
#define	_PROJECT_CFG_H



#define DBG_NO_DSP			(0)		// 不调试DSP
#define DBG_TX_PC_EN		(1)		// 是否主动发数据给上位机 


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

#endif




