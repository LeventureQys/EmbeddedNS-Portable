/****************************************************************************/
/* System Name  : A0                                                        */
/* File Name    : algDspUser.h                                              */
/* Version      : 0.0.0                                                     */
/* Histroy      :                                                           */
/* Date         : 2021/09/15                                				*/
/*                Initial version                                           */
/* Discription  : A0 alg configuration                                      */
/****************************************************************************/
/*!
 @file algDspUser.h

 @brief
	 A0 alg configuration header file(s)

 @details


 @author
	  

 @version
	 0.0.0

 @date
	  
*/

#ifndef __ALGDSPUSER_H
#define __ALGDSPUSER_H

/* Includes ------------------------------------------------------------------*/
//#include "global.h"
#include "jat_typedef.h"
#include "alg_gamemode4.h"
#include "alg_game71.h"
#include "app_alg_dsp_mbd.h"



// dsp run ans iram address
// adcL in , also also processed out
#define	DSP_ANS_IN_ADCL_ADDR				(0x11)

// only after ans 5 seg filter 
#define DSP_EQ_GROUP_NUM					(1)

#define	DSP_EQ_SEG_ADC_NUM					(5)

#define DSP_EQ_START_ADDR					(0xEDE)				


#define	DSP_GATE_GROUP_NUM					(1)

#define DSP_GATE_START_ADDR					(0xF02)	

#define	DSP_GAIN_NUM						(5)

#define	DSP_GAIN_INIT_ADDR					(0xF0C)
#define	DSP_GAIN_ADC_1_04_ADDR				(DSP_GAIN_INIT_ADDR)
#define	DSP_GAIN_DIRECT10_ADDR				(DSP_GAIN_ADC_1_04_ADDR+1)
#define	DSP_GAIN_09_ADDR					(DSP_GAIN_DIRECT10_ADDR+1)
#define	DSP_GAIN_MIXER11_ADDR				(DSP_GAIN_09_ADDR+1)
#define	DSP_GAIN_ADCIN_ADDR					(DSP_GAIN_MIXER11_ADDR+1)


#define	DSP_R_GAIN_INIT_ADDR				(0xF11)
#define	DSP_R_GAIN_ADC_1_04_ADDR			(DSP_R_GAIN_INIT_ADDR)
#define	DSP_R_GAIN_DIRECT10_ADDR			(DSP_R_GAIN_ADC_1_04_ADDR+1)
#define	DSP_R_GAIN_09_ADDR					(DSP_R_GAIN_DIRECT10_ADDR+1)
#define	DSP_R_GAIN_MIXER11_ADDR				(DSP_R_GAIN_09_ADDR+1)
#define	DSP_R_GAIN_ADCIN_ADDR				(DSP_R_GAIN_MIXER11_ADDR+1)



#define	DSP_MBD_INIT_ADDR					(0xF16)


#define	DSP_RUN_MODE_ADDR					(0xF45)

/** @addtogroup A0_ALG_CLASS
  * @{
  */




/*!
	@brief
		reverb alg used variable and delayLine
*/
#define	ALG_REV_VAR_SIZE			(141)//(134)			// 0x86 32-bit
#define	ALG_REV_DELAYLINE_SIZE		(14930*2)				// for 4/4 sr

/*!
	@brief
		stereo echo alg used variable and delayLine
*/
#define ALG_SECHO_VAR_SIZE			(24)				// 0x18 32-bit
#define ALG_SECHO_DELAYLINE_SIZE	(19202)					// 16-bit delayLine size

/*!
	@brief
		stereo modulator alg used variable and delayLine
*/
#define	ALG_MOD_VAR_SIZE			(0x3A)	
#define	ALG_MOD_DELAYLINE_SIZE		(0x1B5D*2)

/*!
	@brief
		mono pitchshift alg used variable and delayLine
*/
#define ALG_PITCH_VAR_SIZE				(0xB)
#define ALG_PITCH_DELAYLINE_SIZE		(0x8FE)

/*!
	@brief
		stereo tremolo alg used variable and delayLine
*/
#define ALG_TREMOLO_VAR_SIZE                (0x1E)       //byte number = size * 4
#define ALG_TREMOLO_DELAYLINE_SIZE          (0x107)       //byte number = size * 4


/*!
	@brief
		stereo wah alg used variable
*/
#define	ALG_WAH_VAR_SIZE					(0x11)


/*!
	@brief
		stereo phaser alg used variable
*/
#define	ALG_PHA_VAR_SIZE					(0x47)

/*!
	@brief
		sample delay alg used variable
*/

#define ALG_DSP_SAMPLEDELAY_VAR_SIZE		(0x6)
#define ALG_DSP_SAMPLEDEALY_PARA_SIZE		(0x4)

//#define ALG_DSP_SAMPLEDELAY_DELAYLINE_SIZE		(0x4B0)
// maximum 20ms delay
#define ALG_DSP_SAMPLEDELAY_DELAYLINE_SIZE		(0x1E5)




/******************************************************************************/
/*							Struct of user defined para						  */
/******************************************************************************/



typedef struct {

    float32_t** pGroupPara;         /*!< 
    								data pointers point to equalizers' parameters , parameter should be defined as float type, 
    								the sequence of para is : Gain , Fc, Q , Type
									
									
									Gain
									effective when type is 0,1,2


									Fc
									effetive for all type

									Q
    								effective when type is 0,3,4

    								Type
									0: peak, 2 order
									1: low shelf,  2 order
									2: high shelf,  2 order
									3: low pass filter,  2 order
									4: high pass filter,  2 order 
									5: butterworth low pass, 1 order
									6: butterworth high pass, 1 order
									7: butterworth low pass, 2 order
									8: butterworth high pass, 2 order

									
    								*/
									//每组EQ的参数指针, Type/Gain/Fc/Q
} ALG_DSP_EQ_PARA;


typedef	struct {
	
	int32_t** pGroupPara;			/*!<
									data pointers point to noisegates' parameters, parameter should be defined as int32_t type ,
									The sequence of para is preGain, preGainEnergy, threshold, attackTime, releaseTime, level, varAddr
									
									preGain
									range is 0~128
									
									preGainEnergy
									range is 0~128
									
									threshold
									range is -91~0
									
									attackTime
									range is 1~400
									
									releaseTime
									range is 5~680
									
									level
									compared min energy range is 0~65536
									
									varAddr
									data pointer points to variable used in dsp , need to initialize to zero by default 
									*/
	 
									

}ALG_DSP_NGATE_PARA;


//EQ算法初始化
//返回：0, 初始化成功
//      -1,当前DSP正在运行，初始化失败
//      0xn1, 第n组的pos已经被初始化算法占用，n之前的成功初始化
//      0xn2, 第n组的iram不够，n之前的成功初始化

/*!
	@brief
		Initialize equalizer module in dsp
	@param 	*pPara
		see structure @ref ALG_DSP_EQ_PARA
	@return 
		0, success
		-1, failure, dsp is running
		0xn1, failure, position defined has been occupied by other alg
		0xn2, failure, iram in dsp has exceed limit
		
*/

int32_t algDspInitEq(ALG_DSP_EQ_PARA *pPara);

int32_t algDspInitGate(ALG_DSP_NGATE_PARA *pPara);

int32_t algDspInitGain(float* gainPara);

int32_t algDspInitMBD(ALG_DSP_PARA_MBD *pPara);

// write data to dsp iram, 
// algDspSetRunMode() can be called when dsp has STOPPED
void algDspSetRunMode(uint32_t mode);
// read data from dsp iram 
// algDspGetRunMode() can be called when dsp has STOPPED
uint32_t algDspGetRunMode(void);

/*!
	@brief
		A0 dsp initialize function
		download program to A0 dsp
	@param 
		none
	@return none	
*/
/*
 * dspVer = 0,  with ans
 * dspVer = 1, no ans
 */
void algDspInit(void);





/*!
	@brief
		get version number of current dsp alg lib
	@param *pVerStr
		pointer where lib version store
	@return
		length of version number	
*/
int DspAlgVersionGet(char *pVerStr);


#endif

