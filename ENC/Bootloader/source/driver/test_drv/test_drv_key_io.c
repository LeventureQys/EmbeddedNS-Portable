#ifndef __TEST_DRV_KEY_IO_C_
#define __TEST_DRV_KEY_IO_C_

#include "drv_key_io.h"
#include "test_drv_key_io.h"

#define KEYIOPIN1  (0)
#define KEYIOPIN2  (1)

void TestDrvKeyEventCallback1(void);
void TestDrvKeyEventCallback2(void);
void TestDrvKeyEventCallback3(void);
void TestDrvKeyEventCallback4(void);
void TestDrvKeyEventCallback5(void);
void TestDrvKeyEventCallback6(void);

FUNCP keyOffCbTb[] = 
{
  TestDrvKeyEventCallback1,
  TestDrvKeyEventCallback2,
};

FUNCP keyOnCbTb[] = 
{
    TestDrvKeyEventCallback3,
    TestDrvKeyEventCallback4,
};

FUNCP keyLonPressCbTb[] = 
{
    TestDrvKeyEventCallback5,
    TestDrvKeyEventCallback6,
};

uint32_t keyIoInfoTb[2] = 
{
    KEYIOPIN1,KEYIOPIN2//GPIO0_BIT0, GPIO0_BIT1,
};

KEY_IO_PARA_TypeDef keyIo = {
    .keyPressedInMs = 2000,	//长按响应的时间
    .ioNum = {2, 0},//port口对应使用的PIN的数量，不用写0
    .pIoInfo = {keyIoInfoTb, 0}, //数组[0]:写入GPIO0对应的PIN值;数组[1]:写入GPIO1对应的PIN值，不用写0
    .pKeyOnCb = {keyOffCbTb, 0}, //按下执行回调函数，不用写0
    .pKeyOffCb = {keyOffCbTb, 0}, //释放执行回调函数，不用写0
    .pKeyPressedCb = {keyLonPressCbTb, 0 }, //长按执行函数，不用写0
};

//按键初始化
void TestDrvKeyInit(void)
{
    DrvKeyIoInit(&keyIo); //独立按键初始化
}

//按键扫描
void TestDrvKeyIoProcess(void)
{
    DrvKeyIoProcess(&keyIo);//独立按键扫描
}

void TestDrvKeyIo(void)
{	
	//按键初始化
	TestDrvKeyInit();	
	while(1)
	{	
		//主循环调用按键扫描
		TestDrvKeyIoProcess();		
	}	
}

void TestDrvKeyEventCallback1(void)
{
    return;
}

void TestDrvKeyEventCallback2(void)
{
    return;
}

void TestDrvKeyEventCallback3(void)
{
    return;
}

void TestDrvKeyEventCallback4(void)
{
    return;
}

void TestDrvKeyEventCallback5(void)
{
    return;
}

void TestDrvKeyEventCallback6(void)
{
    return;
}

#endif
