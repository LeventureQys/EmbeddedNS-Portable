#ifndef __DRV_KEY_ADC_H__
#define __DRV_KEY_ADC_H__

#ifdef __cplusplus
extern "C" {
#endif
    
    
/* Includes ------------------------------------------------------------------*/
#include "stdint.h"    
#include "a0_hal_gpio.h"  
#include "a0_hal_adc.h"

/* Exported types ------------------------------------------------------------*/   
typedef struct {
    uint32_t    adcCh;
    uint8_t   	keyNum;                 //键值数，无任何按键按下也算一个键值，e.g. 3个按键的时候，键值数为4
    uint8_t     state;                  //用于检测记录状态
    uint16_t    *pKeyRangeTb;           //指向的表格值为对应按键判定范围的最大值，e.g. vel < pKeyRangeTb[0] 命中键值0
    FUNCP       *pCb;                   //指向Callback函数的指针
}KEY_ADC_PARA_TypeDef;


typedef struct {
    uint32_t     adcCh;
    
}ADC_PARA_TypeDef;

/* Exported functions --------------------------------------------------------*/
//处理检测到的adc数据
uint16_t DrvAdcProcess(ADC_PARA_TypeDef *pPara);
void DrvKeyAdcProcess(KEY_ADC_PARA_TypeDef *pPara);
void DrvAdcCore(void);

#ifdef __cplusplus
}
#endif

#endif
