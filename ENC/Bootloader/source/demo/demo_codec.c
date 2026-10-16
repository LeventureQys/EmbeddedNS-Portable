uint32_t testCodecPower;

//主循环中调用，用于更改codec寄存器进行测试
void codecPdTest(void)
{
    if (testCodecPower != 0)
    {
        testCodecPower = 0;
        SYS_CODEC_TypeDef       codecCtrl;
        SYS_CODEC_ADC0_TypeDef  codecAdc0;
        SYS_CODEC_ADC1_TypeDef  codecAdc1;
        SYS_CODEC_DAC_TypeDef   codecDac;
        
        codecCtrl.w = HAL_SYS_CODEC_CTRL_GET();
        codecAdc0.w = HAL_SYS_CODEC_ADC0_GET();
        codecAdc1.w = HAL_SYS_CODEC_ADC1_GET();
        codecDac.w = HAL_SYS_CODEC_DAC_GET();
        
        __breakpoint(0);
        
        HAL_SYS_CODEC_ADC_SET(codecAdc0.w, codecAdc1.w);
        HAL_SYS_CODEC_DAC_SET(codecDac.w);
        HAL_SYS_CODEC_CTRL_SET(codecCtrl.w);
    }
    return;
}

//codec初始化函数，用户根据实际应用对应修改，并将代码函数添加到工程中
void CodecInit(void)
{
    SYS_CODEC_TypeDef       codecCtrl;
    SYS_CODEC_ADC0_TypeDef  codecAdc0;
    SYS_CODEC_ADC1_TypeDef  codecAdc1;
    SYS_CODEC_DAC_TypeDef   codecDac;
    
    codecCtrl.w = HAL_SYS_CODEC_CTRL_GET();
    codecAdc0.w = HAL_SYS_CODEC_ADC0_GET();
    codecAdc1.w = HAL_SYS_CODEC_ADC1_GET();
    codecDac.w = HAL_SYS_CODEC_DAC_GET();
    
    codecAdc0.b.lInSel = CODEC_SEL_LINEIN;         //1~3, ADC0/ADC1/LineIn
    codecAdc0.b.rInSel = CODEC_SEL_LINEIN;         //1~3, ADC0/ADC1/LineIn
    codecAdc0.b.lInVol = 0x1B;      //adc in 0dB
    codecAdc0.b.rInVol = 0x1B;      //adc in 0dB
    
    codecAdc0.b.lBypassSel = 0;     //hp out do not select bypass
    codecAdc1.b.rBypassSel = 0;     //hp out do not select bypass
    codecDac.b.r2lHpSel = 0;        //hp out do not select r2l
    
    codecDac.b.lHpVol = 0x39;       //hp out 0dB
    codecDac.b.rHpVol = 0x39;       //hp out 0dB
    
    codecCtrl.b.mclkMode = CODEC_256FS;             //256FS, for 48K, if 32K need to config to 384FS
    codecCtrl.b.audioIfMode = CODEC_I2S_MODE;       //I2S
    codecCtrl.b.deEmphasis = CODEC_DEEMPH_OFF;      //disable emphasis
    
    HAL_SYS_CODEC_ADC_SET(codecAdc0.w, codecAdc1.w);
    HAL_SYS_CODEC_DAC_SET(codecDac.w);
    HAL_SYS_CODEC_CTRL_SET(codecCtrl.w);
    
    return;
}

//根据db值，配置codec的hpout增益，如有需要，添加到用户代码
void dacHpVolSet(int32_t dbVol)
{
    if ((dbVol >= -40) && (dbVol <= 0))
    {
        SYS_CODEC_DAC_TypeDef   codecDac;
        codecDac.w = HAL_SYS_CODEC_DAC_GET();
        codecDac.b.lHpVol = (57 + dbVol);   //0x11;//0x39;       //hp out 0dB
        codecDac.b.rHpVol = (57 + dbVol);   //0x11;//0x39;       //hp out 0dB
        HAL_SYS_CODEC_DAC_SET(codecDac.w);
    }
}