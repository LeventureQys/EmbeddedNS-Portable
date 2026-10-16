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
**	文件名	: Helper.c
**
**	程序员	: 
**
**	日　期  : 2022年4月1日
**
**	文件描述: 辅助性函数方法。
**
****************************************************************************************************
*/
#include "Helper.h"



/* 函数声明 */
static void _BitFieldSet (uint32 *var, uint32 val, uint32 sta, uint32 num);
static void _BitFieldSet2(uint32 *var, uint32 val, uint32 sta, uint32 end);
static void _BitFieldClr (uint32 *var, uint32 sta, uint32 num);
static void _BitFieldClr2(uint32 *var, uint32 sta, uint32 end);

// static void _HSV2RGB(fp32 h, fp32 s, fp32 v, uint8 *r, uint8 *g, uint8 *b);
static void _HSV2RGB(uint32 h, uint32 s, uint32 v, uint8 *rr, uint8 *gg, uint8 *bb);
static void _RGB2HSV(uint8 r, uint8 g, uint8 b, fp32 *h, fp32 *s, fp32 *v);


/* 变量定义和声明 */
const HELPER_HANDLE Hp = 
{
	.BitSet  = _BitFieldSet,
	.BitSet2 = _BitFieldSet2,
	.BitClr  = _BitFieldClr,
	.BitClr2 = _BitFieldClr2,

	.HSV2RGB = _HSV2RGB,
	.RGB2HSV = _RGB2HSV,
};


#if (1)		// 位域操作
/*
****************************************************************************************************
**
**                                     _BitFieldSet()
**
**
**	功能描述: 设置一个变量的位域值
**
**	参  数  : var   变量
**            val   值
**            sta   起始位
**            num   位数目
**
**	返回值  : 
**
****************************************************************************************************
*/
static void _BitFieldSet(uint32 *var, uint32 val, uint32 sta, uint32 num)
{
	uint32 mask = (0xFFFFFFFF << sta) & (~(0xFFFFFFFF << (sta+num)));	
	*var &= (~mask);
	*var |= (val << sta) & mask;
}


/*
****************************************************************************************************
**
**                                     _BitFieldSet2()
**
**
**	功能描述: 设置一个变量的位域值
**
**	参  数  : var   变量
**            val   值
**            sta   起始位
**            end   结束位
**
**	返回值  : 
**
****************************************************************************************************
*/
static void _BitFieldSet2(uint32 *var, uint32 val, uint32 sta, uint32 end)
{
	uint32 mask = (0xFFFFFFFF << sta) & (~(0xFFFFFFFF << (end+1)));
	*var &= (~mask);
	*var |= (val << sta) & mask;
}


/*
****************************************************************************************************
**
**                                     _BitFieldClr()
**
**
**	功能描述: 清除一个变量的位域值
**
**	参  数  : var   变量
**            sta   起始位
**            num   位数目
**
**	返回值  : 
**
****************************************************************************************************
*/
static void _BitFieldClr(uint32 *var, uint32 sta, uint32 num)
{
	uint32 mask = (0xFFFFFFFF << sta) & (~(0xFFFFFFFF << (sta+num)));
	*var = *var & (~mask);
}


/*
****************************************************************************************************
**
**                                     _BitFieldClr2()
**
**
**	功能描述: 清除一个变量的位域值
**
**	参  数  : var   变量
**            sta   起始位
**            end   结束位
**
**	返回值  : 
**
****************************************************************************************************
*/
static void _BitFieldClr2(uint32 *var, uint32 sta, uint32 end)
{
	uint32 mask = (0xFFFFFFFF << sta) & (~(0xFFFFFFFF << (end+1)));
	*var = *var & (~mask);
}
#endif


#if (1)		// 颜色转换
/*
****************************************************************************************************
**
**                                        _HSV2RGB()
**
**	功能描述: 颜色转换，HSV -> RGB
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
static void _HSV2RGB(uint32 h, uint32 s, uint32 v, uint8 *rr, uint8 *gg, uint8 *bb)
{
	int32 r, g, b;
	int32 hi;
	int32 f, p, q, t;
	int32 tmp;


	if (h == 360*QF)
		h = 0;

	tmp = h * QF / (60*QF);
	hi = tmp / QF;
	f  = tmp % QF;

	p = v * (QF - s) / QF;
	q = v * (QF - f * s / QF) / QF;
	t = v * (QF - (QF - f) * s / QF) / QF;
	
	switch (hi)
	{
		case 0:
			r = v;
			g = t;
			b = p;
			break;

		case 1:
			r = q;
			g = v;
			b = p;
			break;

		case 2:
			r = p;
			g = v;
			b = t;
			break;

		case 3:
			r = p;
			g = q;
			b = v;
			break;

		case 4:
			r = t;
			g = p;
			b = v;
			break;

		default:        // case 5:
			r = v;
			g = p;
			b = q;
			break;
	}

	*rr = (uint8)((r * 255 + QF_DIV2) / QF);
	*gg = (uint8)((g * 255 + QF_DIV2) / QF);
	*bb = (uint8)((b * 255 + QF_DIV2) / QF);
}

#if 0
static void _HSV2RGB(fp32 h, fp32 s, fp32 v, uint8 *rr, uint8 *gg, uint8 *bb)
{
	fp32  r, g, b;
	int32 hi;
	fp32  f, p, q, t;
	fp32  tmp;


	if (h == 360.0f)
		h = 0.0f;

	tmp = h * 0.016667f;	// h / 60.0f;
	hi = (int32)tmp;
	f  = tmp - hi;

	p = v * (1.0f - s);
	q = v * (1.0f - f * s);
	t = v * (1.0f - (1.0f - f) * s);
	
	switch (hi)
	{
		case 0:
			r = v;
			g = t;
			b = p;
			break;

		case 1:
			r = q;
			g = v;
			b = p;
			break;

		case 2:
			r = p;
			g = v;
			b = t;
			break;

		case 3:
			r = p;
			g = q;
			b = v;
			break;

		case 4:
			r = t;
			g = p;
			b = v;
			break;

		default:        // case 5:
			r = v;
			g = p;
			b = q;
			break;
	}

	*rr = (uint8)(r * 255 + 0.5f);
	*gg = (uint8)(g * 255 + 0.5f);
	*bb = (uint8)(b * 255 + 0.5f);
}
#endif


/*
****************************************************************************************************
**
**                                        _RGB2HSV()
**
**	功能描述: 颜色转换，RGB -> HSV
**
**	参  数  : 
**
**	返回值  : 
**
****************************************************************************************************
*/
static void _RGB2HSV(uint8 R, uint8 G, uint8 B, fp32 *hh, fp32 *ss, fp32 *vv)
{
	fp32 r, g, b;
	fp32 h, s, v;
	fp32 min, max;
	int  MIN, MAX;           // 用于比较，整数做比较更好


	r = (fp32)R / 255;
	g = (fp32)G / 255;
	b = (fp32)B / 255;
	
				 { MIN = R; min = r; }
	if (G < MIN) { MIN = G; min = g; }
	if (B < MIN) { MIN = B; min = b; }
	
				 { MAX = R; max = r; }
	if (G > MAX) { MAX = G; max = g; }
	if (B > MAX) { MAX = B; max = b; }

	v = max;

	if (MAX == 0.0f)  s = 0.0f;
	else              s = 1.0f - min / max;

	if (MAX == MIN)
	{
		h = 0.0f;
	}
	else if (MAX == R)
	{
		if (G >= B)  h = 60.0f * (g - b) / (max - min) + 0.0f;
		else         h = 60.0f * (g - b) / (max - min) + 360.0f;
	}
	else if (MAX == G)
	{
		h = 60.0f * (b - r) / (max - min) + 120.0f;
	}
	else if (MAX == B)
	{
		h = 60.0f * (r - g) / (max - min) + 240.0f;
	}
	else
	{
		h = 0.0f;
	}

	*hh = h;
	*ss = s;
	*vv = v;
}
#endif




