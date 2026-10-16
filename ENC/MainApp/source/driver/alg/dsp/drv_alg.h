#ifndef	__DRV_ALG_H__
#define  __DRV_ALG_H__

#include "jat_alg_config.h"
#include "a0_hal_dsp.h"
#include "arm_math.h"
#include "jat_alg.h"
#include "algDspUser.h"
//#include "para_default_filter.c"
//#include "para_default_filter.h"

#define TM_START_PROC_LEN	(480)
#define TM_FIFO_LEN			(TM_START_PROC_LEN*2)

// may add gradual change
#define MCU_ALG_GAIN_IDX_ADC_LIN	(0)
#define MCU_ALG_GAIN_IDX_ADC_RIN	(1)
#define MCU_ALG_GAIN_IDX_SPK_LIN	(2)
#define MCU_ALG_GAIN_IDX_SPK_RIN	(3)

#define Mcu_Gain_USB_15							(4)
#define Mcu_Gain_Direct_12					(5)

#define MCU_ALG_GAIN_USBL_OUT		(6)
#define MCU_ALG_GAIN_USBR_OUT		(7)

#define MCU_ALG_GAIN_DACL_OUT		(8)
#define MCU_ALG_GAIN_DACR_OUT		(9)


#define MCU_ALG_GAIN_Mix2_USB		(10)
//Mix2_USB      Garry
//#define MCU_ALG_GAIN_Mix2_USBl		(10)
//#define MCU_ALG_GAIN_Mix2_USBR		(11)



//#define ALG_GAIN_NUM        		(MCU_ALG_GAIN_DACR_OUT+1)
//Garry
#define ALG_GAIN_NUM        		(MCU_ALG_GAIN_Mix2_USB+1)

// in that if codec set to 96K, fifo length need 96*10*2 (10ms)
//#define TM_START_PROC_LEN_96K	(960)
//#define TM_FIFO_LEN_96K			(TM_START_PROC_LEN_96K*2)

// 2*TM_FIFO_LEN + 3 = 
#define	ANS_RINGBUF_LEN				(1923)

typedef enum
{   
    DRV_ALG_IDX_EQ,
    DRV_ALG_IDX_GAIN,
}DRV_ALG_IDX;


/*
初始化DSP算法模块，包括ANS、FFT、EQ、DC阻塞、噪声门、增益控制、压缩器、
最后，函数通过设置 bDspRunEn = 1 来启用 DSP 运行。
*/
void DspAlgInit(void);



/*
DSP算法的主处理函数，主要用于满足条件时对输入信号进行处理
函数根据bDspRunEn和obsvDspRunMode的值判断是否需要执行处理逻辑
如果条件满足，函数会从延迟线缓冲区中获取输入数据，并调用DspAlgDataAccess 函数进行数据处理。
*/
void DspAlgWAnsMainProcess(void);




/*
设置dsp算法增益刷新id设置
*/
void DspAlgGainRefreshIdxSet(uint8_t idx);



/*
检查并处理用户传入的参数，根据参数内容执行相应的操作
*/
void DrvUserAlgParaCheck(void *pParaIn);
void AlgGainStepChange(int32_t *gain, int32_t step);

/*
音频帧处理
DrvAlgFrameProcess 主要执行以下任务：
输入信号处理       ：将定点输入信号转换为浮点信号，并根据需要缩小精度。
增益调整           ：根据特定的增益值调整输入信号的强度。
DC阻塞             ：去除输入信号中的直流偏移。
信号混合           ：对左声道和右声道的信号进行混合处理。
游戏模式处理       ：根据选择的模式进行不同的音频处理（如游戏音效）。
滤波处理           ：对信号进行滤波（如均衡器处理）。
频率偏移           ：调整信号的频率范围。
USB麦克风信号处理  ：对USB麦克风输入信号进行增益调整。
输出信号调整       ：将处理后的信号输出到指定的缓冲区。
*/
void DrvAlgFrameProcess(void **pMem);



/*
DSP算法的主处理函数，主要用于满足条件时对输入信号进行处理
*/
void DrvAlgProcess(void);




/*
初始化DSP算法模块
*/
void DrvAlgInit(void);


/*
设置特定的数字信号处理（DSP）算法参数，它通过 idx 索引选择不同的算法参数
并根据不同的算法类型调用不同的参数设置函数
*/
void DrvAlgParaSet(uint32_t idx, uint32_t *pPara);

void DrvAlgRmsGetProcess(void);

/*
作用：根据 testMbdParaSetFlag 的不同值，执行不同的算法参数设置操作
AlgParaSetTest 函数通过检查 testMbdParaSetFlag 的值，选择性地配置不同的 DSP 算法参数。
不同的值触发不同的 DSP 算法配置，例如 MBD、EQ（均衡器）、噪声门等，
每次设置后会调用频率移位参数设置函数，并重置标志位 testMbdParaSetFlag 为 0，表示参数设置完成。
*/
void AlgParaSetTest(void);
__RAM_CODE void DspAlgDrcParaRefresh(uint32_t addrStart);

extern float gainDspPara[DSP_GAIN_NUM];


extern ALG_DSP_PARA_MBD algParaMbd;           //Garry
extern int32_t     compParaGroup0[5];         //Garry
extern void ControlNoiseGate(uint8_t state);  //Garry
extern void ControlAns(uint8_t state);        //Garry
extern void DspAlgInit(void);                 //Garry
extern __RAM_CODE void DrvAlgFrameProcess(void **pMem);     //Garry
extern float rmsValue[2];                                   //Garry
#endif

