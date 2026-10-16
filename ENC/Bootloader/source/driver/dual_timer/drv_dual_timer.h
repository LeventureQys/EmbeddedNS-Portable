#ifndef __DRV_DUAL_TIMER_H
#define __DRV_DUAL_TIMER_H
#include "a0_hal_def.h"
#include "a0_hal_timer.h"

/** @addtogroup DUAL_TIMER
  * @{
  */ 
/** @addtogroup DUAL_TIMER_Exported_Types DUAL TIMER Exported Types
  * @{
  */

/**
  * @brief DUAL_TIMER Configuration Structure definition
  */

typedef struct
{
    FUNCP               cb;                 /*!< callback function for timer    */
    float_t             time;               /*!< interrupt time in ms           */
    uint32_t            timerIdx;           /*!< timer index numebr, 0~1       */
}Dual_TIM_INIT_TypeDef;


/* Private macros --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @addtogroup DUAL_TIMER_Exported_Functions DUAL TIMER Exported Functions
  * @{
  */

/** @brief  dual timer irq handler function
  * @param  timerIdx    0~1, index number of dual timer
  * @return None
  */
void HalDualTimerIrqHandler(uint32_t timerIdx);

/** @brief  dual timer module initial process function
  * @param  pInit    pointer to structure of @ref Dual_TIM_INIT_TypeDef, time max is 11263 seconds at 96MHz PCLK divided by 256 
  * @retval == 0    initial ok
  * @retval == -1   initial error, timerIdx is out of range, it must be 0~1
  * @retval == -2   initial error, callback function of input parameter is null
  * @retval == -3   initial error,  time out of range,the max time is 11263s
  */
int32_t HalDualTimerInit(Dual_TIM_INIT_TypeDef *pInit);

/** @brief  timer module de-initial process function
  * @param  timerIdx    0~1, index number of timer
  * @retval == 0    de-initial ok
  * @retval == -1   de-initial error, timerIdx is out of range, it must be 0~1
  */
int32_t HalDualTimerDeinit(uint32_t timerIdx);

/** @brief  timer module state get
  * @param  timerIdx    0~1, index number of timer
  * @retval == 0    state in idle, timer of the input tiemrIdx is idle, can be initialled to use
  * @retval == 1    state in use, timer of the input timerIdx is in use, it can not be initialled to use, but can be de-initialled to release
  * @retval == -1,  error, timerIdx is out of range, it must be 0~1
  */
int32_t HalDualTimerStateGet(uint32_t timerIdx);

void DualTimerStart(uint32_t timerIdx);
void DualTimerStop(uint32_t timerIdx);
uint32_t DualTimerLoadSet(uint32_t timerIdx, uint32_t setValue);
int32_t DualTimerBGLoadSet(uint32_t timerIdx, uint32_t setValue);
uint32_t DualTimerLoadGet(uint32_t timerIdx);
uint32_t DualTimerBGLoadGet(uint32_t timerIdx);




/**
  * @}
  */

  
  
  
  

/**
  * @}
  */

/** 
  * @}
  */















#endif
