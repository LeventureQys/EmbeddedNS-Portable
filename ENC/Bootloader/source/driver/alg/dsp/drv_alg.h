#ifndef	__DRV_ALG_H__
#define  __DRV_ALG_H__

#include "jat_alg_config.h"
#include "a0_hal_dsp.h"
#include "arm_math.h"
#include "jat_alg.h"
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


#define ALG_GAIN_NUM        		(MCU_ALG_GAIN_DACR_OUT+1)






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

void DspAlgInit(void);
void DspAlgWAnsMainProcess(void);



void DspAlgGainRefreshIdxSet(uint8_t idx);
void DrvUserAlgParaCheck(void *pParaIn);
void AlgGainStepChange(int32_t *gain, int32_t step);

void DrvAlgFrameProcess(void **pMem);
void DrvAlgProcess(void);
void DrvAlgInit(void);

void DrvAlgParaSet(uint32_t idx, uint32_t *pPara);

void DrvAlgRmsGetProcess(void);
void AlgParaSetTest(void);


#endif

