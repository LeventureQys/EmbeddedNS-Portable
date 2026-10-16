
#include "drv_key_io.h"
#include "drv_matrix_key.h"
#include "demo_key_io.h"
#define KEYIOPIN1  (0)
#define KEYIOPIN2  (1)

void app_event_null2(void)
{
    return;
}

void app_event_null3(void)
{
    return;
}
uint32_t keyIoInfoTb[2] = 
{
    KEYIOPIN1,KEYIOPIN2//BIT0, BIT1,
};
FUNCP keyOffCbTb[] = 
{
  app_event_null2,
};


FUNCP keyNullCbTb[] = 
{
    app_event_null3,
    app_event_null3,
};
KEY_IO_PARA keyIo = {
    .keyPressedInMs = 2000,
    .ioNum = {2, 0},
    .pIoInfo = {keyIoInfoTb, 0},
    .pKeyOnCb = {keyNullCbTb, 0},
    .pKeyOffCb = {keyOffCbTb, 0},
    .pKeyPressedCb = {keyNullCbTb, 0 },
};


//按键初始化
void app_key_init(void)
{
    drv_key_io_init(&keyIo); //独立按键初始化
    drv_matrix_key_Init(); //矩阵按键初始化
}

//按键扫描
void app_key_io_process(void)
{
    drv_key_io_process(&keyIo);//独立按键扫描
    drv_matrix_key_scan();  //矩阵按键扫描

}
