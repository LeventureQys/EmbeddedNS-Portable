更新库注意事项：
1. 确认Hal目录下的Hal层代码版本和当前应用SDK一致，如不一致，请联系菁音并进行确认！！
2. Headers及文件夹内文件直接更新替换，且不允许用户自行更改
3. drv_uac10.lib直接更新替换，且不允许用户自行更改
4. 其他.c/.h文件，如果用户没有更改过，可则直接覆盖替换
5. drv_uac10_user_config.h文件中的define，用户可根据实际需求，做适当更改，如不清楚，请联系菁音确认再更改，切勿盲目更改！！

V1.2.0版本及以上库更新说明：
[文件说明]
drv_uac10_user_config.h:    用户对驱动层代码进行配置，除了该文件外，其他文件请谨慎修改或不更改
drv_uac10_hid.c:            HID相关函数以及参考应用例程，如用户需要添加代码，建议在其他文件中添加，通过接口函数调用
drv_uac10_midi.c:           MIDI相关函数以及参考应用例程，如用户需要添加代码，建议在其他文件中添加，通过接口函数调用
drv_uac10.c./h:             库驱动和应用层的接口相关函数，不可随意更改
drv_uac10_spk.c:            SPK相关函数，不可随意更改
drv_uac10_mic.c:            MIC相关函数，不可随意更改
drv_uac10_filter.c/.h:      96K/192K录音时需要用到的抗混叠滤波器，不可随意更改
Headers:                    静态库头文件，可通过config文件得知该库版本及配置，如需要进行版本更新，请将Headers打包发给菁音升级库包，不可随意更改
drv_uac10.lib:              静态库文件
[函数接口]
<1> 初始化
    <1.1> DrvUsbHidUserParaHandlerCallbackInstall，加载hid通信处理函数，一般用于PC调音工具对应的效果调试处理，是V1.2.0库新增函数接口
    <1.2> DrvUac10Init，对应原函数AppUsbDevInit
<2> 主循环
    <2.1> DrvUac10Process, 对应原函数AppUsbDevProcess
<3> 音频帧中断（DMA中断）
    <3.1> DrvUac10FrameProcess，对应原函数AppUsbDevFrameProcess，且内部包括了检查USB-SPK数据流断流的函数调用DrvUac10SpkStreamingCheck，原函数AppUsbDevSpkStreamingCheck
    <3.2> DrvUac10SpkDataRead，DrvUac10MicDataWrite，分别从SPK拿数据和往MIC写数据，对应原函数AppUsbDevSpkDataRead，AppUsbDevMicDataWrite
    <3.3> DrvUac10MicVolHandler, DrvUac10SidetoneVolHandler, DrvUac10SpkVolHandler，新增3个音量控制函数，将原来放在usb中断中直接处理音量的拉出来，由用户控制处理音量所处的数据流位置，且这3个音量包含了mute信息支持渐变处理
<4> IRQ中断
    <4.1> DrvUac10IrqHandler, 对应原函数AppUsbDevIrqHandler
