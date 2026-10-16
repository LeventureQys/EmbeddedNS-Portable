

uint32_t testSleep;

//main主循环中调用sleepPro()，watch窗口中，手动更改变量，testSleep=0，进入sleep模式进行测试
__RAM_CODE void demo_sleep(void)
{
    if (testSleep != 0)
    {
        testSleep = 0;
        
        HAL_ADC_PD_EN();
        
        SYS_CODEC_TypeDef       codecCtrl;
        SYS_CODEC_ADC0_TypeDef  codecAdc0;
        SYS_CODEC_ADC1_TypeDef  codecAdc1;
        SYS_CODEC_DAC_TypeDef   codecDac;
        
        codecCtrl.w = HAL_SYS_CODEC_CTRL_GET();
        codecAdc0.w = HAL_SYS_CODEC_ADC0_GET();
        codecAdc1.w = HAL_SYS_CODEC_ADC1_GET();
        codecDac.w = HAL_SYS_CODEC_DAC_GET();
        
        codecCtrl.b.micBiasPd = 1;
        codecCtrl.b.vcmPd = 1;
        codecAdc0.b.lAdcPd = 1;
        codecAdc0.b.lFEPd = 1;
        codecAdc0.b.lBypassPd = 1;
        codecAdc0.b.rAdcPd = 1;
        codecAdc0.b.rFEPd = 1;
        codecAdc1.b.rBypassPd = 1;
        codecDac.b.lDacPd = 1;
        codecDac.b.lOutPd = 1;
        codecDac.b.lHpPd = 1;
        codecDac.b.rDacPd = 1;
        codecDac.b.rOutPd = 1;
        codecDac.b.rHpPd = 1;
        
        HAL_SYS_CODEC_ADC_SET(codecAdc0.w, codecAdc1.w);
        HAL_SYS_CODEC_DAC_SET(codecDac.w);
        HAL_SYS_CODEC_CTRL_SET(codecCtrl.w);

        I2S_CR_TypeDef       cr;
        cr.w = HAL_I2S_CTRL_GET();
        cr.b.clkEn = 0;
        HAL_I2S_CTRL_SET(cr.w);

        hal_i2s_clk_select(1, 0, 0);

        SYS_CTRL->SYS_USB_CFG0 &= (~(BIT7|BIT2));
        
        SYS_CLK_TypeDef         clkCfg;
        clkCfg.w = SYS_CTRL->SYS_CLK;
        clkCfg.b.clkEnPclk = 0;
        clkCfg.b.clkEnSpi = 0;
        clkCfg.b.clkEnSdc = 0;
        clkCfg.b.clkEnDsp = 0;
        clkCfg.b.clkEnDma = 0;
        clkCfg.b.clkEnUsbPhy = 0;
        clkCfg.b.clkEnUsb = 0;
        clkCfg.b.ramEnCache = 0;
        clkCfg.b.ramEnKbd = 0;
        clkCfg.b.ramEnDsp = 0;
        clkCfg.b.ramEn02 = 0;
        clkCfg.b.ramEn1 = 0;    
        clkCfg.b.clkLowSel0 = 0;
        clkCfg.b.clkLowSel1 = 7;
        clkCfg.b.clkSysSel = 0;    //from osc
        SYS_CTRL->SYS_CLK = clkCfg.w;

        SYS_PLL_CFG0_TypeDef    pllCfg0;
        pllCfg0.w = SYS_CTRL->SYS_PLL_CFG0;
        pllCfg0.b.pllEn = 0;
        SYS_CTRL->SYS_PLL_CFG0 = pllCfg0.w;
        
        SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
        
        __WFI();                            //模拟3.3V+数字3.3V=3mA（0.5+2.5）
        
        SCB->SCR = 0;
        
        pllCfg0.b.pllEn = 1;
        SYS_CTRL->SYS_PLL_CFG0 = pllCfg0.w;
        uint32_t delay = 200;
        while(delay--);
        SYS_CTRL->SYS_CLK = 0x00FFFF02;

        hal_i2s_clk_select(0, 1, 1);
        
        SYS_CTRL->SYS_USB_CFG0 |= ( (BIT7|BIT2));
        
        cr.b.clkEn = 1;
        HAL_I2S_CTRL_SET(cr.w);
        
        codecCtrl.b.digCorePd = 0;
        codecCtrl.b.micBiasPd = 0;
        codecCtrl.b.vcmPd = 0;
        HAL_SYS_CODEC_CTRL_SET(codecCtrl.w);
    }
}
