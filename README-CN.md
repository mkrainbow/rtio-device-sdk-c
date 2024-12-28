# RTIO设备SDK-C语言

- [RTIO设备SDK-C语言](#rtio设备sdk-c语言)
  - [Linux环境Demo编译](#linux环境demo编译)
  - [运行Demo](#运行demo)
    - [运行RTIO服务端](#运行rtio服务端)
      - [通过Docker运行RTIO](#通过docker运行rtio)
      - [编译源码运行RTIO](#编译源码运行rtio)
    - [运行`rtio_demo_simple`](#运行rtio_demo_simple)
    - [运行`rtio_demo_tls`](#运行rtio_demo_tls)
  - [SDK集成](#sdk集成)
  - [致谢](#致谢)

> 简体中文 | [English](./README.md)  

C语言版的物联网设备端SDK，用于连接RTIO服务。

## Linux环境Demo编译

以Debian-11为例安装编译工具和Demo依赖库。

```sh
sudo apt updte
sudo apt install gcc g++ make cmake
sudo apt install openssl libssl-dev
```

编译Demo。

```sh
$ cd rtio-device-sdk-c/
$ cmake -S . -B build
$ cd build/
$ make
...
[100%] Linking C executable ...
```

## 运行Demo

### 运行RTIO服务端

可通过“Docker”或者“源码编译”方式运行RTIO服务。

#### 通过Docker运行RTIO

```sh
$ sudo docker pull registry.cn-guangzhou.aliyuncs.com/rtio/rtio:v0.8.0
v0.8.0: Pulling from rtio/rtio
...
Digest: sha256:...

$ sudo docker run  --rm -p 17017:17017 -p 17917:17917 registry.cn-guangzhou.aliyuncs.com/rtio/rtio:v0.8.0
2024-06-03 13:12:23.264 INF rtio starting ...
```

通过以下命令登录容器中，比如查看命令帮助。

```sh
$ sudo docker run -it --rm --entrypoint /bin/sh  -p 17017:17017 -p 17917:17917 registry.cn-guangzhou.aliyuncs.com/rtio/rtio:v0.8.0
/home/rainbow $ ./rtio -h
Usage of ./rtio: 
...
```

（代码块中使用"$"代替"#",避免命令被渲染成注释，下文相同。）

#### 编译源码运行RTIO

工具：

- Golang：版本1.21.0或以上(安装步骤：<https://golang.google.cn/doc/install>)。
- GNU Make：建议版本4.3或以上。
- Git.

获取代码。

```sh
$ git clone https://github.com/mkrainbow/rtio.git
$ cd rtio/
$ make
$ ls ./out/
examples  rtio
```

通过以下命令运行服务，可通过`./out/rtio -h`查看帮助。

```sh
$ ./out/rtio -disable.deviceverify -disable.hubconfiger -log.level info
2024-12-19 17:07:14.198 INF rtio starting ...
```

### 运行`rtio_demo_simple`

```sh
$ cd bin/
$ ./rtio_demo_simple
[INFO] [DEMO_RTIO] [1734767058] [rtio_demo_simple.c:64 main] RTIO server=localhost:17017.
[DEBUG] [DEMO_RTIO] [1734767058] [rtio_demo_simple.c:66 main] The fixed RAM used by RTIO, size=2072 bytes.
```

打开另一终端运行`curl`模拟请求到设备URI`/rainbow`。如下，`curl`请求“hello”，响应为“World!”（因data字段为二进制，HTTP传输前采用base64编码）。

```sh
$ echo -n "Hello"| base64
SGVsbG8=

$ curl http://localhost:17917/cfa09baa-4913-4ad7-a936-3e26f9671b10 -d '{"method":"copost", "uri":"/rainbow","id":12668,"data":"aGVsbG8="}'
{"id":12668,"fid":0,"code":"OK","data":"V29ybGQh"}

$ echo -n "V29ybGQh"| base64 -d
World!
```

设备端日志如下，显示收到请求数据“Hello”。

```sh
[INFO] [DEMO_RTIO] [1734767083] [rtio_demo_simple.c:31 uriRainbow] Handling the copost req with uri=/rainbow, pReqData=hello.
```

以上演示通过`copost`请求到设备端的`/rainbow`，并成功传输请求和返回响应数据（设备端无须base64编解码，`rtio服务`会做转换）。

### 运行`rtio_demo_tls`

RTIO服务端需要启用TLS设备接入，以下以docker方式为例。

首先，进入交互模式。

```sh
$ sudo docker run -it --entrypoint /bin/sh  --rm  -p 17017:17017 -p 17917:17917 registry.cn-guangzhou.aliyuncs.com/rtio/rtio:v0.8.0
/home/rainbow $ ls
certificates        rtio     # ...
```

运行RTIO，并使能TLS。

```sh
/home/rainbow $ ./rtio -disable.deviceverify -disable.hubconfiger -enable.hub.tls -hub.tls.certfile ./certificates/demo_server.crt -hub.tls.keyfile ./certificates/demo_server.key -log.level info
2024-12-24 09:39:38.337 INF rtio starting ...
2024-12-24 09:39:38.337 INF TLS access enabled
```

另一终端运行`rtio_demo_tls`,注意在编译`build/bin/`目录下运行，因为代码中证书位置指向`certificates/RTIORootCA.crt`。

```sh
$ cd bin/
$ ./rtio_demo_tls
[INFO] [DEMO_RTIO_TLS] [1735034352] [rtio_demo_tls.c:77 main] RTIO server=localhost:17017.
[DEBUG] [DEMO_RTIO_TLS] [1735034352] [rtio_demo_tls.c:79 main] The fixed RAM used by RTIO, size=2072 bytes.
```

打开另一终端运行`curl`模拟请求到设备URI`/rainbow`。如下，`curl`请求“hello”，响应为“World!”（因data字段为二进制，HTTP传输前采用base64编码）。

```sh
$ curl http://localhost:17917/cfa09baa-4913-4ad7-a936-3e26f9671b10 -d '{"method":"copost", "uri":"/rainbow","id":12668,"data":"aGVsbG8="}'
{"id":12668,"fid":0,"code":"OK","data":"V29ybGQh"}
```

设备端日志如下，显示收到请求数据“Hello”。

```sh
[INFO] [DEMO_RTIO_TLS] [1735034642] [rtio_demo_tls.c:37 uriRainbow] Handling the copost req with uri=/rainbow, pReqData=hello.
```

## SDK集成

以下以POSIX平台`rtio_demo_simple`为例。拷贝`rtio_demo_simple`与`rtio-device-sdk-c`在同一个目录中。

```sh
$ cp -r rtio-device-sdk-c/demos-posix/rtio/rtio_demo_simple ./
$ ls -d rtio-device-sdk-c rtio_demo_simple 
rtio-device-sdk-c  rtio_demo_simple
$ cd rtio_demo_simple/
$ ls
CMakeLists.txt  core_rtio_config.h  demo_config.h  rtio_demo_simple.c
```

修改CMakeLists.txt，使其引用到依赖的cmake文件。

```cmake
# ...
# Get root dirs and the cmake-files it depends on.
include( "../../demosRoot.cmake" )
include( "${RTIO_SDK_ROOT_DIR}/tools/cmake/rtioSDK_POSIX_Plaintext.cmake" )
include( "${PLATFORM_DEMOS_ROOT_DIR}/logging-stack/logging.cmake" )
```

替换如下：

```cmake
# ...
# Get root dirs and the cmake-files it depends on.
include( "../rtio-device-sdk-c/tools/cmake/rtioSDK_POSIX_Plaintext.cmake" )
include( "../rtio-device-sdk-c/demos-posix/logging-stack/logging.cmake" )
```

使用`cmake`和`make`编译程序:

```sh
$ cmake -S . -B build
# ...
-- Configuring done
-- Generating done
-- Build files have been written to: ...

$ cd build/
$ make
# ...
[100%] Built target rtio_demo_simple
```

非POSIX平台，可参考demos-esp32中Demo进行设置。

## 致谢

该项目基于 [aws-iot-device-sdk-embedded-C](https://github.com/aws/aws-iot-device-sdk-embedded-C/tree/202211.00?tab=readme-ov-file) (202211.00, MIT license)开发（主要增加coreRTIO项目），感谢项目作者及贡献者。
