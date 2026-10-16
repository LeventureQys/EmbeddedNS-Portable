
#ifndef __APP_ALG_DSP_ANS_H__
#define __APP_ALG_DSP_ANS_H__

#define	DSP_ANS_RUN_MODE	(1)


#define	DSP_ANS_DNSAMPLE_EQ_SEG	(4)

// cked
#define	DSP_ANS_ADDR_FFT256_SIN_TABLE	(0x0971)
#define DSP_ANS_ADDR_FFT256_LOG_TABLE	(DSP_ANS_ADDR_FFT256_SIN_TABLE+1)

// cked
#define	DSP_ANS_ADDR_FFT256_C_1D256		(0xE62)

#define	DSP_ALG_ANS_VAR_SIZE				(0x42)
#define	DSP_ALG_ANS_CALCBUFF_SIZE			(0x1A6D)


// address of nsAnsEqDownCoeff
// cked
#define DSP_ANS_INIT_ADDR					(0xE11)
// cked
#define DSP_ANS_ONOFF_ADDR					(0xF41)

// cked
#define DSP_GATE_ONOFF_ADDR					(0xFB3)
// cked
#define	DSP_ADDR_FFT256_R4_BR_TABLE_SRAM	(0xE64)

#define	DSP_ADDR_FFT256_R4_BR_TABLE_IRAM	(0xE65)

typedef struct
{
	uint8_t		ansOnOff;
	// new added var, for ans noise suppression mode select
	uint8_t		ansMode;
	
	int32_t		varAddr;	
	int32_t		bufferAddr2;
	
}DSP_ALG_PARA_ANS;


int32_t DspAlgANSVarSize(void);

int32_t DspAlgANSCalcBuffSize(void);

void DspANSAlgSinLogTbInit(void);

void DspANSAlgFFTInit(void);

void DspAlgANSVarInit(DSP_ALG_PARA_ANS *pPara);

void DspAlgANSCalcBufferInit(DSP_ALG_PARA_ANS *pPara);

void DspAlgANSParaUpdate(int32_t *pAddrDsp);

void DspAlgFFTRadix4BitRvsTbInit(uint8_t tableIsInSram);



#endif


