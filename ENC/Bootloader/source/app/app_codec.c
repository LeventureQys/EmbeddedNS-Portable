

#include "app_codec.h"
#include "app_i2s.h"

#include "a0_hal_i2s.h"
#include "a0_hal_otg.h"
#include "a0_hal_gpio.h"
#include "a0_hal_sys_ctrl.h"

//#define AP5868

/* Private define ------------------------------------------------------------*/
/* Private type --------------------------------------------------------------*/
/* Private function declarations ---------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void AppCodecDmicInit(void)
{
    SYS_CODEC_TypeDef       codecCtrl;
    codecCtrl.w = HAL_SYS_CODEC_CTRL_GET();
    codecCtrl.b.dMic0En = 1;
    codecCtrl.b.dMic1En = 1;
    HAL_SYS_CODEC_CTRL_SET(codecCtrl.w);
    /* ≈‰÷√DMICµƒIO∏¥”√ */
    HAL_GPIO_AF_SET(GPIO0, 28, GPIO_AF3_MIC_CLK);
    HAL_GPIO_AF_SET(GPIO1, 8, GPIO_AF4_MIC_DIN1);
    HAL_GPIO_AF_SET(GPIO1, 9, GPIO_AF4_MIC_DIN0);
    return;
}

// dBVolL, -27~36, -127 means no change
// dBVolR, -27~36, -127 means no change
void AppCodecMicVolCfg(int8_t dBVolL, int8_t dBVolR)
{
    SYS_CODEC_ADC0_TypeDef  codecAdc0;
    SYS_CODEC_ADC1_TypeDef  codecAdc1;
    codecAdc0.w = HAL_SYS_CODEC_ADC0_GET();
    codecAdc1.w = HAL_SYS_CODEC_ADC1_GET();
    if (dBVolL != -127)
    {
        codecAdc0.b.lInVol = 0x1B + dBVolL;
    }
    if (dBVolR != -127)
    {
        codecAdc1.b.rInVol = 0x1B + dBVolR;
    }
    HAL_SYS_CODEC_ADC_SET(codecAdc0.w, codecAdc1.w);
    return;
}

void AppCodecMicBoostCfg(uint8_t boostEn)
{
    SYS_CODEC_ADC0_TypeDef  codecAdc0;
    SYS_CODEC_ADC1_TypeDef  codecAdc1;
    codecAdc0.w = HAL_SYS_CODEC_ADC0_GET();
    codecAdc1.w = HAL_SYS_CODEC_ADC1_GET();
    codecAdc0.b.lMicBoost = boostEn;
    codecAdc1.b.rMicBoost = boostEn;
    HAL_SYS_CODEC_ADC_SET(codecAdc0.w, codecAdc1.w);
    return;
}

void AppCodecMicPD(void)
{
    SYS_CODEC_TypeDef       codecCtrl;
    SYS_CODEC_ADC0_TypeDef  codecAdc0;
    SYS_CODEC_ADC1_TypeDef  codecAdc1;
    
    codecCtrl.w = HAL_SYS_CODEC_CTRL_GET();
    codecAdc0.w = HAL_SYS_CODEC_ADC0_GET();
    codecAdc1.w = HAL_SYS_CODEC_ADC1_GET();
    
    codecAdc0.b.lAdcPd = 1;
    codecAdc0.b.lBypassPd = 1;
    codecAdc0.b.lFEPd = 1;
    codecAdc1.b.rAdcPd = 1;
    codecAdc1.b.rBypassPd = 1;
    codecAdc1.b.rFEPd = 1;
    codecCtrl.b.micBiasPd = 1;
    
    HAL_SYS_CODEC_ADC_SET(codecAdc0.w, codecAdc1.w);
    HAL_SYS_CODEC_CTRL_SET(codecCtrl.w);
}

void AppCodecOutPD(void)
{
    SYS_CODEC_TypeDef       codecCtrl;
    SYS_CODEC_DAC_TypeDef   codecDac;
    
    codecCtrl.w = HAL_SYS_CODEC_CTRL_GET();
    codecDac.w = HAL_SYS_CODEC_DAC_GET();
    
    codecCtrl.b.vcmPd = 1;
    codecDac.b.lDacPd = 1;
    codecDac.b.lHpPd = 1;
    codecDac.b.lOutPd = 1;
    codecDac.b.rDacPd = 1;
    codecDac.b.rHpPd = 1;
    codecDac.b.rOutPd = 1;
    
    HAL_SYS_CODEC_DAC_SET(codecDac.w);
    HAL_SYS_CODEC_CTRL_SET(codecCtrl.w);
}

void AppCodecInit(void)
{
    SYS_CODEC_TypeDef       codecCtrl;
    SYS_CODEC_ADC0_TypeDef  codecAdc0;
    SYS_CODEC_ADC1_TypeDef  codecAdc1;
    SYS_CODEC_DAC_TypeDef   codecDac;
    
    codecCtrl.w = HAL_SYS_CODEC_CTRL_GET();
    codecAdc0.w = HAL_SYS_CODEC_ADC0_GET();
    codecAdc1.w = HAL_SYS_CODEC_ADC1_GET();
    codecDac.w = HAL_SYS_CODEC_DAC_GET();
    
//    codecAdc0.b.lBypassPd = 1;
//    codecAdc1.b.rBypassPd = 1;
    codecAdc0.b.lBypassSel = 0;             //hp out do not select bypass
    codecAdc1.b.rBypassSel = 0;             //hp out do not select bypass
    codecAdc0.b.lInVol = 0x1B ; //adc in 0dB
    codecAdc1.b.rInVol = 0x1B ; //adc in 0dB
//    codecAdc0.b.lInVol = 0x2F;//20dB
//    codecAdc1.b.rInVol = 0x2F;//20dB
    
    codecAdc0.b.lInSel = CODEC_SEL_LINEIN;  //CODEC_SEL_ADC0;//CODEC_SEL_ADC1;//CODEC_SEL_LINEIN;         //1~3, ADC0/ADC1/LineIn
    codecAdc1.b.rInSel = CODEC_SEL_LINEIN;  //CODEC_SEL_ADC0;//CODEC_SEL_ADC1;//CODEC_SEL_LINEIN;         //1~3, ADC0/ADC1/LineIn
    
#ifdef JAT1821
//    // 1621 use adc0
//    codecAdc0.b.lInSel = CODEC_SEL_ADC0;  //CODEC_SEL_ADC0;//CODEC_SEL_ADC1;//CODEC_SEL_LINEIN;         //1~3, ADC0/ADC1/LineIn
//    codecAdc1.b.rInSel = CODEC_SEL_ADC0;  //CODEC_SEL_ADC0;//CODEC_SEL_ADC1;//CODEC_SEL_LINEIN;         //1~3, ADC0/ADC1/LineIn
//    // mic right powerdown
//    codecAdc1.b.rAdcPd = 1;         // adc pd
//    codecAdc1.b.rBypassPd = 1;      // bypass pd
//    codecAdc1.b.rFEPd = 1;          // fe pd
//    codecAdc1.b.rInVol = 0;         // -27dB
//    codecAdc1.b.rInSel = 0;         // mute
#endif
    
    codecDac.b.r2lHpSel = 0;        //hp out do not select r2l
    codecDac.b.dacDwaEn = 1;        //enable
    codecDac.b.tDacGain = 1;        //-1.5dB
    
    codecDac.b.lHpVol = 0x39 ;       //hp out 0dB
    codecDac.b.rHpVol = 0x39;       //hp out 0dB

#ifdef AP5868
    codecDac.b.lHpPd = 1;
    codecDac.b.rHpPd = 1;
    codecCtrl.b.vcmPd = 1;
#endif

    codecCtrl.b.mclkMode = CODEC_256FS;             // 256FS, for 48K, if 32K need to config to 384FS
    codecCtrl.b.audioIfMode = CODEC_I2S_MODE;       // I2S
    codecCtrl.b.deEmphasis = CODEC_DEEMPH_OFF;      // disable emphasis
    codecCtrl.b.micBiasPd = 0;                      // bias powerdown
    
    HAL_SYS_CODEC_ADC_SET(codecAdc0.w, codecAdc1.w);
    HAL_SYS_CODEC_DAC_SET(codecDac.w);
    HAL_SYS_CODEC_CTRL_SET(codecCtrl.w);
    
    return;
}

/*****************************************************************************/
