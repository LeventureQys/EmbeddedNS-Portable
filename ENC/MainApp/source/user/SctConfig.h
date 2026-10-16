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
**	文件名	: SctConfig.h
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: 对分散加载文件(*.sct)进行宏定义。
**
****************************************************************************************************
*/
#ifndef _SCT_CONFIG_H
#define _SCT_CONFIG_H


												
/* 注意：
   1) 需与Bootload, MainApp的.sct文件一致

*/
#define FLASH_BL_ADDR           (0x00000000)		// 引导程序分区

#define FLASH_DFU_SB_ADDR       (0x00038000)		// DFU状态字节存放区

#define FLASH_FW_INFO_ADDR      (0x00039000)		// 固件信息存放分区
#define FLASH_FW_INFO_SIZE      (1 * 256   )


#define FLASH_APP_ADDR          (0x00008000)		// 应用程序分区


#define FLASH_APP_DS_ADDR       (0x00028000)		// 数据存储区(Data Store)
#define FLASH_APP_DS_SIZE       (8 * 1024  )

#endif




