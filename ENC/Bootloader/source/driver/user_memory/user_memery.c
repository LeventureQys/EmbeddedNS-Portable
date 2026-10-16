#include "user_memery.h"

/*******************************要记忆的变量***************************************************************/
__RAM1 uint8_t Mic_mode;
__RAM1 uint8_t dsp_mic_vol;
__RAM1 uint8_t dsp_mon_vol;
__RAM1 uint8_t inrecord_on_flag;
/*******************************关机记忆*******************************************************************/
#define DATA_NUM 4
__RAM1 UINT32	PowerOffSaveData[DATA_NUM];
#define POWEROFF_SAVE_ADDR			0x00060000
UINT8 write_flash_cnt;
////////////////////////////////////////////////////////////
//读flash
void ReadFlash(void)
{
	UINT32 *pt;
	pt = (UINT32 *)POWEROFF_SAVE_ADDR;
	PowerOffSaveData[0] = *pt++;
	PowerOffSaveData[1] = *pt++;
	PowerOffSaveData[2] = *pt++;
	PowerOffSaveData[3] = *pt++;

	Mic_mode    = (UINT8) (PowerOffSaveData[0]&0xff);
	dsp_mic_vol     = (UINT8) (PowerOffSaveData[1]&0xff);
	dsp_mon_vol    = (UINT8) (PowerOffSaveData[2]&0xff);
    inrecord_on_flag = (UINT8) (PowerOffSaveData[3]&0xff);
}

//__RAM1 void WriteFlash(void)
__RAM_CODE void WriteFlash(void)
{
	//保存相关参数
	uint32_t argv[3];
	
	//更新PowerOffSaveData数组内数据，用以保存
	PowerOffSaveData[0] = (UINT32) Mic_mode;
	PowerOffSaveData[1] = (UINT32) dsp_mic_vol;
	PowerOffSaveData[2] = (UINT32) dsp_mon_vol;
	PowerOffSaveData[3] = (UINT32) 	inrecord_on_flag;
    
	argv[0] = POWEROFF_SAVE_ADDR;
	argv[1] = DATA_NUM*4;	//保存1个32位数据，所以要乘以4.		
	argv[2] = 0;	
	hal_flash_erase(argv);			//如果擦除失败,重新进行擦除
	HalSpiFlashTickDelay(10);
    
	argv[0] = POWEROFF_SAVE_ADDR;
	argv[1] = DATA_NUM*4;	//保存1个32位数据，所以要乘以4.
	argv[2] = (UINT32)&PowerOffSaveData[0];
	hal_flash_write(argv);
	
}



__RAM_CODE void  write_flash_pro(uint32_t TimeDelay)
{
	if(write_flash_cnt)
	{
		if(--write_flash_cnt == 0)
		{
			WriteFlash();
		}	
	}

}

uint32_t obsvWrFlashTime = 0;
__RAM_CODE void WritrFlashTest(uint32_t TimeDelay)
{

    static uint32_t OldTimeMscnt = 0;
    uint32_t newDmaTimeCnt = HAL_TickGet();
    if ((newDmaTimeCnt - OldTimeMscnt) >= TimeDelay)
    {
        OldTimeMscnt = HAL_TickGet();
        WriteFlash();
		obsvWrFlashTime++;
    }
	
}
/*******************************关机记忆*******************************************************************/
__RAM_CODE void *mmset(void *src, int ch, size_t len)/*移动函数*/
{
    int i = 0;
    char *psrc = src;
    for(i = 0; i < len; i++)
    {
        psrc[i] = ch;
    }
    return src;
}

__RAM_CODE void *mmcpy(void *src,  void *dst, size_t len)/*复制函数*/
{
    int i = 0;
    char *psrc = src;
    char *pdst = dst;
    for(i = 0; i < len; i++)
    {
        psrc[i] = pdst[i];
    }
    
    return src;
}

