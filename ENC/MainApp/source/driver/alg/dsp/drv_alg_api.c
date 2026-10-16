void UserEnterUpdateMode(void);
float32_t dspAlgEqCoeffW2Dsp[2][14][5];
float32_t dspAlgSpkEqCoeffW2Dsp[7][5];


uint8_t filterSpkRefreshEn = 0;
uint8_t filterDacRefreshEn = 0;
uint8_t filterSpkRefreshEn2 = 0;             //Garry

extern float spkFilterCoeff[5 *SPK_FILTER_NUM];
extern float spkFilterCoeffSet[5 * SPK_FILTER_NUM];

extern float spkFilterCoeff2[5 *SPK_FILTER_NUM];
extern float spkFilterCoeffSet2[5 * SPK_FILTER_NUM];

extern float dacFilterCoeff[5 * DACOUT_FILTER_NUM];
extern float dacFilterCoeffSet[5 * DACOUT_FILTER_NUM];

void DrvAlgUserParaInit(uint32_t idx, uint16_t *pPara)
{
    if (idx == 0)
    {   // adc eq para
      
        for (uint32_t i = 0; i < 14; i++)
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
		
		if (filterSpkRefreshEn2 != 0)
    {
        memcpy(&spkFilterCoeff2, &spkFilterCoeffSet2, sizeof(spkFilterCoeffSet2));

        filterSpkRefreshEn2 = 0;
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
		
		//Garry
    if(EqIdx == 2)
    {   //spk eq
        for (uint32_t i = 0; i < segNumMax; i++)
        {
            AlgFilterDesign(pPara, &spkFilterCoeffSet2[5 * i], ALG_SAMPLE_RATE);
            pPara++;
        }
        filterSpkRefreshEn2 = 1;
    }
		
    return;
}



// 设置dsp算法增益刷新id设置
void DspAlgGainRefreshIdxSet(uint8_t idx)
{
    dspAlgGainRefershIdx = idx;
}

/*
这个 switch 语句根据 idx 的值选择不同的处理流程。
在这里，只有一个具体的 case，即 DRV_ALG_IDX_EQ，
它表示均衡器（Equalizer，EQ）相关的算法设置。
DRV_ALG_IDX_EQ：当 idx 等于 DRV_ALG_IDX_EQ 时，
会执行以下两行代码，设置DSP和MCU的均衡器参数

DspAlgEqParaSet：此函数用于设置DSP（数字信号处理器）部分的均衡器参数，
参数 pPara[1] 指向均衡器的具体参数（例如滤波器系数），5 可能表示滤波
器的数量或长度，pPara[0] 作为某种标识传递（可能是均衡器的模式或类型）。

McuAlgEqParaSet：此函数用于设置MCU（微控制器）部分的均衡器参数，
参数与 DspAlgEqParaSet 相同，可能是为了确保MCU和DSP的均衡器设置一致。
*/
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



