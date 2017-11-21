alink 是由阿里推出的智能硬件模组、阿里智能云、阿里智能 APP，阿里智能生活事业部为厂商提供一站式设备智能化解决方案。
本工程对接 alink embed 版本，并对阿里底层交互流程进行封装，你无需关心设备具体是如何进行交互的、升级的，对如果更改相关配置请使用 make menuconfig。

# Feature:
1. 配网(一键配网、热点配网、零配网)
2. 绑定、解绑
3. OTA 升级
4. 数据传输非透传

# 开发流程
1. 熟悉[esp32_idf的使用](http://esp-idf.readthedocs.io/en/latest/?badge=latest)
2. 了解[阿里智能](https://open.aliplus.com/docs/open)相关知识
3. 跑通[alink light demo](https://github.com/espressif/esp32-alink-demo)，初步了解 alink 相关功能
4. [签约入驻](https://open.aliplus.com/docs/open/open/enter/index.html)
5. [产品注册](https://open.aliplus.com/docs/open/open/register/index.html)
6. 产品开发
    - **初始化**, 调用 alink_init() 传入产品注册的信息，注册事件回调函数
    - **配网**，设备配网过程中的所有动作，都会传入事件回调函数中
        - 当设备连上服务器时，需要主动上报设备状态
        - 设备配网过程，需要发送激活命令

            > 注：事件回调函数的 task 默认大小为 4k, 请避免在事件回调函数使用较大的栈空间
    - **数据通信**
        - 接收数据，接收到的数据由设置指令和请求指令组成
        - 发送数据，若发送的数据大于 512Byte，请修改配置文件，数据长度最大支持 1.5k
    - **底层驱动**
        - 设置 GPIO、串口等外围设备，参见：http://esp-idf.readthedocs.io/en/latest/api-reference/peripherals/index.html

            > 注： alink api 的使用参见：esp32-alink-demo：https://github.com/espressif/esp32-alink-demo
7. [发布上架](https://open.aliplus.com/docs/open/open/publish/index.html)

# 配置与编译
2. **配置 idf**, 使用make menuconfig,
    - 禁用硬件 SHA: Component config->mbedTLS->SHA
    - 使能 SO_REUSEADDR: Component config->LWIP->Enable SO_REUSEADDR option
    - 提高 cpu 频率：Component config->ESP32-specific->CPU frequency (240 MHz)
    - 配置分区表：Partition Table->Factory app, two OTA definitions
3. **配置alink**
    - 使用 make menuconfig，配置日志等级，数据传输模式、任务优先级等，推荐使用默认值，make menuconfig->Component config->Enable alink_v2.0 function
    
        > 注：如果你想要详细了解esp32相关配置与编译详见esp-idf/README.md

# Related links
* ESP32 概览 : http://www.espressif.com/en/products/hardware/esp32/overview
* ESP32 资料 : http://www.espressif.com/en/products/hardware/esp32/resources
* 烧录工具  : http://espressif.com/en/support/download/other-tools
* 串口驱动  : http://www.silabs.com/products/mcu/Pages/USBtoUARTBridgeVCPDrivers.aspx
* 阿里智能开放平台：https://open.aliplus.com/docs/open/
