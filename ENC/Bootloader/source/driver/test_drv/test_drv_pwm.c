#ifndef __TEST_DRV_PWM_C_
#define __TEST_DRV_PWM_C_


/**
  ******************************************************************************
  * @file    demo_pwm.c
  * @brief   demo_pwm.
  * @version V1.0.0
  * @date    2022/04/16
  * @author  lijixuan 
  ******************************************************************************
  */

/**
  * 版本日志：
  * V1.0.0, 2022/04/16
  
  */

/* Includes ------------------------------------------------------------------*/
#include "drv_pwm.h"
#include "test_drv_pwm.h"

/** @addtogroup PWM_DEMO
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @defgroup PWM Init Typdef
  * @{
  */
PWM_MODE_PARA_TypeDef pPwmMode0Init;
PWM_MODE_PARA_TypeDef pPwmMode1Init;
PWM_MODE_PARA_TypeDef pPwmMode2Init;
PWM_MODE_PARA_TypeDef pPwmMode34Init;
/** 
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup PWM_Demo_Exported_Functions
  * @{
  */
  
 /**
  * @brief  pwm模式0；模式1；模式2；模式3应用例程；可以更改头文件配置使能相应的模式例程
  * @retval none      
  */
 /*配置PWM测试模式*/ 
#define   PWM_MODE          (0)     /* 0:测试模式0  1:测试模式1 2:测试模式2   3:测试模式34 */
 
void TestDrvPwm(void)
{
    /*配置GPIO时钟*/
    HAL_GPIO_AF_SET(GPIO0, 1, GPIO_AF5_PWM10); 

    #if( PWM_MODE == 0)
    /*模式0初始化*/
    PWM_MODE_PARA_TypeDef*  pMode0 = &pPwmMode0Init;;
    pMode0->mode = 0;         /*配置模式*/
    pMode0->chIdx = 10;       /*配置通道*/
    pMode0->inverseEn = 0;    /*PWM是否需要翻转*/
    pMode0->modePara.paraStandard.baseFreq = 1000;       /*配置基频  1s 1000个周期*/
    pMode0->modePara.paraStandard.dutyRatio = 0.5f;      /*占空比50*/
    DrvPwmInit(pMode0);      /*传入初始化的参数*/
    #endif

    #if( PWM_MODE == 1)
    /*模式1初始化*/
    PWM_MODE_PARA_TypeDef*  pMode1 = &pPwmMode1Init;;
    pMode1->mode = 1;         /*配置模式*/
    pMode1->chIdx = 10;       /*配置通道*/
    pMode1->inverseEn = 0;    /*PWM是否需要翻转*/
    pMode1->modePara.paraBreatheAuto.baseFreq = 50;          /*配置基频  配置基频  1s 50个周期*/
    pMode1->modePara.paraBreatheAuto.breatheFadeInTime = 6000; /*淡出时间,最大占空比到最小占空比时间*/
    pMode1->modePara.paraBreatheAuto.breatheFadeOutTime = 6000; /*淡入时间，最小占空比到最大占空比时间*/
    pMode1->modePara.paraBreatheAuto.dutyRatioMax = 1.0f;        /*最大占空比*/
    pMode1->modePara.paraBreatheAuto.dutyRatioMin = 0.0f;        /*最小占空比*/
    DrvPwmInit(pMode1);      /*传入初始化的参数*/
    #endif
    
    #if( PWM_MODE == 2)
    /*模式2初始化(到达最大最小占空比时会触发中断)*/
    PWM_MODE_PARA_TypeDef*  pMode2 = &pPwmMode2Init;;
    pMode2->mode = 2;         /*配置模式*/
    pMode2->chIdx = 10;       /*配置通道*/
    pMode2->inverseEn = 0;    /*PWM是否需要翻转*/
    pMode2->modePara.paraBreatheOnce.baseFreq = 500;          /*配置基频  配置基频  1s 50个周期*/
    pMode2->modePara.paraBreatheOnce.breatheFadeInTime = 6000; /*淡出时间,最大占空比到最小占空比时间*/
    pMode2->modePara.paraBreatheOnce.breatheFadeOutTime = 6000; /*淡入时间，最小占空比到最大占空比时间*/
    pMode2->modePara.paraBreatheOnce.dutyRatioMax = 0.9f;        /*最大占空比*/
    pMode2->modePara.paraBreatheOnce.dutyRatioMin = 0.1f;        /*最小占空比*/
    DrvPwmInit(pMode2);      /*传入初始化的参数*/
    #endif
    
    #if( PWM_MODE == 3)
    PWM_MODE_PARA_TypeDef*  pMode34 = &pPwmMode34Init;;
    pMode34->mode = 3;         /*配置模式*/
    pMode34->chIdx = 10;       /*配置通道*/
    pMode34->inverseEn = 0;    /*PWM是否需要翻转*/
    pMode34->modePara.paraCount.modeSel34 = 1;
    pMode34->modePara.paraCount.pulseWidthMax = 1000;
    pMode34->modePara.paraCount.pulseWidthMin = 500;
    DrvPwmInit(pMode34);      /*传入初始化的参数*/  
    #endif 
    while(1);    
}

 /**
  * @brief  把想要执行的中断函数写在这里
  * @retval none      
  */
void TestDrvPwmIrq(void)
{
    return;
}


 /**
  * @brief  pwm中断入口函数，添加想要执行的中断函数
  * @retval none      
  */
void PWM_IRQHandler(void)
{
    TestDrvPwmIrq();
    return;    
}

/** 
  * @}
  */

/** 
  * @}
  */

#endif
