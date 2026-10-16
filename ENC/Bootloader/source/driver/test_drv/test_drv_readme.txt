1、test_drv说明:

A、test_drv外设模块有：UART, SPI, KEY_IO, KEY_ADC, IIC, PWM, EXTI模块

B、test_drv调用drv层的函数接口用来测试drv层的函数接口功能，用户可以参考test_drv例程去使用drv层提供的函数接口实现功能

2、test_drv使用方法：

A、demo.c  和  test_drv.c 只能选择其一进行编译

B、找到 conf_demo_or_test.h 头文件，打开  #define CONF_SEL_TEST_DRV  关闭  #define CONF_SEL_DEMO

C、找到 conf_demo_or_test.h 头文件，把用到的test_drv模块加进工程编译，使能使用模块的宏定义即可

D、找到 conf_dev_en.h 头文件，把用到的drv模块加进工程编译，使能使用模块的宏定义即可

F、编译