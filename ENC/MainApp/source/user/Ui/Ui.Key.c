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
**	文件名	: Ui.Key.c
**
**	程序员	:
**
**	日　期  : 2021年12月23日
**
**	文件描述: 按键应用。
**
****************************************************************************************************
*/
#include "OsTick.h"
#include "Gpio.h"
#include "Ui.Key.h"
#include "a0_hal_def.h"



/* 宏定义 */
#define KEY_MASK(id)			(uint32)(1u << id)
#define KEY_MASK_FX				(uint32)(1u << KEY_FX)
#define KEY_MASK_RGB			(uint32)(1u << KEY_RGB)

// 长按判定时间，单位ms。用 HAL_TickGet() 的真实毫秒时基，不受主循环快慢影响。
#define KEY_LONG_PRESS_MS		(1000)
// 双键同时按住触发恢复出厂的时间，单位ms
#define KEY_RES_PRESS_MS		(2000)


/* 函数声明 */
static void   _KeyInit(void);
static void   _KeyReset(void);
static void   _KeyProcess(void);
static void   _KeyClickProcess(void);
static void   _KeyEventCleanAll(void);
static uint32 _KeyClick(KEY_ENUM key);
static uint32 _KeyLongClick(KEY_ENUM key);
static uint32 _KeyResClick(void);


/* 变量定义和声明 */
UI_KEY_HANDLE  UiKey =
{
	.Init          = _KeyInit,
	.Reset         = _KeyReset,
	.Process       = _KeyProcess,

	.EventCleanAll = _KeyEventCleanAll,
	.KeyClick      = _KeyClick,
	.KeyLongClick  = _KeyLongClick,
	.KeyResClick   = _KeyResClick,
};

static uint32 KeyEvent      = 0;
static uint32 KeyShortEvent = 0;	// 短按事件标志
static uint32 KeyLongEvent  = 0;	// 长按事件标志
static uint32 KeyResEvent   = 0;	// 特殊按键事件，恢复出厂设置
static uint32 KeyValue      = 0;	// 按键接下状态，1按下，0松开


/*
****************************************************************************************************
**
**                                        _KeyInit()
**
**	功能描述:
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
static void _KeyInit(void)
{
	_KeyReset();
}


/*
****************************************************************************************************
**
**                                        _KeyReset()
**
**	功能描述: 所有按键复位
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
void _KeyReset(void)
{
	KeyEvent = 0;
	KeyValue = 0;

	KeyShortEvent = 0;
	KeyLongEvent  = 0;
	KeyResEvent   = 0;
}


/*
****************************************************************************************************
**
**                                     _GetKeyValue()
**
**  功能描述: 按键扫描处理
**
**  参　数　: id   按键对应的索引
**
**  返回值　: 按下对应的位为1，否则为0
**
****************************************************************************************************
*/
static uint32 _GetKeyValue(uint32 id)
{
	uint32 key = 0;		// 键值是用不同的位表示


	switch (id)
	{
		case KEY_FX:
			if (Gpio.Ui.Key.FxKeyGetStatus())
			{
				key |= KEY_MASK_FX;
			}
			break;

		case KEY_RGB:
			if (Gpio.Ui.Key.RgbKeyGetStatus())
			{
				key |= KEY_MASK_RGB;
			}
			break;
	}

	return key;
}


/*
****************************************************************************************************
**
**                                     _SingleKeyScan()
**
**  功能描述: 完成单个按键的处理
**
**  参　数　: id   按键对应的索引
**            keyVal   按键值。每个按键用位表示。1 按下，0 松开
**
**  返回值　: 是否有新的按键按下。1 有， 0 无
**
****************************************************************************************************
*/
static uint32 _SingleKeyScan(uint32 id, uint32 *keyVal)
{
	static uint32 state[KEY_NUM] = {0};
	static uint32 time [KEY_NUM] = {0};
	static uint32 cnt  [KEY_NUM] = {0};
	uint32 keyMask = KEY_MASK(id);
	uint32 newKey = 0;
	uint32 ret = 0;


	switch (state[id])
	{
		case 0:
			newKey = _GetKeyValue(id);
			if (newKey)
			{
				state[id] = 1;
				cnt[id]   = 10;				// 设置抖动时间，单位ms
				time[id]  = 1;				// 启动定时器
			}
			(*keyVal) &= ~keyMask;
			break;

		case 1:
			if (OsTick.TimerMs(&time[id], TFB_KEY_SCAN))
			{
				time[id] = 1;				// 每1ms检测一次

				newKey = _GetKeyValue(id);
				if (newKey)
				{
					if (--cnt[id] == 0)
					{
						(*keyVal) |= KEY_MASK(id);
						state[id]  = 2;
						ret = 1;
					}
					else
					{
						(*keyVal) &= ~keyMask;
					}
				}
				else
				{
					state[id] = 0;
					(*keyVal) &= ~keyMask;
				}
			}
			break;

		case 2:								// 等待按键松开
			newKey = _GetKeyValue(id);
			if (newKey == 0)
			{
				state[id] = 0;
				(*keyVal) &= ~keyMask;
			}
			else
			{
				(*keyVal) |= keyMask;
			}
			break;

		default:
			(*keyVal) &= ~keyMask;
			break;
	}

	return ret;
}


/*
****************************************************************************************************
**
**                                     _KeyProcess()
**
**  功能描述: 按键扫描处理
**
**  参　数　:
**
**  返回值　:
**
****************************************************************************************************
*/
static void _KeyProcess(void)
{
	uint32 value;		// 指单个按键值
	uint32 keyMask;


	for (uint32 i=0; i<KEY_NUM; i++)
	{
		keyMask = KEY_MASK(i);
		value   = 0;

		if (_SingleKeyScan(i, &value))
		{
			KeyEvent |= keyMask;
		}

		if (value) KeyValue |=  keyMask;
		else	   KeyValue &= ~keyMask;
	}

	_KeyClickProcess();
}


/*
****************************************************************************************************
**
**                                        _KeyClickProcess()
**
**	功能描述: 按键单击、双击、长按等处理
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
static void _KeyClickProcess(void)
{
	// t1/t2/t3 存的是 HAL_TickGet() 按下时刻，不是倒计时；0 表示当前没有进行中的按压。
	// 原实现用 OsTick.TimerMs() 扫描式计数，每轮主循环才减 1，主循环一轮要 4~5ms，
	// 导致 1700 的阈值实际要 7~8 秒才到，长按几乎无法触发。改用真实毫秒时基后与主循环快慢无关。
	static uint32 t1   = 0;
	static uint32 t2   = 0;
	static uint32 t3   = 0;
	static uint32 evt  = 0;
	static uint32 flag = 0;


	// 特殊长按处理 ------------------------------------------------------------
	if (KeyEvent & KEY_MASK(KEY_FX))
	{
		evt |= 1u;
	}
	if (KeyEvent & KEY_MASK(KEY_RGB))
	{
		evt |= 2u;
	}

	if ((evt & 3u) == 3u)
	{
		evt  = 0;
		flag = 1;
		t3   = HAL_TickGet();				// 记录双键同时按下的时刻
	}

	if (flag)
	{
		if (!(KeyValue & KEY_MASK(KEY_FX)) || !(KeyValue & KEY_MASK(KEY_RGB)))
		{
			t3   = 0;
			flag = 0;
			KeyResEvent = 0;
		}
	}

	if (t3 != 0 && (HAL_TickGet() - t3) >= KEY_RES_PRESS_MS)
	{
		t3   = 0;
		flag = 0;
		KeyResEvent = 1;
	}


	// KEY_MUTE_VOL ------------------------------------------------------------
	{
		if (KeyEvent & KEY_MASK(KEY_FX))
		{
			KeyEvent &= ~KEY_MASK(KEY_FX);
			t1 = HAL_TickGet();				// 记录按下时刻
		}

		if (t1 != 0 && !(KeyValue & KEY_MASK(KEY_FX)))		// 提前松开了，就认为短按
		{
			t1 = 0;
			KeyShortEvent |= KEY_MASK(KEY_FX);
		}

		// 按住时间到，还按着，就认为长按（一次按压只触发一次，触发后不再算短按）
		if (t1 != 0 && (KeyValue & KEY_MASK(KEY_FX)) &&
			(HAL_TickGet() - t1) >= KEY_LONG_PRESS_MS)
		{
			t1 = 0;
			KeyLongEvent |= KEY_MASK(KEY_FX);
		}
	}

	// KEY_LED_DISP_MODE -------------------------------------------------------
	{
		if (KeyEvent & KEY_MASK(KEY_RGB))
		{
			KeyEvent &= ~KEY_MASK(KEY_RGB);
			t2 = HAL_TickGet();				// 记录按下时刻
		}

		if (t2 != 0 && !(KeyValue & KEY_MASK(KEY_RGB)))		// 提前松开了，就认为短按
		{
			t2 = 0;
			KeyShortEvent |= KEY_MASK(KEY_RGB);
		}

		// 按住时间到，还按着，就认为长按（一次按压只触发一次，触发后不再算短按）
		if (t2 != 0 && (KeyValue & KEY_MASK(KEY_RGB)) &&
			(HAL_TickGet() - t2) >= KEY_LONG_PRESS_MS)
		{
			t2 = 0;
			KeyLongEvent |= KEY_MASK(KEY_RGB);
		}
	}
}


/*
****************************************************************************************************
**
**                                        _KeyEventCleanAll()
**
**	功能描述: 清除所有按键事件
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
static void _KeyEventCleanAll(void)
{
	KeyShortEvent = 0;
	KeyLongEvent  = 0;
	KeyResEvent   = 0;
}


/*
****************************************************************************************************
**
**                                        _KeyClick()
**
**	功能描述: 按键单击
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
static uint32 _KeyClick(KEY_ENUM key)
{
	if (KeyShortEvent & KEY_MASK(key))
	{
		KeyShortEvent &= ~KEY_MASK(key);
		return 1;
	}
	return 0;
}


/*
****************************************************************************************************
**
**                                      _KeyLongClick()
**
**	功能描述: 按键长按
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
static uint32 _KeyLongClick(KEY_ENUM key)
{
	if (KeyLongEvent & KEY_MASK(key))
	{
		KeyLongEvent &= ~KEY_MASK(key);
		return 1;
	}
	return 0;
}


/*
****************************************************************************************************
**
**                                      _KeyResClick()
**
**	功能描述: 恢复出厂设置按键长按
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
static uint32 _KeyResClick(void)
{
	if (KeyResEvent)
	{
		KeyResEvent = 0;
		return 1;
	}
	return 0;
}


/*
****************************************************************************************************
**
**                                        _KeyDoubleClick()
**
**	功能描述: 按键双击
**
**	参  数  :
**
**	返回值  :
**
****************************************************************************************************
*/
//static uint32 _KeyDoubleClick(KEY_ENUM key)
//{
//	return 1;
//}




