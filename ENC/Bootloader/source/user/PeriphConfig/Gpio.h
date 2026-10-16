/*
*****************************************Copyright (c)**********************************************
**
**
**                                  深圳摩罗志远科技有限公司
**                             Shenzhen Maono Technology Co.,Ltd
**
**                                        www.maono.cn
**
**
**	文件名	: Gpio.h
**
**	程序员	: 
**
**	日　期  : 2023年1月12日
**
**	文件描述: 硬件MCU的GPIO驱动、初始化、配置等。
**
****************************************************************************************************
*/
#ifndef _GPIO_H
#define _GPIO_H
#include "UserRetype.h"



/* GPIO pin definitions */
#define GPIO_PIN_0				(1u<<0)				/*!< GPIO pin 0  */
#define GPIO_PIN_1				(1u<<1)				/*!< GPIO pin 1  */
#define GPIO_PIN_2				(1u<<2)				/*!< GPIO pin 2  */
#define GPIO_PIN_3				(1u<<3)				/*!< GPIO pin 3  */
#define GPIO_PIN_4				(1u<<4)				/*!< GPIO pin 4  */
#define GPIO_PIN_5				(1u<<5)				/*!< GPIO pin 5  */
#define GPIO_PIN_6				(1u<<6)				/*!< GPIO pin 6  */
#define GPIO_PIN_7				(1u<<7)				/*!< GPIO pin 7  */
#define GPIO_PIN_8				(1u<<8)				/*!< GPIO pin 8  */
#define GPIO_PIN_9				(1u<<9)				/*!< GPIO pin 9  */
#define GPIO_PIN_10				(1u<<10)			/*!< GPIO pin 10 */
#define GPIO_PIN_11				(1u<<11)			/*!< GPIO pin 11 */
#define GPIO_PIN_12				(1u<<12)			/*!< GPIO pin 12 */
#define GPIO_PIN_13				(1u<<13)			/*!< GPIO pin 13 */
#define GPIO_PIN_14				(1u<<14)			/*!< GPIO pin 14 */
#define GPIO_PIN_15				(1u<<15)			/*!< GPIO pin 15 */
#define GPIO_PIN_16				(1u<<16)			/*!< GPIO pin 16 */
#define GPIO_PIN_17				(1u<<17)			/*!< GPIO pin 17 */
#define GPIO_PIN_18				(1u<<18)			/*!< GPIO pin 18 */
#define GPIO_PIN_19				(1u<<19)			/*!< GPIO pin 19 */
#define GPIO_PIN_20				(1u<<20)			/*!< GPIO pin 20 */
#define GPIO_PIN_21				(1u<<21)			/*!< GPIO pin 21 */
#define GPIO_PIN_22				(1u<<22)			/*!< GPIO pin 22 */
#define GPIO_PIN_23				(1u<<23)			/*!< GPIO pin 23 */
#define GPIO_PIN_24				(1u<<24)			/*!< GPIO pin 24 */
#define GPIO_PIN_25				(1u<<25)			/*!< GPIO pin 25 */
#define GPIO_PIN_26				(1u<<26)			/*!< GPIO pin 26 */
#define GPIO_PIN_27				(1u<<27)			/*!< GPIO pin 27 */
#define GPIO_PIN_28				(1u<<28)			/*!< GPIO pin 28 */
#define GPIO_PIN_29				(1u<<29)			/*!< GPIO pin 29 */
#define GPIO_PIN_30				(1u<<30)			/*!< GPIO pin 30 */
#define GPIO_PIN_31				(1u<<31)			/*!< GPIO pin 31 */


typedef struct
{
	void (*Init)(void);
	
	struct
	{
		struct
		{
			void (*On )(void);
			void (*Off)(void);
		} RedLed;
		
		struct
		{
			void (*On )(void);
			void (*Off)(void);
		} GreenLed;
		
		struct
		{
			void (*On )(void);
			void (*Off)(void);
		} BlueLed;
	} Ui;
	
} GPIO_HANDLE;

extern GPIO_HANDLE Gpio;

#endif




