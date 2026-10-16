#ifndef __SPI_FLASH_CORE_C__
#define __SPI_FLASH_CORE_C__

#ifdef __cplusplus
extern "C" {
#endif
/**************************************START OF THE FILE****************************************************************/
/*!**************************************************************************************************
\copyright Copyright (C) 2015 MEDELI Electronics Co., LTD.			
                                                                                                    *
\defgroup       SPI_FLASH    SPI_FLASH GROUP 
                                                                                                    *
\file           spi_flash_core.c
                                                                                                    *
\brief          SPI FLASH操作代码文件（当前芯片为MX25L4006E）
                                                                                                    *
\version        V0.04
                                                                                                    *
\author         Ryan 2015-11-26 18:26:33
****************************************************************************************************/
#include "spi_flash_public.h"


#define FLASH_SPI	SPI2
/*-------------------------------------------------------------------------------------------------*/
/*                                           变量声明                                              */
/*-------------------------------------------------------------------------------------------------*/
SPI_FLASH_VARS sSpiFlashVars;
SPI_FLASH_VARS *c_psSpiFlashVars = &sSpiFlashVars;
SPI_FLASH_MSG sSpiFlashMsg;
SPI_FLASH_MSG *c_psSpiFlashMsg = &sSpiFlashMsg;



/*-------------------------------------------------------------------------------------------------*/
/*                                           函数声明                                              */
/*-------------------------------------------------------------------------------------------------*/



/*!**************************************************************************************************
\brief          初始化
                                                                                                    *
\param[in]      No param_in
                                                                                                    *
\param[out]     No param_out
                                                                                                    *
****************************************************************************************************/
void SPI_FLASH_Init(void)
{
	SPI_InitTypeDef	SpiInitParas =  {0};
	
	/* 控制寄存器设置 */
	/* select SPI as master*/
    SpiInitParas.Mstr = SPI_MASTER;
    /* select SPI MSB to LSB */
    SpiInitParas.Dord = SPI_MSB2LSB;
	/* select SPI HIGH BYTE or LOW BYTE */
    SpiInitParas.ByteDord = SPI_HIGHB2LOW;
	/* select SPI polarity and phase */
    SpiInitParas.CphaCpol = CPHA1CPOL1;
	/* select SPI transfer width/frame size */
	SpiInitParas.TransWidth = SPI_8BIT;
	/* select SPI fifo width */
    SpiInitParas.BusWidth = SPI_8BIT;
	
	/*clk divide from PCLK */
    SpiInitParas.BaudRata = 1000000;
	/* SPI Enable */
    SpiInitParas.SpiEn = 1;
	SpiInitParas.Csset = 1;
	
	SpiInitParas.TimeOutVal = 0xfff;
	
	SPIInit(FLASH_SPI,&SpiInitParas);
	
	/* 初始化变量 */
	c_psSpiFlashVars->eStatus = SPI_FLASH_STATUS_IDLE;
	
	c_psSpiFlashVars->dwEraseAddr = 0;
	
	c_psSpiFlashVars->dwEraseSize = 0;
	
	c_psSpiFlashVars->dwWrSrcAddr = 0;
	
	c_psSpiFlashVars->dwWrDestAddr = 0;
	
	c_psSpiFlashVars->dwWrSize = 0;
	
	c_psSpiFlashVars->dwRdSrcAddr = 0;
	
	c_psSpiFlashVars->dwRdDestAddr = 0;
	
	c_psSpiFlashVars->dwRdSize = 0;
	
	/* 初始化消息变量 */
	c_psSpiFlashMsg->byMsgWr = 0;
	
	c_psSpiFlashMsg->byMsgRd = 0;
	
	return;
}

/****************************************************************************************************/
void SPI_FLASH_RdID(DWORD dwDestAddr)
{
	BYTE byData[4];

  	SpiCsClr(FLASH_SPI);

	byData[0] = SPI_FLASH_CMD_RD_ID;
	
	HAL_SPI_TxData(FLASH_SPI,byData, 1);
	
	HAL_SPI_RxData(FLASH_SPI,(BYTE *)dwDestAddr, 2);

	SpiCsSet(FLASH_SPI);
}

/*!**************************************************************************************************
\brief          允许写
                                                                                                    *
\param[in]      No param_in
                                                                                                    *
\param[out]     No param_out
                                                                                                    *
****************************************************************************************************/
void SPI_FLASH_EnableWrite(void)
{
	BYTE byData[1];

	SpiCsClr(FLASH_SPI);
	
	byData[0] = SPI_FLASH_CMD_WREN;
	
	HAL_SPI_TxData(FLASH_SPI,byData,1);

	SpiCsSet(FLASH_SPI);
}



/*!**************************************************************************************************
\brief          扇区擦除
                                                                                                    *
\param[in]      dwSectorAddr			扇区地址
                                                                                                    *
\param[out]     No param_out
                                                                                                    *
****************************************************************************************************/
void SPI_FLASH_EraseSector(DWORD dwSectorAddr)
{
	BYTE byData[4];
 
  	/* 允许写 */
  	SPI_FLASH_EnableWrite();

  	SpiCsClr(FLASH_SPI);

	byData[0] = SPI_FLASH_CMD_SE;
	
	byData[1] = (dwSectorAddr & 0xFF0000) >> 16;
	
	byData[2] = (dwSectorAddr & 0xFF00) >> 8;
	
	byData[3] = dwSectorAddr & 0xFF;
	
	HAL_SPI_TxData(FLASH_SPI,byData,4);

	SpiCsSet(FLASH_SPI);
}


/*!**************************************************************************************************
\brief          PAGE WRITE
                                                                                                    *
\param[in]		FLASH
                                                                                                    *
\param[in]      SDRAM
                                                                                                    *                                                                                                    
\param[out]     No param_out
                                                                                                    *
****************************************************************************************************/
void SPI_FLASH_PageWr(DWORD dwDestAddr, DWORD dwSrcAddr, DWORD dwSize)
{
	BYTE byData[4];
 
  	/* 允许写 */
  	SPI_FLASH_EnableWrite();

  	SpiCsClr(FLASH_SPI);

	byData[0] = SPI_FLASH_CMD_PP;
	
	byData[1] = (dwDestAddr & 0xff0000) >> 16;
	
	byData[2] = (dwDestAddr & 0xff00) >> 8;
	
	byData[3] = dwDestAddr & 0xff;
	
	HAL_SPI_TxData(FLASH_SPI,byData, 4);
	
	HAL_SPI_TxData(FLASH_SPI,(BYTE *)dwSrcAddr, dwSize);

	SpiCsSet(FLASH_SPI);
}



/*!**************************************************************************************************
\brief          PAGE READ
                                                                                                    *
\param[in]		FLASH
                                                                                                    *
\param[in]      SDRAM
                                                                                                    *                                                                                                    
\param[out]     No param_out
                                                                                                    *
****************************************************************************************************/
void SPI_FLASH_PageRd(DWORD dwSrcAddr, DWORD dwDestAddr, DWORD dwSize)
{
	BYTE byData[4];

  	SpiCsClr(FLASH_SPI);

	byData[0] = SPI_FLASH_CMD_READ;
	
	byData[1] = (dwSrcAddr & 0xff0000) >> 16;
	
	byData[2] = (dwSrcAddr & 0xff00) >> 8;
	
	byData[3] = dwSrcAddr & 0xff;
	
	HAL_SPI_TxData(FLASH_SPI,byData, 4);
	
	HAL_SPI_RxData(FLASH_SPI,(BYTE *)dwDestAddr, dwSize);

	SpiCsSet(FLASH_SPI);
}

/*!**************************************************************************************************
\brief          查询FLASH是否处于空闲状态
                                                                                                    *
\param[in]      No param_in
                                                                                                    *
\param[out]     result
                                                                                                    *
****************************************************************************************************/
BOOL SPI_FLASH_CheckIdleStatus(void)
{
	BOOL boRet;

	if(c_psSpiFlashVars->eStatus == SPI_FLASH_STATUS_IDLE)
	{
		boRet = TRUE;
	}
	else 
	{
		boRet = FALSE;
	}

	return boRet;
}



/*!**************************************************************************************************
\brief          查询FLASH是否正在进行擦写
                                                                                                    *
\param[in]      No param_in
                                                                                                    *
\param[out]     result
                                                                                                    *
****************************************************************************************************/
BOOL SPI_FLASH_CheckBusy(void)
{
	BYTE byData[2];
	
	BOOL boRet;
	
	SpiCsClr(FLASH_SPI);
	
	byData[0] = SPI_FLASH_CMD_RDSR;
	
	HAL_SPI_TxData(byData,1);
	
	HAL_SPI_RxData(byData,2);

	SpiCsSet(FLASH_SPI);

	if(byData[0] & 0x01)
	{
		boRet = TRUE;
	}
	else 
	{
		boRet = FALSE;
	}

	return boRet;
}



///*!**************************************************************************************************
//\brief          SPI FLASH MSG WRITE
//                                                                                                    *
//\param[in]      message type
//                                                                                                    *
//\param[in]      para1-3
//                                                                                                    *                                                                                                                                                                                                                                                                                                                
//\param[out]     No param_out
//                                                                                                    *
//****************************************************************************************************/
//void SPI_FLASH_MsgWr(SPI_FLASH_MSG_TYPE eType, DWORD dwPara1, DWORD dwPara2, DWORD dwPara3, DWORD dwPara4)
//{
//	SPI_FLASH_MSG_PARA *psMsgPara;
//	
//	psMsgPara = &c_psSpiFlashMsg->sMsg[c_psSpiFlashMsg->byMsgWr++];
//	
//	psMsgPara->eType = eType;
//	
//	psMsgPara->dwSrcAddr = dwPara1;
//	
//	psMsgPara->dwDestAddr = dwPara2;
//	
//	psMsgPara->dwSize = dwPara3;
//	
//	psMsgPara->dwFlagAddr = dwPara4;

//	/* 防溢出 */
//	if(c_psSpiFlashMsg->byMsgWr == SPI_FLASH_MSG_NUM)
//	{
//		c_psSpiFlashMsg->byMsgWr = 0;
//	}
//}



///*!**************************************************************************************************
//\brief          SPI FLASH MSG PROCESS
//                                                                                                    *
//\param[in]      No param_in
//                                                                                                    *
//\param[out]     No param_out
//                                                                                                    *
//****************************************************************************************************/
//void SPI_FLASH_MsgProc(void)
//{
//	SPI_FLASH_MSG_PARA *psMsgPara;
//	
//	if(c_psSpiFlashVars->eStatus == SPI_FLASH_STATUS_IDLE)
//	{
//		/* SPI FLASH处于空闲状态 */	
//		if(c_psSpiFlashMsg->byMsgRd != c_psSpiFlashMsg->byMsgWr)
//		{
//			psMsgPara = &c_psSpiFlashMsg->sMsg[c_psSpiFlashMsg->byMsgRd];
//			
//			c_psSpiFlashMsg->byMsgRd++;
//			
//			/* 防溢出 */
//			if(c_psSpiFlashMsg->byMsgRd == SPI_FLASH_MSG_NUM)
//			{
//				c_psSpiFlashMsg->byMsgRd = 0;
//			}
//			
//			/* 处理数据信息 */
//			switch(psMsgPara->eType)
//			{
//				case(SPI_FLASH_MSG_ERASE):
//				{
//					c_psSpiFlashVars->eStatus = SPI_FLASH_STATUS_ERASE;
//					
//					c_psSpiFlashVars->dwEraseAddr = psMsgPara->dwDestAddr;
//					
//					c_psSpiFlashVars->dwEraseSize = psMsgPara->dwSize;
//				}
//				break;
//				
//				case(SPI_FLASH_MSG_WRITE):
//				{
//					c_psSpiFlashVars->eStatus = SPI_FLASH_STATUS_WRITE;
//					
//					/* SDRAM */
//					c_psSpiFlashVars->dwWrSrcAddr = psMsgPara->dwSrcAddr;
//					
//					/* FLASH */
//					c_psSpiFlashVars->dwWrDestAddr = psMsgPara->dwDestAddr;
//					
//					c_psSpiFlashVars->dwWrSize = psMsgPara->dwSize;
//				}
//				break;	
//				
//				case(SPI_FLASH_MSG_READ):
//				{
//					c_psSpiFlashVars->eStatus = SPI_FLASH_STATUS_READ;
//					
//					/* FLASH */
//					c_psSpiFlashVars->dwRdSrcAddr = psMsgPara->dwSrcAddr;
//					
//					/* SDRAM */
//					c_psSpiFlashVars->dwRdDestAddr = psMsgPara->dwDestAddr;
//					
//					c_psSpiFlashVars->dwRdSize = psMsgPara->dwSize;
//				}
//				break;
//				
//				default:break;
//			}
//			
//			/* 更新标记地址 */
//			c_psSpiFlashVars->dwFlagAddr = psMsgPara->dwFlagAddr;
//		}
//	}
//}



///*!**************************************************************************************************
//\brief          SPI FLASH DATA PROCESS
//                                                                                                    *
//\param[in]      No param_in
//                                                                                                    *
//\param[out]     No param_out

//****************************************************************************************************/
//void SPI_FLASH_DataProc(void)
//{
//	SPI_FLASH_STATUS eStatus = c_psSpiFlashVars->eStatus;
//	
//	DWORD *pdwFlag;
//	
//	DWORD dwVal,dwLengthNew;
//	
//	if(eStatus != SPI_FLASH_STATUS_IDLE)
//	{
//		switch(eStatus)
//		{
//			case(SPI_FLASH_STATUS_ERASE):
//			{
//				if(SPI_FLASH_CheckBusy() == FALSE)
//				{
//					SPI_FLASH_EraseSector(c_psSpiFlashVars->dwEraseAddr);

//					/* 更新擦除地址 */
//					c_psSpiFlashVars->dwEraseAddr += SPI_FLASH_SECTOR_SIZE;
//					
//					c_psSpiFlashVars->dwEraseSize -= SPI_FLASH_SECTOR_SIZE;
//					
//					/* 判断是否擦完 */
//					if(c_psSpiFlashVars->dwEraseSize == 0)
//					{
//						c_psSpiFlashVars->eStatus = SPI_FLASH_STATUS_IDLE;
//						
//						/* 通知API任务完成 */
//						if(c_psSpiFlashVars->dwFlagAddr != 0)
//						{
//							pdwFlag = (DWORD *)c_psSpiFlashVars->dwFlagAddr;
//							
//							dwVal = *pdwFlag;
//							
//							dwVal++;
//							
//							*pdwFlag = dwVal;
//						}
//					}
//				}
//			}
//			break;
//			
//			case(SPI_FLASH_STATUS_WRITE):
//			{
//				if(SPI_FLASH_CheckBusy() == FALSE)
//				{
//					if(c_psSpiFlashVars->dwWrSize != 0)
//					{/*****所有数据是否已写完********/
//						if(c_psSpiFlashVars->dwWrDestAddr & (SPI_FLASH_PAGE_SIZE - 1))
//						{/***起始地址不是page的首地址，一次写入的数据要少于256字节**********/
//							dwLengthNew = (SPI_FLASH_PAGE_SIZE - c_psSpiFlashVars->dwWrDestAddr & (SPI_FLASH_PAGE_SIZE - 1));

//							if(c_psSpiFlashVars->dwWrSize < dwLengthNew)
//							{
//								dwLengthNew = c_psSpiFlashVars->dwWrSize;
//							}
//						}
//						else
//						{/***起始地址是page的首地址，一次写入的数据最多可以是256字节**********/
//							if(c_psSpiFlashVars->dwWrSize < SPI_FLASH_PAGE_SIZE)
//							{
//								dwLengthNew = c_psSpiFlashVars->dwWrSize;
//							}
//							else
//							{
//								dwLengthNew = SPI_FLASH_PAGE_SIZE;
//							}	
//						}						
//						SPI_FLASH_PageWr(c_psSpiFlashVars->dwWrDestAddr, c_psSpiFlashVars->dwWrSrcAddr,dwLengthNew);
//						/* 更新写地址 */	
//						c_psSpiFlashVars->dwWrSize -= dwLengthNew;
//					
//						c_psSpiFlashVars->dwWrDestAddr += dwLengthNew;
//		
//						c_psSpiFlashVars->dwWrSrcAddr += dwLengthNew;					
//					}					
//					else
//					{/*****所有数据已写完********/
//						c_psSpiFlashVars->eStatus = SPI_FLASH_STATUS_IDLE;
//						
//						/* 通知API任务完成 */
//						if(c_psSpiFlashVars->dwFlagAddr != 0)
//						{
//							pdwFlag = (DWORD *)c_psSpiFlashVars->dwFlagAddr;
//							
//							dwVal = *pdwFlag;
//							
//							dwVal++;
//							
//							*pdwFlag = dwVal;
//						}
//					}
//				}
//			}
//			break;
//			
//			case(SPI_FLASH_STATUS_READ):
//			{
//				if(c_psSpiFlashVars->dwRdSize != 0)
//				{/*****所有数据是否已读完********/
//					if(c_psSpiFlashVars->dwRdSize < SPI_FLASH_PAGE_SIZE)
//					{
//						dwLengthNew = c_psSpiFlashVars->dwRdSize;
//					}
//					else
//					{
//						dwLengthNew = SPI_FLASH_PAGE_SIZE;
//					}						
//					SPI_FLASH_PageRd(c_psSpiFlashVars->dwRdSrcAddr, c_psSpiFlashVars->dwRdDestAddr, dwLengthNew);
//					/* 更新读地址 */	
//					c_psSpiFlashVars->dwRdSize -= dwLengthNew;
//				
//					c_psSpiFlashVars->dwRdSrcAddr += dwLengthNew;
//				
//					c_psSpiFlashVars->dwRdDestAddr += dwLengthNew;
//				}
//				else
//				{/*****所有数据已读完********/
//					c_psSpiFlashVars->eStatus = SPI_FLASH_STATUS_IDLE;
//					
//					/* 通知API任务完成 */
//					if(c_psSpiFlashVars->dwFlagAddr != 0)
//					{
//						pdwFlag = (DWORD *)c_psSpiFlashVars->dwFlagAddr;
//						
//						dwVal = *pdwFlag;
//							
//						dwVal++;
//						
//						*pdwFlag = dwVal;
//					}
//				}
//			}
//			break;	

//			default: break;
//		}
//	}
//}


///*!**************************************************************************************************
//\brief          查询是否所有的消息全部处理了
//                                                                                                    *
//\param[in]      No param_in
//                                                                                                    *
//\param[out]     No param_out

//****************************************************************************************************/
//BOOL SPI_FLASH_AskEnd(void)
//{
//	if(c_psSpiFlashMsg->byMsgRd == c_psSpiFlashMsg->byMsgWr)
//	{
////		return TRUE;
//		{//LDLF YJ 170303:应该保证最后一次成功写入
//			if(c_psSpiFlashVars->eStatus == SPI_FLASH_STATUS_IDLE)
//			{
//				return TRUE;
//			}
//			else
//			{
//				return FALSE;
//			}
//			}
//	}
//	else
//	{
//		return FALSE;
//	}

//}



/****************************************END OF THE FILE***************************************************************/
#ifdef __cplusplus
};
#endif

#endif


