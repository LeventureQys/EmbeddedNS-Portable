Project Security Level:TopSecurity

项目涉密等级:绝密

# ENC_Webrtc 来自WebRtc的降噪项目

制作此代码，是用于后续在单片机以及在别的设备上进行移植所使用，所以这里需要将整个代码从C++移植到纯C类型

## 文件夹及说明文件

| 文件夹名称   | 更新时间   | 作者    | 说明 |
| :-------------: | :-----------: | ------------: |------------:|
| ENC_NotGoodEnough | 2025.2.2|Venture|一个另外的Noise Suppresor 实现方式，其中三分频的实现有所变化|
|   Original   | 2025.2.2      | Venture     | C++ noise suppresor 满血版本|
|  PureC   | 2025.2.2      | Wendy     | 修改后的满血C语言版本 |
|   Simplify   | 2025.2.2      | Wendy     | 阉割版的Noise Suppresor|
| Webrtc_Source | 2025.2.18 | Venture | 一个webrtc的完整版源码 |
| SplitFilter | 2025.2.20 | Wendy | 迁移C语言的新分频器实现 |


## 项目说明

本项目为WebRtc下属的NoiseSuppressor 项目单独拆解，我们尝试将这个项目进行各种类型的移植和阉割，以满足我们对降噪算法的要求。

降噪效果展示：

未降噪：

![企业微信截图_17400148087576](https://raw.githubusercontent.com/LeventureQys/Picturebed/main/image/企业微信截图_17400148087576.png)

降噪后：
![20250220163058](https://raw.githubusercontent.com/LeventureQys/Picturebed/main/image/20250220163058.png)

其效果可以说相当显著，目前已经完成了C语言端的移植。


## 编译说明

需要编译Project下的项目，使用CMake直接编译即可，不需要任何依赖。其中ENC_Core为NoiseSuppressor库
