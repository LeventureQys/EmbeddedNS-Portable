/**
  ******************************************************************************
  * @file    drv_pwm.h
  * @brief   Header file of PWM DRV module.
  * @version V1.0.0
  * @date    2022/04/16
  * @author  lijixuan
  ******************************************************************************
  */
#include "conf_drv_en.h"
#ifdef DRV_PWM_ENABLED

/* Includes ------------------------------------------------------------------*/
#include "drv_pwm.h"

/** @addtogroup PWM_Drive_Driver
  * @{
  */

int32_t DrvPwmInit(PWM_MODE_PARA_TypeDef* pInit)
{
    if(pInit->mode == 0)
    {
        HalPwmConfigMode0(pInit->chIdx,(void*)&pInit->modePara);
        uint32_t cr = 0;
        uint32_t icr = 0;
        uint32_t inverseEn = pInit->inverseEn & 0x1;
        uint32_t channel = (1 << pInit->chIdx);
        cr |= (channel << PWM_CR_START_POS);
        cr |= ((channel * inverseEn) << PWM_CR_OUTINV_POS);
        icr |= (channel << PWM_ICR_INTEN_POS);           //default enable interrupt
        PWM_InitTypeDef pVarInit;
        pVarInit.baseCfg.CR = cr;
        pVarInit.baseCfg.ICR = icr;
        pVarInit.handleNumber = 16;
        HalPwmInit(&pVarInit);
        return 0;
    }
    if(pInit->mode == 1)
    {
        HalPwmConfigMode1(pInit->chIdx,(void*)&pInit->modePara);
        uint32_t cr = 0;
        uint32_t icr = 0;
        uint32_t inverseEn = pInit->inverseEn & 0x1;
        uint32_t channel = (1 << pInit->chIdx);
        cr |= (channel << PWM_CR_START_POS);
        cr |= ((channel * inverseEn) << PWM_CR_OUTINV_POS);
        icr |= (channel << PWM_ICR_INTEN_POS);           //default enable interrupt
        PWM_InitTypeDef pVarInit;
        pVarInit.baseCfg.CR = cr;
        pVarInit.baseCfg.ICR = icr;
        pVarInit.handleNumber = 16;
        HalPwmInit(&pVarInit);
        return 0;
    }
    if(pInit->mode == 2)
    {
        HalPwmConfigMode2(pInit->chIdx,(void*)&pInit->modePara);
        uint32_t cr = 0;
        uint32_t icr = 0;
        uint32_t inverseEn = pInit->inverseEn & 0x1;
        uint32_t channel = (1 << pInit->chIdx);
        cr |= (channel << PWM_CR_START_POS);
        cr |= ((channel * inverseEn) << PWM_CR_OUTINV_POS);
        icr |= (channel << PWM_ICR_INTEN_POS);           //default enable interrupt
        PWM_InitTypeDef pVarInit;
        pVarInit.baseCfg.CR = cr;
        pVarInit.baseCfg.ICR = icr;
        pVarInit.handleNumber = 16;
        HalPwmInit(&pVarInit);
        return 0;
    }
    if(pInit->mode == 3)    
    {
        HalPwmConfigMode34(pInit->chIdx,(void*)&pInit->modePara);
        uint32_t cr = 0;
        uint32_t icr = 0;
        uint32_t inverseEn = pInit->inverseEn & 0x1;
        uint32_t channel = (1 << pInit->chIdx);
        cr |= (channel << PWM_CR_START_POS);
        cr |= ((channel * inverseEn) << PWM_CR_OUTINV_POS);
        icr |= (channel << PWM_ICR_INTEN_POS);           //default enable interrupt
        PWM_InitTypeDef pVarInit;
        pVarInit.baseCfg.CR = cr;
        pVarInit.baseCfg.ICR = icr;
        pVarInit.handleNumber = 16;
        HalPwmInit(&pVarInit);
        return 0;
    }
	return -1;
}
/** 
  * @}
  */

#endif
