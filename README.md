# bt_gateway

　使用 [**yunba-c-sdk**](https://github.com/yunba/yunba-c-sdk.git) 实现 Bluetooth gateway 功能。
　
## 环境搭建
  
蓝牙设备：
[Linklt One](http://www.seeedstudio.com/wiki/LinkIt_ONE%E5%BC%80%E5%8F%91%E6%9D%BF)
[代码](https://github.com/alexbank/bt_demo.git)

PC端：
带有蓝牙模块的。

在 ubuntu 上，
```bash
apt-get install bluetooth
```

修改文件　

```
/etc/bluetooth/rfcomm.conf
```

##　步骤

１）下载 [**yunba-c-sdk**](https://github.com/yunba/yunba-c-sdk.git) 编译出库文件。

２）使用 eclipse 创建工程文件，包含添加 yunba-c-sdk 头文件以及库文件路径等。

３）参考 [code](https://github.com/yunba/yunba-c-sdk/blob/master/src/samples/stdinpub_present.c)

４）添加对蓝牙设备的控制，这里我们实现了通过蓝牙对 LED 进行开关操作