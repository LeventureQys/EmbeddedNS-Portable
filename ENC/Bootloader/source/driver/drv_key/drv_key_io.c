
/**
  ******************************************************************************
  * @file    drv_key_io.c
  * @brief   Independent key module.
  * @version V1.0.0
  * @date    2021/09/20
  * @author  ziyingli
  ******************************************************************************
  */
#ifndef __DRV_KEY_IO_C__
#define __DRV_KEY_IO_C__

#include "conf_drv_en.h"
#ifdef DRV_KEY_IO_ENABLED

#include "drv_key_io.h"

/* Exported constants --------------------------------------------------------*/   
#define SCAN_CYCLE                                      (20)        //20ms
#define STATE_SCAN_CNT                                  (2)
#define STATE_PRESSED_CNT                               (3)
GPIO_TypeDef*  GpioPort[2]={GPIO0,GPIO1};
uint8_t x;
uint8_t y;

/* Exported functions --------------------------------------------------------*/
 /** @brief Key initialization function
  * @param  pPara Pointer to KEY_IO_PARA_TypeDef  structure 
  * @return None
*/
void DrvKeyIoInit(KEY_IO_PARA_TypeDef *pPara)
{
    uint32_t ioMask;
   
    for (uint8_t i = 0; i < IO_PORT_MAX; i++)
    {
        ioMask = 0;
        for (uint8_t j = 0; j < pPara->ioNum[i]; j++)
        {
            ioMask |= (1 << pPara->pIoInfo[i][j]);
            #if (IO_KEY_PUPD == 1)
            //上拉
                    HAL_GPIO_PUPD_SET(GpioPort[i],pPara->pIoInfo[i][j],GPIO_PULLUP);
            #elif (IO_KEY_PUPD == 2)
            //下拉
                     HAL_GPIO_PUPD_SET(GpioPort[i],pPara->pIoInfo[i][j],GPIO_PULLDOWN);
            #else
            //无上下拉
                     HAL_GPIO_PUPD_SET(GpioPort[i],pPara->pIoInfo[i][j],GPIO_NOPULL);
            #endif        
        }
        pPara->ioMask[i] = ioMask; 
        HAL_GPIO_AF_SET(GpioPort[i],ioMask,GPIO_AF0);
        GPIO_DIR_IN(GpioPort[i],ioMask);  
        pPara->state[i] = GPIO_IN_GET(GpioPort[i], ioMask);
    }

    pPara->state[STATE_SCAN_CNT] = HAL_TickGet();
    pPara->state[STATE_PRESSED_CNT] = 0;
}

 /** @brief Key scanning and processing function
  * @param pPara Pointer to KEY_IO_PARA_TypeDef  structure 
  * @return None
*/
void DrvKeyIoProcess(KEY_IO_PARA_TypeDef *pPara)
{
    uint32_t curMs = HAL_TickGet();
    if ((curMs - pPara->state[STATE_SCAN_CNT]) > SCAN_CYCLE)
    {
        pPara->state[STATE_SCAN_CNT] = curMs;       
        for (uint8_t i = 0; i < IO_PORT_MAX; i++)  //i->port
        {
            if (pPara->ioNum[i] != 0)
            {
                uint32_t ioState = GPIO_IN_GET(GpioPort[i], pPara->ioMask[i]);
                uint32_t ioStateTemp = ioState ^ pPara->state[i];
                
                pPara->state[i] = ioState;
                /* 逐个判断每个IO */
                for (uint8_t j = 0; j < pPara->ioNum[i]; j++)   //j->pin
                {
                    if ((ioStateTemp & (1 << pPara->pIoInfo[i][j])) != 0)  
                    { 
                        /* IO状态变化 */
#if (IO_KEY_ON_STATE == 0)                    
                        if ((ioState & (1 << pPara->pIoInfo[i][j])) == 0)  //按下低电平
#else
                        if ((ioState & (1 << pPara->pIoInfo[i][j])) != 0  // 按下高电平
#endif                    
                        {
                            /* 当前状态：释放->按下 */
                            pPara->state[STATE_PRESSED_CNT]++; 
                            x = i;
                            y = j;                             
                        }
                        /* 释放 */
                        else
                        {
                            #if 1 /*new verbsion*/
                            if(pPara->state[STATE_PRESSED_CNT] <= ((pPara->keyPressedInMs/100) - 1))
                            {
                                pPara->pKeyOnCb[x][y]();
								pPara->pKeyOffCb[x][y](); 								
                            }
                            pPara->state[STATE_PRESSED_CNT] = 0;
//                            pPara->pKeyOffCb[x][y]();  
                            #else  ////old verbsion                       
                            pPara->state[STATE_PRESSED_CNT] = 0;
                            pPara->pKeyOffCb[x][y]();
                            #endif
                        }
                    }
                    else
                    {
                        #if 1  //new verbsion
                        if (ioState  !=  pPara->ioMask[x]) 
                        {                       
                            pPara->state[STATE_PRESSED_CNT]++;
                        }
                        if((pPara->state[STATE_PRESSED_CNT] * SCAN_CYCLE) == pPara->keyPressedInMs)
                        {                            
                            pPara->pKeyPressedCb[x][y]();                          
                        }
                        #else  ////old verbsion
                        if (pPara->state[STATE_PRESSED_CNT] != 0)
                        {
                            /* 已经检测到按下过 */
                            pPara->state[STATE_PRESSED_CNT]++;
                            if (pPara->keyPressedInMs != 0) 
                            {
                                /*长按*/
                                if ((pPara->state[STATE_PRESSED_CNT] * SCAN_CYCLE) == pPara->keyPressedInMs)
                                {
                                    pPara->pKeyPressedCb[x][y]();
                                }
                                  /*短按*/
                                else if (pPara->state[STATE_PRESSED_CNT] == 2)
                                {
                                    pPara->pKeyOnCb[x][y]();
                                }
                            }
                        }
                        #endif
                    }
                }                
            }
        }
    }
    return;    
}
#endif

#endif
