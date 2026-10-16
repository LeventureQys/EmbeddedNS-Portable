
extern ALG_DSP_PARA_MBD algParaMbd;


#define DSP_RUN_MODE_HIGH_DELAY		(1)
#define DSP_RUN_MODE_LOW_DELAY		(0)

// 0, low-delay mode
// 1, high-delay mode
uint8_t obsvDspRunMode = 0;



// for dsp switch mode used VARs
int32_t dsp_switch_state = 0;
uint32_t dsp_state_count = 0;
uint8_t	dsp_mode_staged = 0;
// just for dbg usage
uint8_t dsp_mode_staged_err_times = 0;





//void DspAlgEqParaRefresh(void);
void algDspRun(void);

typedef struct
{
    float		frameAdcL[TM_FIFO_LEN];	// in adc / out1

	// later frame adcR can be removed
    float		frameAdcR[TM_FIFO_LEN];	//          out2


    int32_t     dspBuffIdx;
    int32_t     mcuBuffIdx;

    int32_t		tm_inremain;
} ALG_GENENRAL_TM_DELAYLINE;


ALG_GENENRAL_TM_DELAYLINE   *palgGeneralTmDelayLine;
uint8_t dbgDspCycleArrayIdx = 0;
volatile uint32_t dbgDspCycleArray[64];
volatile uint32_t dbgDspCycleCnt[3];
volatile int32_t dbgDspCycleSum = 0;
uint32_t dsp_not_finished_time = 0;


uint8_t bDspRunEn = 0;
extern uint8_t dspAlgAdcEqRefreshEn;

extern float32_t dspAlgEqCoeffW2Dsp[2][7][5];
extern float32_t dspAlgSpkEqCoeffW2Dsp[7][5];

float_t sidetoneVol;
float_t sidetoneVolCur;
extern uint32_t SidetoneFlag;
static uint8_t dspAlgGainRefershIdx = 0;
extern int32_t noiseGateParaGroup0[1][7];
extern int32_t algVarGate[10];
/*************增益刷新*******************/
uint8_t  dspAlgGainRefreshEn =0 ;
extern float gainDspPara[DSP_GAIN_NUM];

__RAM_CODE void DspAlgGainParaRefresh(float *gainPara)
{
    if (dspAlgGainRefreshEn == 1)//调dsp,Gain
    {
       LibDrvMemcpy((uint8_t *)DSP_IRAM_ADDR(DSP_GAIN_ADC_1_04_ADDR), (uint8_t *)&gainDspPara[0], 4);
       LibDrvMemcpy((uint8_t *)DSP_IRAM_ADDR(DSP_GAIN_DIRECT10_ADDR), (uint8_t *)&gainDspPara[1], 4);
       LibDrvMemcpy((uint8_t *)DSP_IRAM_ADDR(DSP_GAIN_09_ADDR), (uint8_t *)&gainDspPara[2], 4);
       LibDrvMemcpy((uint8_t *)DSP_IRAM_ADDR(DSP_GAIN_MIXER11_ADDR), (uint8_t *)&gainDspPara[3], 4);
       LibDrvMemcpy((uint8_t *)DSP_IRAM_ADDR(DSP_GAIN_ADCIN_ADDR), (uint8_t *)&gainDspPara[4], 4);

        dspAlgGainRefreshEn = 0;
    }
}

//interface to refresh eq in dsp
__RAM_CODE void DspAlgEqParaRefresh(void)
{
    if (dspAlgAdcEqRefreshEn == 1)
    {   //adc eq
        uint32_t hidEqDspAddr;
        hidEqDspAddr = DSP_IRAM_ADDR(DSP_EQ_START_ADDR);
        LibDrvMemcpy((uint8_t *)hidEqDspAddr, (uint8_t *)dspAlgEqCoeffW2Dsp[0][0], 25 * 4);
        dspAlgAdcEqRefreshEn = 0;
    }
   
}

__RAM_CODE static void DspAlgDataAccess(float32_t **fpInOut, ALG_GENENRAL_TM_DELAYLINE *pPara, uint32_t frameSize)
{
// can only run @ high delay mode
    float32_t *pChInOut[2];
    uint16_t  cntI;
    uint32_t  dspIramAddrInOut[2];
    // to store fifo length
    int32_t	idx_cmp_val1;
    int32_t	in_remain_dec_val1;

    union
    {
        int32_t     i32;
        float       f32;
    } inData[1];
    union
    {
        int32_t     i32;
        float       f32;
    } outData[2];

    // judge whether dsp has stopped
    if ( REG_DSP_CON_RUN_CHECK() != 0)
    {
        // dsp hasn't stopped
        dsp_not_finished_time++;
        //return;
    }
	// check for current mode dsp run at
	if ( algDspGetRunMode() != DSP_RUN_MODE_HIGH_DELAY)
	{
		return;
	}

	if (dsp_switch_state == 1)
	{
		DSP_IRAM(DSP_GAIN_MIXER11_ADDR) = 0;
		DSP_IRAM(DSP_GAIN_ADCIN_ADDR) = 0;

		if (DSP_IRAM(DSP_R_GAIN_MIXER11_ADDR) == 0)
		{
			// real input, output gain has reached '0'
			dsp_switch_state = 2;
		}
	}

	else if (dsp_switch_state == 2)
	{
		if (dsp_mode_staged == DSP_RUN_MODE_HIGH_DELAY)
		{ // should NOT occur
			dsp_mode_staged_err_times++;
		}
		DSP_IRAM(DSP_RUN_MODE_ADDR) = DSP_RUN_MODE_LOW_DELAY;
		dsp_switch_state = 3;
		
		obsvDspRunMode = DSP_RUN_MODE_LOW_DELAY;
		return;
		
	}

//	else if (dsp_switch_state == 3)
//	{ 
//		dsp_state_count++;
//		if (dsp_state_count == 10)
//		{
//			dsp_switch_state = 4;
//		}
//	}

	else if (dsp_switch_state == 3)
	{ // time to restore input, output gain
		DSP_IRAM(DSP_GAIN_MIXER11_ADDR) = 0x3F800000;
		DSP_IRAM(DSP_GAIN_ADCIN_ADDR) = 0x3F800000;
		dsp_switch_state = 0;
	}

    dspIramAddrInOut[0] = DSP_ANS_IN_ADCL_ADDR;
	
    idx_cmp_val1 = TM_FIFO_LEN;
    in_remain_dec_val1 = TM_START_PROC_LEN;

    int32_t idx = pPara->dspBuffIdx;

    // ring buffer adcL
    pChInOut[0] = *fpInOut++;
    // ring buffer adcR
    pChInOut[1] = *fpInOut++;


    for (cntI = 0; cntI < frameSize; cntI++)
    {
        // outData[0] is usbL out data, (read from dsp iram)
        outData[0].i32 = DSP_IRAM(dspIramAddrInOut[0]);
        // inData[0] is adcL in data (to go through ans)
        inData[0].f32 = pChInOut[0][idx];
        DSP_IRAM(dspIramAddrInOut[0]) = inData[0].i32;

		// ans processed data all fetched from dspIramAddrInOut[0]
		//outData[1].i32 = outData[0].i32;

		// usb mic L/R
		pChInOut[0][idx] = outData[0].f32;
		//pChInOut[1][idx] = outData[1].f32;
		

        // increase dsp iram pointer addr
        dspIramAddrInOut[0]++;
        dspIramAddrInOut[1]++;

        idx++;
        if (idx >= idx_cmp_val1)
        {
            idx = 0;
        }
    }
    pPara->dspBuffIdx = idx;
    pPara->tm_inremain = pPara->tm_inremain - in_remain_dec_val1;


#if 0
    dbgDspCycleCnt[0] = REG_DSP_CNT();
    if (dbgDspCycleCnt[0] > dbgDspCycleCnt[1])
    {
        dbgDspCycleCnt[1] = dbgDspCycleCnt[0];
    }
    dbgDspCycleArray[dbgDspCycleArrayIdx++] = dbgDspCycleCnt[0];
    dbgDspCycleArrayIdx = dbgDspCycleArrayIdx & 0x3F;
    dbgDspCycleSum = dbgDspCycleSum - dbgDspCycleArray[dbgDspCycleArrayIdx] + dbgDspCycleCnt[0];
    // average dsp cycle
    dbgDspCycleCnt[2] = dbgDspCycleSum / 64;
#endif

	  DspAlgMBDRefresh(&algParaMbd);
		DspAlgGainParaRefresh(&gainDspPara[0]);
    DspAlgEqParaRefresh();
    algDspRun();
    return;
}


__RAM_CODE void DspAlgFrameHighDlyProcess(float32_t **pfMem)
{
	ALG_GENENRAL_TM_DELAYLINE *pPara = palgGeneralTmDelayLine;
	float32_t *pFifoInAdcL = pPara->frameAdcL;
	//float32_t *pFifoInAdcR = pPara->frameAdcR;
	
	float32_t *pBuf[2];
	pBuf[0] = *pfMem++; 	// adcL
	
	
	float32_t	fSample[1];
	int32_t idx = pPara->mcuBuffIdx;
	for (uint32_t i = 0; i < ALG_FRAME_LEN; i++)
	{
	
		// exchange data from buffer and delayline
		fSample[0] = pFifoInAdcL[idx];
		
		pFifoInAdcL[idx] = pBuf[0][i];
		pBuf[0][i] = fSample[0];
		
	
	
		idx++;
		if (idx == TM_FIFO_LEN)
		{
			idx = 0;
		}
	}
	pPara->mcuBuffIdx = idx;
	pPara->tm_inremain = pPara->tm_inremain + ALG_FRAME_LEN;

}


__RAM_CODE void DspAlgFrameLowDlyProcess(float32_t **pfMem)
{
	uint16_t  cntI;
    uint32_t  dspIramAddrInOut[2];
	float32_t *pChInOut[1];

	union
    {
        int32_t     i32;
        float       f32;
    } inData[1];
    union
    {
        int32_t     i32;
        float       f32;
    } outData[2];

	if ( REG_DSP_CON_RUN_CHECK() != 0)
    {
        // dsp hasn't stopped
        dsp_not_finished_time++;
        //return;
    }
	
	// check for current mode dsp run at
	if ( algDspGetRunMode() != DSP_RUN_MODE_LOW_DELAY)
	{
		return;
	}

	
	if (dsp_switch_state == 1)
	{
		DSP_IRAM(DSP_GAIN_MIXER11_ADDR) = 0;
		DSP_IRAM(DSP_GAIN_ADCIN_ADDR) = 0;

		if (DSP_IRAM(DSP_R_GAIN_MIXER11_ADDR) == 0)
		{
			// clear delayLine
			LibDrvMemset((void*)palgGeneralTmDelayLine, 0, sizeof(ALG_GENENRAL_TM_DELAYLINE));
			// real input, output gain has reached '0'
			dsp_switch_state = 2;
		}
	}

	else if (dsp_switch_state == 2)
	{
		if (dsp_mode_staged == DSP_RUN_MODE_LOW_DELAY)
		{ // should NOT occur
			dsp_mode_staged_err_times++;
		}
		DSP_IRAM(DSP_RUN_MODE_ADDR) = DSP_RUN_MODE_HIGH_DELAY;
		dsp_switch_state = 3;
		obsvDspRunMode = DSP_RUN_MODE_HIGH_DELAY;
		dsp_state_count = 0;
		// clear output buffer
		pChInOut[0] = *pfMem;
		LibDrvMemset((void*)pChInOut[0], 0, ALG_FRAME_LEN*4);
		return;
		
	}

//	else if (dsp_switch_state == 3)
//	{ // time to restore input, output gain
//		dsp_switch_state = 4;
//	}

	else if (dsp_switch_state == 3)
	{
	// time to restore input, output gain
		DSP_IRAM(DSP_GAIN_MIXER11_ADDR) = 0x3F800000;
		DSP_IRAM(DSP_GAIN_ADCIN_ADDR) = 0x3F800000;
		dsp_switch_state = 0;
	}
	
	dspIramAddrInOut[0] = DSP_ANS_IN_ADCL_ADDR;
	pChInOut[0] = *pfMem++;

	for (cntI = 0; cntI < ALG_FRAME_LEN; cntI++)
	{
	
		outData[0].i32 = DSP_IRAM(dspIramAddrInOut[0]);

		inData[0].f32 = pChInOut[0][cntI];
		DSP_IRAM(dspIramAddrInOut[0]) = inData[0].i32;

		pChInOut[0][cntI] = outData[0].f32;

		dspIramAddrInOut[0]++;
		
	}
	
	DspAlgGainParaRefresh(&gainDspPara[0]);
	DspAlgMBDRefresh(&algParaMbd);
	DspAlgEqParaRefresh();
  DspAlgGateUnitRefresh((ALG_DSP_PARA_GATE *)&noiseGateParaGroup0[0][0]);
	algDspRun();
	return;

}



__RAM_CODE void DspAlgFrameProcess(float32_t **pfMem)
{
    if (bDspRunEn != 0)
    {

		if (obsvDspRunMode == 1)
		{
			DspAlgFrameHighDlyProcess(pfMem);
		}
		
		else if (obsvDspRunMode == 0)
		{
			DspAlgFrameLowDlyProcess(pfMem);
		}
		
    }
}


/*
 * WAns means with ans
 */
__RAM_CODE void  DspAlgWAnsMainProcess(void)
{
    if ( (bDspRunEn != 0) && (obsvDspRunMode == DSP_RUN_MODE_HIGH_DELAY))
    {
        if (REG_DSP_CON_RUN_CHECK() == 0)
        {
            ALG_GENENRAL_TM_DELAYLINE *pPara = palgGeneralTmDelayLine;
            float32_t *pFifoAdcL = pPara->frameAdcL;
            float32_t *pFifoAdcR = pPara->frameAdcR;

            float32_t *pSrcInOut[2] =
            {
                pFifoAdcL,
                pFifoAdcR,
            };

            DspAlgDataAccess(pSrcInOut, pPara, TM_START_PROC_LEN);
        }
    }
    return;
}

__RAM_CODE uint32_t DrvAlgAnsTmCalcChk(void)
{
	if (obsvDspRunMode == DSP_RUN_MODE_LOW_DELAY)
	{ // if in low delay mode
		return 0;
	}
    uint32_t ReturnValue = 0;;
    ALG_GENENRAL_TM_DELAYLINE *pPara = palgGeneralTmDelayLine;
    if (pPara->tm_inremain > TM_START_PROC_LEN)
    {
        ReturnValue = 1;
    }
    else
    {
        ReturnValue = 0;
    }

		
    return ReturnValue;
}




/*
 * mode = 0, switch to short-delay mode
 * mode = 1, switch to long-delay mode
 * return value 
 * 1, normal
 * 2, 
 *
 */
__RAM_CODE uint8_t AppAlgDspRunModeSwitch(uint8_t mode)
{
//_Pragma ("push")		 \
//_Pragma ("O0")
//代码区，函数

	if (dsp_switch_state != 0)
	{
		return 2;
	}
	if ( mode >= 2 )
	{ // check for validity
		return 2;
	}

	if (obsvDspRunMode == mode)
	{ // already in this mode
		return 2;
	}

	// trigger dsp in volume(GainAdcIn) and out volume(Gain_Mixer_11) to '0'
	dsp_switch_state = 1;

	
	
	dsp_mode_staged = mode;

	return 1;
//_Pragma ("pop")	
}



