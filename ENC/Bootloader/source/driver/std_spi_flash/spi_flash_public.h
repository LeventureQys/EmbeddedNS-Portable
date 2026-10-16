#ifndef  __SPI_FLASH_PUBLIC_H__
#define  __SPI_FLASH_PUBLIC_H__

#ifdef __cplusplus
extern "C" {
#endif
/**************************************START OF THE FILE****************************************************************/
/*!**************************************************************************************************
\copyright Copyright (C) 2015 MEDELI Electronics Co., LTD.
                                                                                                    *
\ingroup        SPI_FLASH
                                                                                                    *
\file           spi_flash_public.h
                                                                                                    *
\version        V0.03
                                                                                                    *
\brief          SPI_FLASH 头文件
                                                                                                    *
\author         sunyin 2015-11-13 16:19:57
****************************************************************************************************/
#include "a0_hal_spi_std.h"



/*-------------------------------------------------------------------------------------------------*/
/*                                           宏定义                                                */
/*-------------------------------------------------------------------------------------------------*/
/*!*************************************************************************************************
\brief  BASE SETTING
****************************************************************************************************/
#define SPI_FLASH_BAUDRATE				(6250000)													/*!< 波特率设置 */
#define SPI_FLASH_PORT					(HAL_GPIO_PORT_0)
#define SPI_FLASH_PIN					(0x000f0000)
                                        			                  
#define SPI_FLASH_SECTOR_NUM			(128)
#define SPI_FLASH_SECTOR_SIZE			(0x1000)													/*!< 每个扇区的数据长度4k,单位BYTE */
#define SPI_FLASH_PAGE_NUM				(2048)                                  			                                                			
#define SPI_FLASH_PAGE_SIZE				(0x100)														/*!< 每个页的数据长度256,单位BYTE */
#define SPI_FLASH_MSG_NUM				(64)

#define SPI_FLASH_EN_PING_GROUP			(HAL_GPIO_PORT_0)											/*!< GPIO所在的组 */
#define SPI_FLASH_EN_PIN				(HAL_GPIO_PIN_8)											/*!< SPI总线的LED和FLASH公用逻辑，使能管脚*/



/*-------------------------------------------------------------------------------------------------*/
/*                                           枚举声明                                              */
/*-------------------------------------------------------------------------------------------------*/
/*!*************************************************************************************************
\brief  SPI FLASH COMMAND
****************************************************************************************************/
typedef enum
{
	SPI_FLASH_CMD_WRSR = 0x01,																		/*!< write status register */
	SPI_FLASH_CMD_PP = 0x02,																		/*!< Page Program */
	SPI_FLASH_CMD_READ = 0x03,																		/*!< read data */
	SPI_FLASH_CMD_RDSR = 0x05,																		/*!< read status register */
	SPI_FLASH_CMD_WREN = 0x06,																		/*!< write Enable */
	SPI_FLASH_CMD_SE = 0x20,																		/*!< Sector Erase */
	SPI_FLASH_CMD_RD_ID = 0x9F,																		/*!< outputs JEDEC ID: 1-byte Manufacturer ID & 2-byte Device ID */
}SPI_FLASH_CMD;



/*!*************************************************************************************************
\brief  SPI FLASH STATUS
****************************************************************************************************/
typedef enum
{	
	SPI_FLASH_STATUS_IDLE,
	SPI_FLASH_STATUS_ERASE,
	SPI_FLASH_STATUS_WRITE,
	SPI_FLASH_STATUS_READ,	
}SPI_FLASH_STATUS;



/*!*************************************************************************************************
\brief  SPI FLASH MSG TYPE
****************************************************************************************************/
typedef enum
{
														/*    名称               |参数1(DWORD)        |参数2(DWORD)          |参数3(DWORD)            |ret(DWORD)          */
	SPI_FLASH_MSG_NULL,									/*!<  */
	SPI_FLASH_MSG_ERASE,								/*!< 擦除                |                    |(in dw)擦除地址       |(in dw)长度             |                    */
	SPI_FLASH_MSG_WRITE,								/*!< 写                  |(in dw)数据源       |(in dw)目的地址       |(in dw)长度             |                    */
	SPI_FLASH_MSG_READ,									/*!< 读                  |(in dw)spi 地址     |(out dw)目的地址      |(in dw)长度             |                    */
}SPI_FLASH_MSG_TYPE;



/*-------------------------------------------------------------------------------------------------*/
/*                                           结构体声明                                            */
/*-------------------------------------------------------------------------------------------------*/
/*!*************************************************************************************************
\brief  SPI FLASH 模块变量
****************************************************************************************************/
typedef struct
{
	SPI_FLASH_STATUS			eStatus;										/*!< 当前状态 */
	
	DWORD						dwFlagAddr;										/*!< 标记地址 */
	
	DWORD						dwEraseAddr;									/*!< 擦：地址 */
	DWORD						dwEraseSize;									/*!< 擦：数据大小 */

	DWORD						dwWrSrcAddr;									/*!< 写：SDRAM地址 */
	DWORD						dwWrDestAddr;									/*!< 写：FLASH地址 */
	DWORD						dwWrSize;										/*!< 写：数据大小 */

	DWORD						dwRdSrcAddr;									/*!< 读：FLASH地址 */
	DWORD						dwRdDestAddr;									/*!< 读：SDRAM地址 */
	DWORD						dwRdSize;										/*!< 读：数据大小 */
}SPI_FLASH_VARS;



/*!*************************************************************************************************
\brief  SPI FLASH 消息参数
****************************************************************************************************/
typedef struct
{
	SPI_FLASH_MSG_TYPE			eType;
	DWORD						dwSrcAddr;
	DWORD						dwDestAddr;
	DWORD						dwSize;
	DWORD						dwFlagAddr;
}SPI_FLASH_MSG_PARA;



/*!*************************************************************************************************
\brief  SPI FLASH 消息变量
****************************************************************************************************/
typedef struct
{
	SPI_FLASH_MSG_PARA			sMsg[SPI_FLASH_MSG_NUM];						/*!<SPI_FLASH消息*/
	BYTE						byMsgWr;										/*!<SPI_FLASH消息处理写指针*/
	BYTE						byMsgRd;										/*!<SPI_FLASH消息处理读指针*/
}SPI_FLASH_MSG;



/*-------------------------------------------------------------------------------------------------*/
/*                                           函数声明                                              */
/*-------------------------------------------------------------------------------------------------*/
extern void SPI_FLASH_Init(void);
extern void SPI_FLASH_PageRdID(DWORD dwDestAddr);
extern void SPI_FLASH_MsgWr(SPI_FLASH_MSG_TYPE eType, DWORD dwPara1, DWORD dwPara2, DWORD dwPara3, DWORD dwPara4);
extern void SPI_FLASH_MsgProc(void);
extern void SPI_FLASH_DataProc(void);
extern BOOL SPI_FLASH_CheckIdleStatus(void);
extern BOOL SPI_FLASH_AskEnd(void);


/****************************************END OF THE FILE****************************************************************/
#ifdef __cplusplus
};
#endif

#endif



