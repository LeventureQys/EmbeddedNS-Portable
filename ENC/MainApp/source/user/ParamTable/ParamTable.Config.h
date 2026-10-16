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
**	文件名	: ParamTable.Config.h
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: 配置参数表
**
****************************************************************************************************
*/
#ifndef _CONFIG_H
#define _CONFIG_H
#include "UserRetype.h"



/* 1) 研发阶段可以用Build，也就是每次新编译后，Build都会改变。也意味着系统都要
	  执行一次恢复出厂设置
	  
   2) 量产后，不能用Build，需要手动改，视情况改。需要时，就加1即可
*/

#define NEW_REINIT_FLAG		(0)	// Niko


// 系统配置数据结构，协议中是Little-Endian格式
typedef struct
{
	uint16 Heartbeat;			// 心跳参数，只读，写无效。
	uint16 DeviceAddr;			// 设备地址，用于多台设备联机
	uint16 PresetNum;			// 系统预设总数
	uint16 PstStartupIdx;		// 系统启动时，加载的预设
	uint16 PstSaveIdx;			// 保存预设
	uint16 PstLoadIdx;			// 加载预设
	uint16 PstDeleIdx;			// 删除预设
	uint16 CommErrAck;			// 通讯错误响应
	uint16 DFU;					// 升级标志
	uint16 SysReInit;			// 升级或者烧写程序时，是否恢复出厂设置
	uint16 ResFactory;			// 写 1 恢复出厂设置，这个是给上位机使用的
	uint16 SyncFlag;			// 上位机批量数据同步标志
	uint16 SysErrLog[2];		// 系统错误日记
} CONFIG_PARAM;

#endif




