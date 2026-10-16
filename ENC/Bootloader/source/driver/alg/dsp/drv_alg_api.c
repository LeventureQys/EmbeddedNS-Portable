void UserEnterUpdateMode(void);
float32_t dspAlgEqCoeffW2Dsp[2][7][5];
float32_t dspAlgSpkEqCoeffW2Dsp[7][5];


uint8_t filterSpkRefreshEn = 0;
uint8_t filterDacRefreshEn = 0;

extern float spkFilterCoeff[5 *SPK_FILTER_NUM];

extern float spkFilterCoeffSet[5 * SPK_FILTER_NUM];

extern float dacFilterCoeff[5 * DACOUT_FILTER_NUM];
extern float dacFilterCoeffSet[5 * DACOUT_FILTER_NUM];

void DrvAlgUserParaInit(uint32_t idx, uint16_t *pPara)
{
    if (idx == 0)
    {   // adc eq para
      
        for (uint32_t i = 0; i < 5; i++)
        {
            uint16_t info = *pPara++;
            paraDefaultAdcEq[0][i].type = (uint32_t)((uint8_t)info);
            paraDefaultAdcEq[0][i].gain = (float_t)((int32_t)((int8_t)(info >> 8)));
            info = *pPara++;
            paraDefaultAdcEq[0][i].fc = (float_t)((int32_t)((int16_t)info));
            info = *pPara++;
            paraDefaultAdcEq[0][i].q = (float_t)((int32_t)((int16_t)info)) / 10.0f;
        }
    }

}


// dsp adc 5 seg eq set para 
void DspAlgEqParaSet(ALG_PARA_FILTER *pEqPara, uint32_t segNumMax, uint32_t EqIdx)
{
    if(EqIdx == 0)
    {   //adc eq
        for (uint32_t i = 0; i < segNumMax; i++)
        {
            AlgFilterDesign(pEqPara, &dspAlgEqCoeffW2Dsp[EqIdx][i][0], ALG_SAMPLE_RATE);
            pEqPara++;
        }
        dspAlgAdcEqRefreshEn = 1;
    }

    return;
}


//mcu eq
__RAM_CODE void McuAlgFilterRefresh(void)
{
    if (filterSpkRefreshEn != 0)
    {
        memcpy(&spkFilterCoeff, &spkFilterCoeffSet, sizeof(spkFilterCoeffSet));

        filterSpkRefreshEn = 0;
    }
		 if(filterDacRefreshEn != 0)
    {
        memcpy(&dacFilterCoeff, &dacFilterCoeffSet, sizeof(dacFilterCoeffSet));
        filterDacRefreshEn = 0;
    }
		
}

//mcu eq
__RAM_CODE void McuAlgEqParaSet(ALG_PARA_FILTER *pPara, uint32_t segNumMax, uint32_t EqIdx)
{
    if(EqIdx == 0)
    {   //spk eq
        for (uint32_t i = 0; i < segNumMax; i++)
        {
            AlgFilterDesign(pPara, &spkFilterCoeffSet[5 * i], ALG_SAMPLE_RATE);
            pPara++;
        }
        filterSpkRefreshEn = 1;
    }
  if(EqIdx == 1)
    {   //dac eq
        for (uint32_t i = 0; i < segNumMax; i++)
        {
            AlgFilterDesign(pPara, &dacFilterCoeffSet[5 * i], ALG_SAMPLE_RATE);
            pPara++;
        }
        filterDacRefreshEn = 1;
    }
		
    return;
}



// 设置dsp算法增益刷新id设置
void DspAlgGainRefreshIdxSet(uint8_t idx)
{
    dspAlgGainRefershIdx = idx;
}


__RAM_CODE void DrvAlgParaSet(uint32_t idx, uint32_t *pPara)
{
    switch(idx)
    {
	    case DRV_ALG_IDX_EQ:
	        DspAlgEqParaSet((ALG_PARA_FILTER *)&pPara[1], 5, (uint32_t)pPara[0]);
          McuAlgEqParaSet((ALG_PARA_FILTER *)&pPara[1], 5, (uint32_t)pPara[0]);
	        break;

	    default:
	        break;
    }
}


void DrvUserAlgParaCheck(void *pParaIn)
{
    uint32_t *pParaBuff = (uint32_t *)pParaIn;
    if (pParaBuff[0] == 0)
    {
        // 长度为0，无需处理
        return;
    }
    uint32_t address = pParaBuff[1];
    if (address >= 0x8000)
    {
        return;
    }
    if (address == 0x3FFF)
    {
        UserEnterUpdateMode();
    }
    switch (address)
    {
    // 根据实际情况添加处理代码
    default:
        break;
    }
    pParaBuff[0] = 0;
    return;
}

