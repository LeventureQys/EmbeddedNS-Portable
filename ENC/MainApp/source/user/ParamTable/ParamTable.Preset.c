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
**	文件名	: ParamTable.Preset.c
**
**	程序员	: 
**
**	日　期  : 2022年7月22日
**
**	文件描述: 预设参数表
**
****************************************************************************************************
*/
#include "ParamTable.h"



/*	参数地址减去0x2000，就是本表的索引地址。
	给PresetTable赋值前，PresetTableDef的值就是出厂缺省值。
*/
uint16 PresetTable[PRESET_TBL_SIZE];

const uint16 PresetTableDef[PRESET_TBL_SIZE] =
{
/* Python 自动生成的起始行 */
//	0x0925,     // Checksum       ~~~~~~~~ 预设数据校验和 ~~~~~~~~
//	0x0975,     // Checksum       ~~~~~~~~ 预设数据校验和 ~~~~~~~~  Liang
	0x095E,     // Checksum       ~~~~~~~~ 预设数据校验和 ~~~~~~~~  Liang
	0x0001,     // ValidFlag      ~~~~~~~~ 预设有效标志 ~~~~~~~~
	0x0020,     // PresetName1    ~~~~~~~~ 预设名称（64字节） ~~~~~~~~
	0x0000,     // PresetName2    
	0x0000,     // PresetName3    
	0x0000,     // PresetName4    
	0x0000,     // PresetName5    
	0x0000,     // PresetName6    
	0x0000,     // PresetName7    
	0x0000,     // PresetName8    
	0x0000,     // PresetName9    
	0x0000,     // PresetName10   
	0x0000,     // PresetName11   
	0x0000,     // PresetName12   
	0x0000,     // PresetName13   
	0x0000,     // PresetName14   
	0x0000,     // PresetName15   
	0x0000,     // PresetName16   
	0x0000,     // PresetName17   
	0x0000,     // PresetName18   
	0x0000,     // PresetName19   
	0x0000,     // PresetName20   
	0x0000,     // PresetName21   
	0x0000,     // PresetName22   
	0x0000,     // PresetName23   
	0x0000,     // PresetName24   
	0x0000,     // PresetName25   
	0x0000,     // PresetName26   
	0x0000,     // PresetName27   
	0x0000,     // PresetName28   
	0x0000,     // PresetName29   
	0x0000,     // PresetName30   
	0x0000,     // PresetName31   
	0x0000,     // PresetName32   
	0x0000,     // MIC Mute       ~~~~~~~~ MIC静音(Mute) ~~~~~~~~
	0x003C,     // MIC Gain       ~~~~~~~~ MIC音量(Gain) ~~~~~~~~   默认60
	0x0000,     // OnOff          ~~~~~~~~ 降噪开关 ~~~~~~~~
	0x0038,     // Threshold      ~~~~~~~~ 降噪强度 ~~~~~~~~
	
    0x0000,     // MIC Mode       ~~~~~~~~ MIC均衡器模式(场景) ~~~~~~~~
    0x0001,     // Band-1 Enable  ~~~~~~~~ 参量均衡器(场景) ~~~~~~~~
    0x0004,     // Band-1 Type    
    0x0014,     // Band-1 Freq    
    0x0140,     // Band-1 Gain    
    0x0007,     // Band-1 Q       
		
    0x0001,     // Band-2 Enable  
    0x0003,     // Band-2 Type    
    0x4E20,     // Band-2 Freq    
    0x0140,     // Band-2 Gain    
    0x0007,     // Band-2 Q       
		
    0x0001,     // Band-3 Enable  
    0x0000,     // Band-3 Type    
    0x0064,     // Band-3 Freq    
    0x0140,     // Band-3 Gain    
    0x0007,     // Band-3 Q       
		
    0x0001,     // Band-4 Enable  
    0x0000,     // Band-4 Type    
    0x01F4,     // Band-4 Freq    
    0x0140,     // Band-4 Gain    
    0x0007,     // Band-4 Q       
		
    0x0001,     // Band-5 Enable  
    0x0000,     // Band-5 Type    
    0x03E8,     // Band-5 Freq    
    0x0140,     // Band-5 Gain    
    0x0007,     // Band-5 Q       
		
    0x0001,     // Band-6 Enable  
    0x0000,     // Band-6 Type    
    0x07D0,     // Band-6 Freq    
    0x0140,     // Band-6 Gain    
    0x0007,     // Band-6 Q      
		
    0x0001,     // Band-7 Enable  
    0x0000,     // Band-7 Type    
    0x2710,     // Band-7 Freq    
    0x0140,     // Band-7 Gain    
    0x0007,     // Band-7 Q       
    
    0x0000,     // MIC Enable     ~~~~~~~~ MIC压缩器(Comp) ~~~~~~~~
    0x0000,     // MIC MakeUpGain  
    0x0244,     // MIC Threshold  
    0x0005,     // MIC Attack     
    0x03E8,     // MIC Release    
    0x003C,     // MIC Rate       
    0x000A,     // MIC KneeWidth  

    0x0000,     // MIC Enable     ~~~~~~~~ MIC限幅器(Limit) ~~~~~~~~
    0x0000,     // MIC preGain    
    0x0244,     // MIC Threshold  
    0x0005,     // MIC Attack     
    0x01F4,     // MIC Release    
    0x0000,     // MIC Rate       

    0x0000,     // MIC Sig Level    ~~~~~~~~ MIC电平指示 ~~~~~~~~
    0x0000,     // MicSigLevelMute  ~~~~~~~~ MIC电平信号静音 ~~~~~~~~
    0x0001,     // MIC/PC Mix       ~~~~~~~~ 监听总开关 ~~~~~~~~        Garry
	  0x0046,     // MIC HP GAIN      ~~~~~~~~ 监听音量 ~~~~~~~~          Garry
	  0x0001,     // MIC OutPut GAIN  ~~~~~~~~ 麦克风监听开关 ~~~~~~~~    Garry
	  0x0001,     // PC  OutPut GAIN  ~~~~~~~~ 电脑监听开关 ~~~~~~~~      Garry
/* Python 自动生成的结束行 */
};


/*
****************************************************************************************************
**
**                                        PresetSetDefault()
**
**
**	功能描述: 对各预设参数赋初始值。
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
void PresetSetDefault(void)
{	
	for (uint32 i=0; i<PRESET_TBL_SIZE; i++)
	{
		PresetTable[i] = PresetTableDef[i];
	}
}




