#ifndef	__APP_ALG_DSP_SPLIT_FILTER_H__
#define	__APP_ALG_DSP_SPLIT_FILTER_H__

// cked
#define DSP_SPLT_FILTER_BUF_ADDR		(0xF8B)

// address of  splFilt_ap_coeff1
// cked
#define DSP_SPLT_FILTER_COEFF_ADDR		(0xFA9)


#define DSP_RESAMP_FILTER_COEFF_ADDR	(DSP_SPLT_FILTER_COEFF_ADDR + 6)

#define	DSP_ALG_SPLFILTER_DELAYLINE_SIZE	(160*6)


typedef struct
{
	int	delayLineAddr;

}DSP_ALG_PARA_SPLT_FILT;



void DspAlgSplitFilterInit(DSP_ALG_PARA_SPLT_FILT *pPara);


#endif

