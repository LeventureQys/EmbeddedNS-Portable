
#include "conf_drv_en.h"
#ifdef DRV_DUAL_TIMER_ENABLE

#include "a0_hal_conf.h"
#include "a0_hal_def.h"
#include "drv_dual_timer.h"
/* Private typedef -----------------------------------------------------------*/
typedef struct {
    
    FUNCP       DualfuncTimerIrq[2];  
    uint32_t    DualtimerState[2];
    
}HAL_DUAL_TIM_Var_TypeDef;

/* Private function prototypes -----------------------------------------------*/
static void HalDualTimerCallbackNull(void)
{
    return;
}
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static HAL_DUAL_TIM_Var_TypeDef  halDualTimerVar = {
    
    .DualfuncTimerIrq={ 
             HalDualTimerCallbackNull,
             HalDualTimerCallbackNull,    
    },  
};



/** @addtogroup DUAL_TIMER
  * @{
  */ 
/** @defgroup DUAL_TIMER_Exported_Functions DUAL TIMER Exported Functions
  * @{
  */
/* Exported functions --------------------------------------------------------*/

__RAM_CODE void HalDualTimerIrqHandler(uint32_t timerIdx)
{
    halDualTimerVar.DualfuncTimerIrq[timerIdx]();
    return;
}

//return    >= 0,   state value, 0, timer idle, 1, timer in use
//          -1,     timerIdx out of range

int32_t HalDualTimerStateGet(uint32_t timerIdx)
{
    if (timerIdx < 2)
    {
        return (halDualTimerVar.DualtimerState[timerIdx]);
    }
    else
    {
        return -1;
    }
}

//The maximum time of 96 PCLK divided by 256 is 11263s
//return    0,  ok
//          -1, timerIdx out of range
//          -2, cb is null
//          -3  time out of range
int32_t HalDualTimerInit(Dual_TIM_INIT_TypeDef *pInit)
{
     if (pInit->timerIdx < 2)
     {
          DTIM_CR_TypeDef ctrlReg = {
            .b = {
                .intEn = 1,
                .enable = 1,
                .oneShotCount=0, //0, Wrapping mode(default) 
                .timerSize=1,// 1, 32-bit counter
                .timerPre= 0,  //0, div1(default)
                .timerMode=1,   // 1,periodic mode
            },
        };
         DTIM_TypeDef *DTIMx=DTIM0;
         if(pInit->timerIdx==0)
         {
           DTIMx = DTIM0;         
         } 
        else if(pInit->timerIdx==1) 
        {
            DTIMx = DTIM1;   
        }               
        HAL_DTIM_INT_CLR(DTIMx);
        NVIC_DisableIRQ(TIMER_DUAL_IRQn);
        if (pInit->cb != NULL)
        {
            halDualTimerVar.DualfuncTimerIrq[pInit->timerIdx] = pInit->cb;
            halDualTimerVar.DualtimerState[pInit->timerIdx] = 1;
            if(pInit->time<44000)  //时间在此范围内不分频
            {
                ctrlReg.b.timerPre=0;
                uint32_t reloadValue = (uint32_t)(((float_t)SystemPclk / 1000.0f) * pInit->time + 0.5f) - 1;    /* 和SysTick计数进行对比测试，发现需要-1才准，具体在芯片上主频上去之后再测试 */
                HAL_DTIM_LOAD_SET(DTIMx, reloadValue);    //当前计数值立即改变
             //   HAL_DTIM_BG_LOAD_SET(DTIMx, reloadValue); //当前计数值不会立即会改变  
              
            }
            else if(pInit->time>44000 && pInit->time<704000)  //时间在此范围内16分频
            {
             ctrlReg.b.timerPre=1;
             uint32_t clock=(SystemPclk/16);
             uint32_t reloadValue = (uint32_t)(((float_t)clock / 1000.0f) * pInit->time + 0.5f) - 1;    /* 和SysTick计数进行对比测试，发现需要-1才准，具体在芯片上主频上去之后再测试 */
             HAL_DTIM_LOAD_SET(DTIMx, reloadValue);  //当前计数值立即改变
           //  HAL_DTIM_BG_LOAD_SET(DTIMx, reloadValue);   //当前计数值不会立即会改变  

            }
            else if(pInit->time>704000 && pInit->time< 11263000)  //时间在此范围内256分频
            {
             ctrlReg.b.timerPre=2;
             uint32_t clock=(SystemPclk/256);
             uint32_t reloadValue = (uint32_t)(((float_t)clock / 1000.0f) * pInit->time + 0.5f) - 1;    /* 和SysTick计数进行对比测试，发现需要-1才准，具体在芯片上主频上去之后再测试 */
             HAL_DTIM_LOAD_SET(DTIMx, reloadValue);   //当前计数值立即改变
           //  HAL_DTIM_BG_LOAD_SET(DTIMx, reloadValue);  //当前计数值不会立即会改变  
                  
            }
            else 
            {
                return -3;            
            }
           
            NVIC_EnableIRQ(TIMER_DUAL_IRQn);           
            HAL_DTIM_CTRL_SET(DTIMx, ctrlReg.w);
            return 0;     
        }
        
        else
        {
            return -2;
        }
     
     }
     else 
     {
        return -1;
     }
}

/****************************************************************************/
/* Name         : DualTimerStart                                             */
/* Input        : timerIdx, timer index numebr, 0~1                         */
/* Output       : nothing                                                   */
/* Discription  : start timer                                               */
/****************************************************************************/
void DualTimerStart(uint32_t timerIdx)
{

    DTIM_CR_TypeDef ctrlReg;
    if(ctrlReg.b.enable==0)
    {
        ctrlReg.b.intEn=1;
        ctrlReg.b.enable=1;
        NVIC_EnableIRQ(TIMER_DUAL_IRQn);
        if(timerIdx==0)
        {
            DTIM_TypeDef *DTIMx = DTIM0;                  
            HAL_DTIM_CTRL_SET(DTIMx, ctrlReg.w);
        }
        if(timerIdx==1)
        {
            DTIM_TypeDef *DTIMx = DTIM1;
            HAL_DTIM_CTRL_SET(DTIMx, ctrlReg.w);
        }
    }

}

/****************************************************************************/
/* Name         : DualTimerStop                                             */
/* Input        : timerIdx, timer index numebr, 0~1                         */
/* Output       : nothing                                                   */
/* Discription  : stop timer                                                */
/****************************************************************************/
void DualTimerStop(uint32_t timerIdx)
{
     DTIM_CR_TypeDef ctrlReg;
    if(ctrlReg.b.enable==1)
    {
        ctrlReg.b.enable=0;
        NVIC_DisableIRQ(TIMER_DUAL_IRQn);       
        if(timerIdx==0)
        {
            DTIM_TypeDef *DTIMx = DTIM0;                  
            HAL_DTIM_CTRL_SET(DTIMx, ctrlReg.w);
        }
        if(timerIdx==1)
        {
            DTIM_TypeDef *DTIMx = DTIM1;
            HAL_DTIM_CTRL_SET(DTIMx, ctrlReg.w);
        }
    }
}

/****************************************************************************/
/* Name         : DualTimerLoadSet                                          */
/* Input        : timerIdx, timer index numebr, 0~1                         */
/*              : setValue, current count value write to load register      */
/* Output       : 0,  ok ,   -1 ,timerIdx out of range                      */
/* Discription  : Value write to load register                              */
/****************************************************************************/
uint32_t DualTimerLoadSet(uint32_t timerIdx, uint32_t setValue)
{
    if(timerIdx==0)
    {
        DTIM_TypeDef *DTIMx = DTIM0;         
        HAL_DTIM_LOAD_SET(DTIMx, setValue); 
        return 0;
    }
    else if(timerIdx==1)
    {
        DTIM_TypeDef *DTIMx = DTIM1;         
        HAL_DTIM_LOAD_SET(DTIMx, setValue); 
        return 0;
    }
    else
    {
        return 1;
    }
}

/****************************************************************************/
/* Name         : DualTimerBGLoadSet                                        */
/* Input        : timerIdx, timer index numebr, 0~1                         */
/*              : setValue, current count value write to background load register  */
/* Output       : 0,  ok  ,  -1 ,timerIdx out of range                      */
/* Discription  : Value write to background load register                   */
/****************************************************************************/
int32_t DualTimerBGLoadSet(uint32_t timerIdx, uint32_t setValue)
{
    if(timerIdx==0)
    {
       DTIM_TypeDef *DTIMx = DTIM0;         
       HAL_DTIM_BG_LOAD_SET(DTIMx, setValue); 
       return 0;
    }
    else if(timerIdx==1)
    {
        DTIM_TypeDef *DTIMx = DTIM1;         
        HAL_DTIM_BG_LOAD_SET(DTIMx, setValue);  
        return 0;
    }
    else
    {
        return -1;
    }
}

/****************************************************************************/
/* Name         : DualTimerLoadGet                                          */
/* Input        : timerIdx, timer index numebr, 0~1                         */
/* Output       : Value,  Value read from load register                     */
/* Discription  :Value read from load register                              */
/****************************************************************************/
uint32_t DualTimerLoadGet(uint32_t timerIdx)
{
    uint32_t Value;
    if(timerIdx==0)
    {
        DTIM_TypeDef *DTIMx = DTIM0;         
        Value = HAL_DTIM_LOAD_GET(DTIMx);  
    }
    if(timerIdx==1)
    {
        DTIM_TypeDef *DTIMx = DTIM1;         
        Value = HAL_DTIM_LOAD_GET(DTIMx);  
    }
    
    return Value;
}

/****************************************************************************/
/* Name         : DualTimerBGLoadGet                                        */
/* Input        : timerIdx, timer index numebr, 0~1                         */
/* Output       : Value, Value read from background load register           */
/* Discription  :Value read from background load register                   */
/****************************************************************************/

uint32_t DualTimerBGLoadGet(uint32_t timerIdx)
{
    uint32_t Value;
    if(timerIdx==0)
    {
        DTIM_TypeDef *DTIMx = DTIM0;         
        Value = HAL_DTIM_BG_LOAD_GET(DTIMx);  
    }
    if(timerIdx==1)
    {
        DTIM_TypeDef *DTIMx = DTIM1;         
        Value = HAL_DTIM_BG_LOAD_GET(DTIMx);  
    }
    return Value;
}


//return    0,  ok
//          -1, timerIdx out of range
int32_t HalDualTimerDeinit(uint32_t timerIdx)
{
	int32_t result;
    if(timerIdx<2)
    {
        if (timerIdx==0)
        {
            DTIM_TypeDef *DTIMx = DTIM0;
            //NVIC_DisableIRQ(TIMER_DUAL_IRQn);
            HAL_DTIM_LOAD_SET(DTIMx, 0); //加载寄存器设置为0
            halDualTimerVar.DualfuncTimerIrq[timerIdx] = HalDualTimerCallbackNull;
            halDualTimerVar.DualtimerState[timerIdx] = 0;
            result = 0;
        }
        else if(timerIdx==1)
        {
           DTIM_TypeDef *DTIMx = DTIM1;
          // NVIC_DisableIRQ(TIMER_DUAL_IRQn);
           HAL_DTIM_LOAD_SET(DTIMx, 0); //加载寄存器设置为0
           halDualTimerVar.DualfuncTimerIrq[timerIdx] = HalDualTimerCallbackNull;
           halDualTimerVar.DualtimerState[timerIdx] = 0;
           result = 0;
        }        
    }
    else
    {
		result = -1;
    }   
	return result;	
}

/**
  * @}
  */
/**
  * @}
  */
  
#endif
