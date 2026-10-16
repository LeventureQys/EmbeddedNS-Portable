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
**	文件名　: ParamTable.h
**
**	程序员　: 
** 
**	日　期  : 2022年7月22日
** 
**	文件描述: 系统参数表管理。
** 
***************************************************************************************************
*/
#ifndef _PARAM_TABLE_H
#define _PARAM_TABLE_H
#include "UserRetype.h"
#include "ParamTable.DevInfo.h"
#include "ParamTable.Config.h"
#include "ParamTable.Preset.h"



/* Python 自动生成的起始行 */
#define DEV_INFO_TBL_SIZE             (62)
#define CONFIG_TBL_SIZE               (14)
//#define PRESET_TBL_SIZE               (44)
//#define PRESET_TBL_SIZE               (45) // Steve test 增加一栏声音增强
#define PRESET_TBL_SIZE               (93) // Niko Add 71Game


/* 设备信息 - 参数地址定义 */
#define PARAM_MODEL_CODE              (0x0000)
#define PARAM_MODEL_NAME              (0x0003)
#define PARAM_FW_REV_X                (0x000B)
#define PARAM_FW_REV_Y                (0x000C)
#define PARAM_FW_REV_Z                (0x000D)
#define PARAM_FW_REV_BUILD            (0x000E)
#define PARAM_FW_REV_TYPE             (0x000F)
#define PARAM_DSP1_FW_REV_X           (0x0010)
#define PARAM_DSP2_FW_REV_X           (0x0013)
#define PARAM_USB_VID                 (0x0016)
#define PARAM_USB_PID                 (0x0017)
#define PARAM_DATE_YEAR               (0x0018)
#define PARAM_DATE_MONTH              (0x0019)
#define PARAM_DATE_DAY                (0x001A)
#define PARAM_TIME_HOUR               (0x001B)
#define PARAM_TIME_MINUTE             (0x001C)
#define PARAM_TIME_SECOND             (0x001D)
#define PARAM_SN_START                (0x001E)
#define PARAM_SN_END                  (0x003D)


/* 配置数据 - 参数地址定义 */
#define PARAM_CONFIG_START            (0x1000)
#define PARAM_CONFIG_END              (0x100D)

#define PARAM_HEART_BEAT              (0x1000)
#define PARAM_DEVICE_ADDR             (0x1001)
#define PARAM_PRESET_NUMBER           (0x1002)
#define PARAM_STARTUP_LOAD_NO         (0x1003)
#define PARAM_SAVE                    (0x1004)
#define PARAM_LOAD                    (0x1005)
#define PARAM_DELETE                  (0x1006)
#define PARAM_COMM_ERR_ACK            (0x1007)
#define PARAM_DFU                     (0x1008)
#define PARAM_SYSTEM_REINIT           (0x1009)
#define PARAM_REC_FACTORY             (0x100A)
#define PARAM_SYNC_FLAG               (0x100B)
#define PARAM_SYS_ERR                 (0x100C)
#define PARAM_SYS_ERR2                (0x100D)



/* 预设 - 参数地址定义，每一类型的DSP功能的首和尾参数地址 */
#define PARAM_PRESET_START            (0x2000)
//#define PARAM_PRESET_END              (0x2038)
//#define PARAM_PRESET_END              (0x202B)  // Liang
//#define PARAM_PRESET_END              (0x202C)	// Steve
#define PARAM_PRESET_END              (0x205C)	// Niko

#define PARAM_CHECKSUM                (0x2000)

#define PARAM_PRESET_VALID_FLAG       (0x2001)

#define PARAM_PRESET_NAME_START       (0x2002)
#define PARAM_PRESET_NAME_END         (0x2021)
          
#define PARAM_MIC_MUTE                (0x2022)
#define PARAM_MIC_GAIN                (0x2023)
#define PARAM_NS_EN                   (0x2024)  // Liang
#define PARAM_NS_THRESHOLD            (0x2025)

#define PARAM_MIC_MODE                (0x2026)  //Garry   ~~~~~~~~ 参量均衡器(场景) ~~~~~~~~
#define PARAM_Band1_ENABLE            (0x2027)  //        ~~~~~~~~     Band1      ~~~~~~~~ 
#define PARAM_Band1_TYPE              (0x2028)
#define PARAM_Band1_FREQ              (0x2029)
#define PARAM_Band1_GAIN              (0x202A)
#define PARAM_Band1_Q                 (0x202B)

#define PARAM_Band2_ENABLE            (0x202C)  //         ~~~~~~~~     Band2      ~~~~~~~~ 
#define PARAM_Band2_TYPE              (0x202D)
#define PARAM_Band2_FREQ              (0x202E)
#define PARAM_Band2_GAIN              (0x202F)
#define PARAM_Band2_Q                 (0x2030)

#define PARAM_Band3_ENABLE            (0x2031) //          ~~~~~~~~     Band3      ~~~~~~~~ 
#define PARAM_Band3_TYPE              (0x2032)
#define PARAM_Band3_FREQ              (0x2033)
#define PARAM_Band3_GAIN              (0x2034)
#define PARAM_Band3_Q                 (0x2035)

#define PARAM_Band4_ENABLE            (0x2036) //          ~~~~~~~~     Band4      ~~~~~~~~ 
#define PARAM_Band4_TYPE              (0x2037)
#define PARAM_Band4_FREQ              (0x2038)
#define PARAM_Band4_GAIN              (0x2039)
#define PARAM_Band4_Q                 (0x203A)

#define PARAM_Band5_ENABLE            (0x203B)  //          ~~~~~~~~     Band5      ~~~~~~~~ 
#define PARAM_Band5_TYPE              (0x203C)
#define PARAM_Band5_FREQ              (0x203D)
#define PARAM_Band5_GAIN              (0x203E)
#define PARAM_Band5_Q                 (0x203F)

#define PARAM_Band6_ENABLE            (0x2040)  //          ~~~~~~~~     Band6      ~~~~~~~~ 
#define PARAM_Band6_TYPE              (0x2041)
#define PARAM_Band6_FREQ              (0x2042)
#define PARAM_Band6_GAIN              (0x2043)
#define PARAM_Band6_Q                 (0x2044)

#define PARAM_Band7_ENABLE            (0x2045)  //          ~~~~~~~~     Band7      ~~~~~~~~ 
#define PARAM_Band7_TYPE              (0x2046)  
#define PARAM_Band7_FREQ              (0x2047)
#define PARAM_Band7_GAIN              (0x2048)
#define PARAM_Band7_Q                 (0x2049)  


#define PARAM_COMP_ENABLE              (0x204A)  //         ~~~~~~~~ MIC压缩器(Comp) ~~~~~~~~
#define PARAM_COMP_MakeUpGain          (0x204B)  //          
#define PARAM_COMP_Threshold           (0x204C)  //          
#define PARAM_COMP_Attack              (0x204D)  //          
#define PARAM_COMP_Release             (0x204E)  //          
#define PARAM_COMP_Rate                (0x204F)  //
#define PARAM_COMP_KneeWidth           (0x2050)  //

                                       
#define PARAM_LIMIT_ENABLE             (0x2051)  //         ~~~~~~~~ MIC压缩器(Comp) ~~~~~~~~
#define PARAM_LIMIT_preGain            (0x2052)  //          
#define PARAM_LIMIT_Threshold          (0x2053)  //          
#define PARAM_LIMIT_Attack             (0x2054)  //          
#define PARAM_LIMIT_Release            (0x2055)  //          
#define PARAM_LIMIT_Rate               (0x2056)  //      

#define PARAM_MIC_SIG_LEVEL            (0x2057)  //         ~~~~~~~~  MIC电平指示     ~~~~~~~~        Garry
#define PARAM_MIC_SIG_ONOFF            (0x2058)  //         ~~~~~~~~  Mic 电平信号静音 ~~~~~~~~       Garry
#define PARAM_MONITOR_RATE             (0x2059)  //         ~~~~~~~~~~耳机监听总开关~~~~~~~~~~~~~~      Garry
#define PARAM_HP_GAIN                  (0x205A)  //         ~~~~~~~~~~耳机监听音量~~~~~~~~~~~~~~      Garry
#define PARAM_MIC_OutPut               (0x205B)  //         ~~~~~~~~~~MIC监听开关~~~~~~~~~~~~~~      Garry
#define PARAM_PC_OutPut                (0x205C)  //         ~~~~~~~~~~PC监听开关~~~~~~~~~~~~~~      Garry

/* A04 Gen2用不上的功能地址
#define PARAM_RGB_SHOW_ONOFF          (0x2056)
#define PARAM_HP_MUTE                 (0x2023)
#define PARAM_HP_GAIN                 (0x2025)
#define PARAM_MONITOR_RATE            (0x2026)

#define PARAM_USB_OUT_INC			  (0x202C)  // Steve
#define PARAM_GAME71_EN 			  (0x202D)  // Niko
#define PARAM_RGB_BRIGHT              (0x202A)
#define PARAM_RGB_SHOW_MODE           (0x202B)

  
*/

/* Python 自动生成的结束行 */


typedef struct
{
	uint8 *ReadBufTemp;
	
	DEV_INFO_PARAM *DevInfo;	// 按结构体索引
	CONFIG_PARAM   *Config;
	PRESET_PARAM   *Preset;
	
	uint16 *DevInfoArray;		// 按数组下标索引
	uint16 *ConfigArray;
	uint16 *PresetArray;
	
	struct
	{
		void (*DevInfo)(uint32 addr, uint32 data);
		void (*Config )(uint32 addr, uint32 data);
		void (*Preset )(uint32 addr, uint32 data);
	} Set;
	
	struct
	{
		void (*DevInfo)(void);
		void (*Config )(void);
		void (*Preset )(void);
	} SetDefault;
	
	struct
	{
		uint32 (*DevInfo)(uint32 addr);
		uint32 (*Config )(uint32 addr);
		uint32 (*Preset )(uint32 addr);
	} Get;
	 
	uint32 (*GetReInitFlag )(uint32 flag);
	uint32 (*Get1stInitFlag)(uint16 *modelCode);
    void (*GetFirmwareVer)(void);
	 
	void (*FwInfoSNSet)(void);

} PARAM_TBL_HANDLE;

extern PARAM_TBL_HANDLE ParamTbl;

#endif




