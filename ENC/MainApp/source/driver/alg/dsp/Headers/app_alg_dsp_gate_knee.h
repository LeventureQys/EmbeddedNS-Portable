


#ifndef __APP_ALG_DSP_GATE_KNEE_H__

#define __APP_ALG_DSP_GATE_KNEE_H__

#include "app_alg_dsp_config.h"


#ifndef PI
    #define PI               3.14159265358979f
#endif



#define	ALG_DSP_VAR_SIZE_GATE_KNEE		(15)


typedef struct
{
	float threshold_db;   // 阈值 (dB)
	float ratio;          // 扩展比 (如2.0表示2:1)
	float attack_ms;      // 启动时间 (ms)
	float release_ms;     // 释放时间 (ms)
	float knee_width_db;  // 膝宽 (dB)

	float hp_freq;        // 侧链高通滤波器截止频率 (Hz)

	int varAddr;
	uint8_t gateKneeRefreshVarFlag;
    
}ALG_DSP_PARA_GATE_KNEE;


typedef struct {

	// 内部状态变量
	float threshold_linear;   // 线性阈值
	float attack_coef;        // 启动系数
	float release_coef;       // 释放系数

	float hp_b0; 
	float hp_b1; 
	float hp_a1; 
	float hp_x_prev;          // 高通滤波器历史输入
	float hp_y_prev;          // 高通滤波器历史输出
	float gain_db;            // 当前增益 (dB)
	float env_follow;         // 包络跟随器状态

	float gain_linear;

	float ratio;
	float kneeLow;
	float kneeHigh;
	float kneeWidth;

	
}ALG_DSP_VAR_GATE_KNEE;



void DspAlgGateKneeParaSet(ALG_DSP_PARA_GATE_KNEE *pPara, ALG_DSP_VAR_GATE_KNEE *pVar );


void DspAlgGateKneeParaSetTest1(ALG_DSP_PARA_GATE_KNEE *pPara, ALG_DSP_VAR_GATE_KNEE *pVar );


void DspAlgGateKneeParaUpdate(uint32_t *pAddrDsp, uint32_t *pSrc);


void DspAlgGateKneeRefresh(ALG_DSP_PARA_GATE_KNEE *pPara, uint32_t dspAddr);

#endif 






