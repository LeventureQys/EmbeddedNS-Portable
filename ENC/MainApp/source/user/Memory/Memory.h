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
**	文件名	: Memory.h
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: 系统存储管理。
**
****************************************************************************************************
*/
#ifndef _MEMERY_H
#define _MEMERY_H
#include "UserRetype.h"
#include "Mem.Driver.h"
#include "Mem.SaveLoad.h"
#include "SctConfig.h"



/* 注意：以下的一些宏的值由py脚本生成。
*/

/* 定义Flash一个扇区的大小 */
#define FLASH_SECTOR_SIZE               (MCU_FLASH_PAGE_BYTES_SIZE)


/* 为了延长Flash寿命，采用多倍存储空间来完成 */
#define DEV_INFO_BANK_NUM               (8)       // DevInfo 8 Banks
#define CFG_DATA_BANK_NUM               (8)       // CfgData 8 Banks
#define PRESET_BANK_NUM                 (8)       // Preset  8 Banks


/* 定义每个参数表存储Bank的空间大小 */
#define DEV_INFO_BANK_SIZE              (256)     // DevInfo 一个Bank的空间大小
#define CFG_DATA_BANK_SIZE              (256)     // CfgData 一个Bank的空间大小
#define PRESET_BANK_SIZE                (256)     // Preset  一个Bank的空间大小。预设存储空间一般用Flash整数倍扇区大小来定义


/* 定义每个参数表存储空间总大小 */
#define DEV_INFO_ALL_BANKS_SIZE         (2048)    // DEV_INFO_BANK_NUM * DEV_INFO_BANK_SIZE
#define CFG_DATA_ALL_BANKS_SIZE         (2048)    // CFG_DATA_BANK_NUM * CFG_DATA_BANK_SIZE
#define PRESET_ALL_BANKS_SIZE           (4096)    // PRESET_BANK_NUM   * PRESET_BANK_SIZE


/* 系统应用中，所有数据存储空间总大小 */
#define USER_DATA_STORE_SIZE            (4096)    // 4 KBytes
#if USER_DATA_STORE_SIZE > FLASH_APP_DS_SIZE
err("实际存储空间超过了Flash分配的空间")
#endif


/* 系统中能存储的预设的最大数目 */
#define NUM_OF_PRESET                   (1)

/* 预设名字符串的长度（参数数目，不是字节数） */
#define PRESET_NAME_STRING_LEN          (32)


/* 内存各数据块的存储地址，大于0x10000000的是外部SPI Flash。小于则是MCU内部Flash地址 */
#define MEM_STORE_ADDR_MASK             (0x0FFFFFFF)

#define MEM_STORE_START_ADDR            (0x00028000)
#define MEM_STORE_END_ADDR              (0x00029FFF)

#define MEM_STORE_DEV_INFO_ADDR         (0x00028000)
#define MEM_STORE_CONFIG_DATA_ADDR      (0x00028800)

#define MEM_STORE_PRESET_1_ADDR         (0x00029000)

#define MEM_STORE_PRESET_ADDR(idx)      (MEM_STORE_PRESET_1_ADDR + idx * (PRESET_BANK_NUM * PRESET_BANK_SIZE))



/* 参数表中，各个内存块的起始地址 */
#define MEM_DEV_INFO_BASE				(0x0000)
#define MEM_CONFIG_DATA_BASE			(0x1000)
#define MEM_PRESET_DATA_BASE			(0x2000)
#define MEM_PRESET1_DATA_BASE			(0x3000)
//#define MEM_PRESET2_DATA_BASE			(0x4000)
//#define MEM_PRESET3_DATA_BASE			(0x5000)
//#define MEM_PRESET4_DATA_BASE			(0x6000)
//#define MEM_PRESET5_DATA_BASE			(0x7000)
//#define MEM_PRESET6_DATA_BASE			(0x8000)
//#define MEM_PRESET7_DATA_BASE			(0x9000)
//#define MEM_PRESET8_DATA_BASE			(0xA000)
	
#define MEM_ADDR_TYPE_MASK				(0xF000)
#define MEM_OFFSET_ADDR_MASK			(0x0FFF)


/* 参数类型（归属） */
#define IS_DEV_INFO		(1)
#define IS_CFG_DATA		(2)
#define IS_CURR_PRESET	(3)
#define IS_USER_PRESET	(4)



typedef struct 
{
	void (*Init)(void);
	void (*Task)(void);
		
	struct
	{
		uint32 (*Rand)(uint16 *addr, uint16 *data);
	} ParamWrite;
	
	struct
	{
		uint32 (*Rand)(uint16 *addr, uint16 *data);
		void   (*Sequ)(uint16 *addr, uint8 *u8buf, uint32 len);
	} ParamRead;
	
	struct
	{
		uint32 (*DevInfoParam)(void);
		uint32 (*ConfigParam )(void);
		uint32 (*PresetParam )(uint32 presetIdx);
	} Save;
	
	struct
	{
		uint32 (*DevInfoParam)(void);
		uint32 (*ConfigParam )(void);
		uint32 (*PresetParam )(uint32 presetIdx);
	} Load;
	
	uint32 (*LoadPreset)(uint32 presetIdx);
	
	void (*ResFactory)(void);
	void (*CfgParamAutoSavePreview)(void);
	void (*PresetParamAutoSavePreview)(void);
    void (*PresetParamAutoSaveCancel)(void);
	
} MEM_HANDLE;

extern MEM_HANDLE Mem;

#endif




