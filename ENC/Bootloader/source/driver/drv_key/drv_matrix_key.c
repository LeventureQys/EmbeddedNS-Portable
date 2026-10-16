/**
  ******************************************************************************
  * @file    drv_matrix_key.c
  * @brief   matrix key module.
  * @version V1.0.0
  * @date    2021/09/27
  * @author  ziyingli
  ******************************************************************************
  */


#ifndef __DRV_MATRIX_KEY_C__
#define __DRV_MATRIX_KEY_C__


/* Includes ------------------------------------------------------------------*/
#include "drv_matrix_key.h"

/* Exported constants --------------------------------------------------------*/
#define  KEY_IN    (BIT3|BIT4|BIT5)
#define IO_KEY_PUPD                     (2)         //0, IO无需上下拉，1，内部上拉，2，内部下拉   
#define GPIO_IN_GET(port, pin)				(HAL_GPIO_IN_DATA_GET(port)&pin)

/* Exported types ------------------------------------------------------------*/
 uint32_t keyVal[KEY_COL_NUM][KEY_ROW_NUM]={

   //col0  列  输入
    1,           //row0  行     输出
    2,           //row1  
    3,           //row2  
    
   //col1  
    4,           //row0  
    5,           //row1  
    6,           //row2  
    
   //col2  
    7,          //row0 
    8,          //row1 
    9,          //row2  

};


 KeypadGpioStruct keypadio = { 

	{{GPIO0, 6}, {GPIO0, 1}, {GPIO0, 2}},  	//rowNum[KEY_ROW_NUM] 行 输出  6
	{{GPIO0, 3}, {GPIO0, 4}, {GPIO0, 5}}    //colNum[KEY_COL_NUM] 列 输入345
};
 


/* Exported functions --------------------------------------------------------*/

/** @brief Initialize the rows and columns of the matrix keys
  * @param  None
  * @return None
*/
//矩阵按键初始化
void drv_matrix_key_Init(void)
{
    //uint32_t  pstaic[KEY_COL_NUM];
    uint8_t irow,icol;
    //行  输出
    for(irow=0;irow<KEY_ROW_NUM;irow++)
    {
        GPIO_TypeDef* RowGPIOx = keypadio.rowNum[irow].GPIOx;
        uint32_t Rowpin = keypadio.rowNum[irow].GPIO_Pin;      
        HAL_GPIO_AF_SET(RowGPIOx, Rowpin, GPIO_AF0);
        
        HAL_GPIO_OUT_EN(RowGPIOx, (1 << Rowpin));
        HAL_GPIO_BIT_CLR(RowGPIOx, (1 << Rowpin)); //初始化拉低
    }
    //列  输入
     uint32_t keyIn = KEY_IN;
    for(icol = 0; icol < KEY_COL_NUM; icol++)  
    {
        GPIO_TypeDef* ColGPIOx = keypadio.colNum[icol].GPIOx;
        uint32_t Colpin = keypadio.colNum[icol].GPIO_Pin;      
        HAL_GPIO_AF_SET(ColGPIOx, Colpin, GPIO_AF0);
        HAL_GPIO_IN_EN(ColGPIOx, (1 << Colpin));
   
        for (uint8_t i = 0; i < 32; i++)
        {
            if ((keyIn & BIT0) != 0)
            {
                #if (IO_KEY_PUPD == 1)
                //上拉
                    HAL_GPIO_PUPD_SET(GPIO0, i, GPIO_PULLUP);
                
                #elif (IO_KEY_PUPD == 2)
                //下拉
                    HAL_GPIO_PUPD_SET(GPIO0, i, GPIO_PULLDOWN);
                #else
                //无上下拉
                    HAL_GPIO_PUPD_SET(GPIO0, i, GPIO_NOPULL);
                #endif                    
            }
            keyIn >>= 1;
        }
          
    }  
}


  
/** @brief Row column scanning of matrix keys
  * @param  None
  * @return The value of the pressed key
*/
//矩阵按键扫描,返回按键按下的行列数
uint32_t drv_matrix_key_scan(void)
{  
    uint32_t key_num=0;
    uint8_t icol, irow;
   uint32_t   val_col[KEY_COL_NUM] = {0};//
   for(irow = 0; irow < KEY_ROW_NUM; irow++)
   {       
     HAL_GPIO_BIT_SET(keypadio.rowNum[irow].GPIOx,(1<<keypadio.rowNum[irow].GPIO_Pin));  //行线拉高
       for(icol = 0; icol < KEY_COL_NUM; icol++) 
       {
         HAL_TickDelay(1); 
         val_col[icol]=GPIO_IN_GET(keypadio.colNum[icol].GPIOx,(1<<keypadio.colNum[icol].GPIO_Pin));   //读列线               
         if( val_col[icol]!=0)  
         {           
            key_num = keyVal[icol][irow];	//返回行列数      
             
            break;          
         }
        }
        HAL_GPIO_BIT_CLR(keypadio.rowNum[irow].GPIOx,(1<<keypadio.rowNum[irow].GPIO_Pin)); //行线拉低
    }
	return key_num;	
}



#endif
