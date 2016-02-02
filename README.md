# bt_gateway

　使用 [**yunba-c-sdk**](https://github.com/yunba/yunba-c-sdk.git) 就可以轻松实现 Bluetooth gateway ，让智能设备接入互联网功能。
　
## 蓝牙设备
  
硬件平台：[Linklt One](http://www.seeedstudio.com/wiki/LinkIt_ONE%E5%BC%80%E5%8F%91%E6%9D%BF)

相应的蓝牙控制 LED [代码](https://github.com/alexbank/bt_demo.git)

## gateway 环境搭建

需要安装 bluetooth 服务。

如果在 ubuntu 平台，可以：

```
apt-get install bluetooth
```

如果是嵌入式平台需要交叉编译 bluetooth。

修改文件　

```
/etc/bluetooth/rfcomm.conf
```

中对应的 address 以及 channel

##　步骤

１）下载 [**yunba-c-sdk**](https://github.com/yunba/yunba-c-sdk.git) 编译出库文件。

２）使用 eclipse 创建工程文件，包含添加 yunba-c-sdk 头文件以及库文件路径等。

３）参考 [code](https://github.com/yunba/yunba-c-sdk/blob/master/src/samples/stdinpub_present.c)

４）添加对蓝牙设备的控制，这里我们实现了通过蓝牙对 LED 进行开关操作
