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
**	文件名	: Ui.c
**
**	程序员	:
**
**	日　期  : 2021年12月23日
**
**	文件描述: Ui处理
**
****************************************************************************************************
*/
#include "OsTick.h"
#include "Gpio.h"
#include "Ui.h"
#include "ParamTable.h"
#include "Dsp.h"
#include "Memory.h"
#include "Com.h"  // Liang
#include "drv_alg.h"    //Garry
#include "a0_hal_gpio.h"
#include "a0_hal_adc.h"   // 电位器：HalAdcInit / HAL_ADC_START / HAL_ADC_BUSY_CHECK / HAL_ADC_CH_DATA_GET
#include "app_codec.h"    // 电位器写 codec：AppCodecMicVolCfg / AppCodecHpVolCfg


/* 宏定义 */
#define MENU_EXIT_TO		(4000)		// 退回到Home的超时时间


#define MIC					(0)
#define HP					(1)


/* 函数声明 */
void UiInit(void);
void UiProcess(void);
void UiDisable(void);


void _UiEventProcess(void);
void _FxProcess(void);
void _MuteProcess(void);
void _SysResProcess(void);
void _LedShowProcess(void);


void _UsbOutProcess(void);
void _Game71Process(void);
void _UiEventFirst(void);

void MicSigLevDet_Process(void);         //Garry 电平指示

void _PotInit(void);                     // 电位器 ADC 初始化
void _PotProcess(void);                  // 电位器电压 -> 麦克风/耳机增益

/* 电位器：GPIO0.27 = 麦克风增益(AIN8 = ADC ch8)，GPIO0.6 = 耳机输出(AIN19 = ADC ch19)
   10bit SAR，读值 0~1023；顺时针(读值变大)增益/音量变大 */
#define POT_MIC_PIN         (27)
#define POT_MIC_CH          (8)
#define POT_HP_PIN          (6)
#define POT_HP_CH           (19)
#define POT_ADC_MAX         (1023)
#define POT_MIC_DB_MIN      (-27)    // codec lInVol 全域 -27 ~ +36 dB
#define POT_MIC_DB_MAX      (5)
#define POT_HP_DB_MIN       (-40)    // codec lHpVol/rHpVol 全域 -40 ~ 0 dB
#define POT_HP_DB_MAX       (0)

/* 变量定义和声明 */
UI_HANDLE  Ui =
{
	.Led = &UiLed,
	.Key = &UiKey,

	.Init    = UiInit,
	.Task    = UiProcess,
	.Disable = UiDisable,
};

UI_PARAM UiParam = {0};
int32 MicSigLev[2] = {-300, -300};		//Garry        MIC电平值

/*
****************************************************************************************************
**
**                                        UiInit()
**
**	功能描述:
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/


void UiInit(void)
{
	// 赋个不一样的值，用于判断
    UiParam.Last.MicMute                = 0xFFFF;
    UiParam.Last.NoiseSuppressorEnabled = 0xFFFF;
//	UiParam.Last.ReverbEnabled          = 0xFFFF;
    UiParam.Last.NoiseSuppressor4Levels = 0xFFFF;	// Liang
//    UiParam.Last.LedShowOnOff           = 0xFFFF;
//    UiParam.Last.LedShowMode            = 0xFFFF;
	  
	  UiParam.Last.UsbOutIncrease			= 0xFFFF;	// Steve
    UiParam.Last.Game71OnOff			= 0xFFFF;	// Niko

	Ui.Led->Init();
	Ui.Key->Init();

    _PotInit();         // 此时 AppCodecInit() 已在 UserFunctionInit() 里跑过，codec 已上电

    _UiEventFirst();
    
	ParamTbl.Config->SyncFlag = 0;
}


/*
****************************************************************************************************
**
**                                        UiProcess()
**
**	功能描述:
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
void UiProcess(void)
{
	Ui.Key->Process();

	_UiEventProcess();		// UI事件处理

//	Ui.Led->Process();
//	Ui.Key->EventCleanAll();
}


/*
****************************************************************************************************
**
**                                        UiDisable()
**
**	功能描述: 关闭UI所有显示
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
void UiDisable(void)
{
	Ui.Led->AllOffAction();
}


/*
****************************************************************************************************
**
**                                        _UiEventProcess()
**
**	功能描述: Ui事件处理
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
void _UiEventProcess(void)
{
		_FxProcess();

  	_MuteProcess();

		_LedShowProcess();

//	_UsbOutProcess();	// Steve
    
//  _Game71Process();   // Niko
	
//	_SysResProcess();
    
    MicSigLevDet_Process();// Garry

    _PotProcess();          // 电位器 -> 麦克风/耳机增益

}


/*
****************************************************************************************************
**
**                                        _UiEventFirst()
**
**	功能描述: Ui事件处理
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
void _UiEventFirst(void)
{
    _UiEventProcess();
    
    Mem.PresetParamAutoSaveCancel();
}

#if (1)		// Mute menu
/*
****************************************************************************************************
**
**                                       _FxProcess()
**
**	功能描述:
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
void _FxProcess(void)
{
	if (Ui.Key->KeyLongClick(KEY_FX))
	{
		if (!ParamTbl.Config->SyncFlag && ParamTbl.Preset->MicMute == 0)
		{
			ParamTbl.Preset->NoiseSuppressorEnabled ^= 1;
		}
	}

	Ui.Led->FxMute(ParamTbl.Preset->MicMute, ParamTbl.Preset->NoiseSuppressorEnabled);


//	if (UiParam.Last.NoiseSuppressorEnabled != ParamTbl.Preset->NoiseSuppressorEnabled)
	if (UiParam.Last.NoiseSuppressorEnabled != ParamTbl.Preset->NoiseSuppressorEnabled || 
		UiParam.Last.NoiseSuppressor4Levels != ParamTbl.Preset->NoiseSuppressor4Levels)  // Liang
	{
		UiParam.Last.NoiseSuppressorEnabled = ParamTbl.Preset->NoiseSuppressorEnabled;
		UiParam.Last.NoiseSuppressor4Levels = ParamTbl.Preset->NoiseSuppressor4Levels;  // Liang

		Mem.PresetParamAutoSavePreview();

//		uint16 data[2];
		uint16 data[4];  // Liang
//		data[0] = PARAM_NS;
		data[0] = PARAM_NS_EN;  // Liang
		data[1] = ParamTbl.Preset->NoiseSuppressorEnabled;
		
		data[2] = PARAM_NS_THRESHOLD;  // Liang
		data[3] = ParamTbl.Preset->NoiseSuppressor4Levels;  // Liang

		Dsp.Program->AMSet.RandParam((uint8*)data, sizeof(data));
		#if DBG_TX_PC_EN
		Com.Tx.ParamRandWrite(data, SIZE_OF(data));
		#endif
	}
}
#endif


#if (1)		// Mute menu
/*
****************************************************************************************************
**
**                                       _MuteProcess()
**
**	功能描述:
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
void _MuteProcess(void)
{
	if (Ui.Key->KeyClick(KEY_FX))
	{
		
		if (!ParamTbl.Config->SyncFlag)
		{
			ParamTbl.Preset->MicMute ^= 1;
		}
		
	}			 

	Ui.Led->FxMute(ParamTbl.Preset->MicMute, ParamTbl.Preset->NoiseSuppressorEnabled);

	if (UiParam.Last.MicMute != ParamTbl.Preset->MicMute)
	{
		UiParam.Last.MicMute = ParamTbl.Preset->MicMute;

		Mem.PresetParamAutoSavePreview();

		uint16 data[2];
		data[0] = PARAM_MIC_MUTE;
		data[1] = ParamTbl.Preset->MicMute;

		Dsp.Program->AMSet.RandParam((uint8*)data, sizeof(data));
		#if DBG_TX_PC_EN  // Liang 
		Com.Tx.ParamRandWrite(data, SIZE_OF(data));
		#endif
	}
}
#endif


#if (1)		// 恢复出厂设置
/*
****************************************************************************************************
**
**                                       _SysResProcess()
**
**	功能描述: 恢复出厂设置处理
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
void _SysResProcess(void)
{
	if (Ui.Key->KeyResClick())
	{
//		Mem.ResFactory();

		Ui.Led->AllLedBlink();

		while (1)		// 手动重启
		{
		}
	}
}
#endif


#if (1)		// Misc Process
/*
****************************************************************************************************
**
**                                    _PotInit()
**
**	功能描述: 两个电位器的引脚复用成 ADC 模拟输入，并启动第一次转换。
**	          GPIO0.27=AIN8=ch8 管麦克风增益，GPIO0.6=AIN19=ch19 管耳机输出增益。
**	          本 MCU 没有单独的「模拟使能」，把 AF 设成 AF2(AINn) 即可。
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
static ADC_InitTypeDef _potAdcInit =
{
    .chSel      = (1U << POT_MIC_CH) | (1U << POT_HP_CH),
    .adcClk     = 16000000,
    .delayCycle = 15,
};

void _PotInit(void)
{
    HAL_GPIO_AF_SET(GPIO0, POT_MIC_PIN, GPIO_AF2_AIN8);     // 模拟输入
    HAL_GPIO_AF_SET(GPIO0, POT_HP_PIN,  GPIO_AF2_AIN19);
    HalAdcInit(&_potAdcInit);                               // 配好多通道模式，注意它自己不启动转换
    HAL_ADC_START();                                        // 启动第一次转换，之后每轮由 _PotProcess 再启动
}


/*
****************************************************************************************************
**
**                                    _PotProcess()
**
**	功能描述: 每 ~20ms 读一次两个电位器的 ADC 值，4 次求平均，映射成 dB 后写 codec。
**	          ADC 值变了才写寄存器，避免每轮都去写 codec 白白抖。
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
void _PotProcess(void)
{
    static uint32 tick = 0;
    static uint32 sumMic = 0;
    static uint32 sumHp  = 0;
    static uint32 cnt    = 0;
    static int8   lastMicDb = 127;      // 127 是哨兵，保证第一轮一定会写一次寄存器
    static int8   lastHpDb  = 127;

    uint32 guard;
    uint32 micAvg;
    uint32 hpAvg;
    int8   micDb;
    int8   hpDb;

    // 主循环一轮 4~5ms，用真实毫秒时基；OsTick.TimerMs 是扫描式假毫秒，长定时会被拉长好几倍
    if ((HAL_TickGet() - tick) < 20)
    {
        return;
    }
    tick = HAL_TickGet();

    // 等上一次转换完成。理论上不会超时，但这里是主循环的必经之路，卡住整机就死了，所以留个退路
    guard = 200000;
    while (HAL_ADC_BUSY_CHECK() && (--guard != 0))
    {
    }

    sumMic += HAL_ADC_CH_DATA_GET(POT_MIC_CH);      // 10bit，0~1023
    sumHp  += HAL_ADC_CH_DATA_GET(POT_HP_CH);
    HAL_ADC_START();                                // 顺手启动下一次转换

    if (++cnt < 4)                                  // 4 次(约 80ms)平均，滤电位器抖动
    {
        return;
    }
    micAvg = sumMic / 4;
    hpAvg  = sumHp  / 4;
    sumMic = 0;
    sumHp  = 0;
    cnt    = 0;

    // 线性映射到 dB。codec 模拟音量本身就是线性档，这里不用做对数补偿
    micDb = (int8)(POT_MIC_DB_MIN + (int32)micAvg * (POT_MIC_DB_MAX - POT_MIC_DB_MIN) / POT_ADC_MAX);
    hpDb  = (int8)(POT_HP_DB_MIN  + (int32)hpAvg  * (POT_HP_DB_MAX  - POT_HP_DB_MIN ) / POT_ADC_MAX);

    if (micDb != lastMicDb)
    {
        AppCodecMicVolCfg(micDb, -127);             // -127 = 右通道不动（麦克风是单声道，右通道已 powerdown）
        lastMicDb = micDb;
    }
    if (hpDb != lastHpDb)
    {
        AppCodecHpVolCfg(hpDb);
        lastHpDb = hpDb;
    }
}



/*
****************************************************************************************************
**
**                                    MicSigLevDet_Process()
**
**	功能描述: 
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
__RAM_CODE void MicSigLevDet_Process(void)
{
   
	static uint32 t = 50;
	if (OsTick.TimerMs(&t, TFB_MIC_SLD))
	{
		t = 50;
		if (!ParamTbl.Config->SyncFlag)
		{
			MicSigLev[CURR] = rmsValue[0];  
		}
		
		
			uint16 data[2];
			data[0] = PARAM_MIC_SIG_LEVEL;
			data[1] = MicSigLev[CURR];		// 参数转换 param = val * 100  

//			if(!ParamTbl.Preset->Mic_SigLevelMute)   //0发送数据，1禁止向电脑发送数据
//			{
//			    Com.Tx.ParamRandWrite(data, SIZE_OF(data));
//			}    
//			else
//			{
//			   (void)data;
//			}   
		if (MicSigLev[LAST] != MicSigLev[CURR])			// 发送给PC
		{
			      MicSigLev[LAST] = MicSigLev[CURR];
            uint16 data[2];
            data[0] = PARAM_MIC_SIG_LEVEL;
            data[1] = MicSigLev[CURR];		// 参数转换 param = val * 100  
            
            if(!ParamTbl.Preset->Mic_SigLevelMute)   //0发送数据，1禁止向电脑发送数据
            {
                Com.Tx.ParamRandWrite(data, SIZE_OF(data));
            }    
            else
            {
                (void)data;
            }                           
		}
		else if (MicSigLev[LAST] ==0)			//  数据为0时也发送给PC
		{
            uint16 data[2];
            data[0] = PARAM_MIC_SIG_LEVEL;
            data[1] = MicSigLev[CURR];		// 参数转换 param = val * 100  
            
            if(!ParamTbl.Preset->Mic_SigLevelMute)   //0发送数据，1禁止向电脑发送数据
            {
                Com.Tx.ParamRandWrite(data, SIZE_OF(data));
            }    
            else
            {
                (void)data;
            }                           
		}
		
	}
}
#endif


#if (1)		// Led show menu
/*
****************************************************************************************************
**
**                                       _LedShowProcess()
**
**	功能描述: LED灯光秀处理
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
void _LedShowProcess(void)
{
	//灯效状态目前只存在RAM里，掉电不保存、也不上报PC/上位机。
	//要做得和静音/降噪一样能存能同步，得给 PRESET_PARAM 加 LedShowOnOff / LedShowMode 两个字段，
	//那要同步改 ParamTable.Preset.h、ParamTable.Preset.c、ParamTable.h(PRESET_TBL_SIZE)、
	//Dsp.Program.ModuleTable.c，还有工程外的Python生成器（这几张表都标了"Python自动生成"），
	//少改一处就会静默不一致。先按纯本地按键行为做，需要持久化时再一起补。
	static uint32 showOnOff = 1;		// 灯效总开关，长按RGB键翻转
	static uint32 showMode  = 0;		// 当前灯效编号 0 ~ RGB_SHOW_MODE_NUM-1，短按RGB键循环

	//短按：切换灯效。静音状态下也照样切，只是画面被强制成红色，
	//一取消静音就立刻按新模式显示。
	if (Ui.Key->KeyClick(KEY_RGB))
	{
		if (!ParamTbl.Config->SyncFlag)
		{
			if (++showMode >= RGB_SHOW_MODE_NUM)
			{
				showMode = 0;
			}
		}
	}

	//长按：翻转灯效总开关。非静音下是"关灯效/显灯效"；
	//静音下画面本来就是固定红色，所以这一下就是"熄灭红灯/恢复红灯"。
	if (Ui.Key->KeyLongClick(KEY_RGB))
	{
		if (!ParamTbl.Config->SyncFlag)
		{
			showOnOff ^= 1;
		}
	}

	Ui.Led->RgbShow(showOnOff, showMode, 100);
}
#endif
//...


#if(0)
/*
****************************************************************************************************
**
**                                       _UsbOutProcess()	// Steve
**
**	功能描述: USB OUT输出音量处理
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
void _UsbOutProcess(void)
{
	if(UiParam.Last.UsbOutIncrease != ParamTbl.Preset->UsbOutIncrease)
	{
		UiParam.Last.UsbOutIncrease = ParamTbl.Preset->UsbOutIncrease;
		
		Mem.PresetParamAutoSavePreview();
		
		uint16 data[2];
		data[0] = PARAM_USB_OUT_INC;
		data[1] = ParamTbl.Preset->UsbOutIncrease;
		
		Dsp.Program->AMSet.RandParam((uint8*)data, sizeof(data));
//		#if DBG_TX_PC_EN	// Steve
//		Com.Tx.ParamRandWrite(data, SIZE_OF(data));
//		#endif
	}
}

#endif


#if(0)
/*
****************************************************************************************************
**
**                                       _Game71Process()	// Niko
**
**	功能描述: 7.1游戏音效开关处理
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
void _Game71Process(void)
{
	if(UiParam.Last.Game71OnOff != ParamTbl.Preset->Game71OnOff)
	{
		UiParam.Last.Game71OnOff = ParamTbl.Preset->Game71OnOff;
		
		Mem.PresetParamAutoSavePreview();
		
		uint16 data[2];
		data[0] = PARAM_GAME71_EN;
		data[1] = ParamTbl.Preset->Game71OnOff;
		
		Dsp.Program->AMSet.RandParam((uint8*)data, sizeof(data));
	}
}

#endif

//...

