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
**	文件名	: Dsp.h
**
**	程序员	: 
**
**	日　期  : 2021年12月23日
**
**	文件描述: DSP类内部使用头文件
**
****************************************************************************************************
*/
#ifndef	_DSP2_H
#define	_DSP2_H
#include "UserRetype.h"



// Dsp.Program.ModuleTable
extern void DspModuleParamSet(uint32 *paramAddr, uint32 *addrStart, uint32 *addrEnd);
extern void DspMute(void);
extern void DspUnmute(void);

void DMParamSetFactory(void);


// Dsp.Driver
void DspDrv_MicVolModule(fp32 Vol);
void DspDrv_MicMuteModule  (uint32 mute);
void DspDrv_MicGainModule(int8 Gain);

//void DspDrv_NoiseSuppModule(uint32 en);
void DspDrv_NoiseSuppModule(uint32 en, int32 threshold);  // Liang
void DspDrv_DacMuteModule  (uint32 mute);
void DspDrv_4LevelsNoiseReduction(uint32 en, int32 levels);  // Garry  4档降噪
void DspDrv_MicVol_Add_GainModule(uint8 Volume);

void DspDrv_UsbOutIncModule(uint32 en);  // Steve 
void DspDrv_MicMonModule(uint32 en);  // Steve
void DspDrv_Game71Module(uint32 en);  // Niko
void DspDrv_Mixer_11(fp32 Vol);          //Garry
void Dsp_setFilterParams(int filterIndex, float gain, float fc, float q, uint32 type);//Garry
void Mcu_setFilterParams(int filterIndex, float gain, float fc, float q, uint32 type);//Garry
 
void Mcu_setFilterParams2(int filterIndex, float gain, float fc, float q, uint32 type);//Garry
                                                      
extern void Dsp_Original(void);
extern void Dsp_Noise(void);
 
extern void Mcu_Original(void);        //Garry
extern void Mcu_Noise(void);					 //Garry
extern void Mcu_Noise2(void);					 //Garry
extern void Mcu_Noise3(void);					 //Garry
#endif




