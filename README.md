# RTIO Device SDK for Embedded C

> English | [简体中文](./README-CN.md)  
> The author's native language is Chinese. This document is translated using AI.  

- [RTIO Device SDK for Embedded C](#rtio-device-sdk-for-embedded-c)
  - [Demo Compilation on Linux](#demo-compilation-on-linux)
  - [Running the Demo](#running-the-demo)
    - [Running RTIO Server](#running-rtio-server)
      - [Running RTIO via Docker](#running-rtio-via-docker)
      - [Running RTIO from Source](#running-rtio-from-source)
    - [Running `rtio_demo_simple`](#running-rtio_demo_simple)
    - [Running `rtio_demo_tls`](#running-rtio_demo_tls)
  - [SDK Integration](#sdk-integration)
  - [Acknowledgements](#acknowledgements)

IoT device SDK in C for RTIO service connectivity.

## Demo Compilation on Linux

Install build tools and demo dependencies on Debian-11, for example.

```sh
sudo apt updte
sudo apt install gcc g++ make cmake
sudo apt install openssl libssl-dev
```

Compile the demo.

```sh
$ cd rtio-device-sdk-c/
$ cmake -S . -B build
$ cd build/
$ make
# ...
[100%] Linking C executable ...
```

## Running the Demo

### Running RTIO Server

You can run the RTIO server either through "Docker" or by compiling from source.

#### Running RTIO via Docker

```sh
$ sudo docker pull registry.cn-guangzhou.aliyuncs.com/rtio/rtio:v0.8.0
v0.8.0: Pulling from rtio/rtio
...
Digest: sha256:...

$ sudo docker run  --rm -p 17017:17017 -p 17917:17917 registry.cn-guangzhou.aliyuncs.com/rtio/rtio:v0.8.0
2024-06-03 13:12:23.264 INF rtio starting ...
```

You can log into the container using the following command, for example, to view the command help.

```sh
$ sudo docker run -it --rm --entrypoint /bin/sh  -p 17017:17017 -p 17917:17917 registry.cn-guangzhou.aliyuncs.com/rtio/rtio:v0.8.0
/home/rainbow $ ./rtio -h
Usage of ./rtio: 
...
```

（Use "$" instead of "#" in code blocks to avoid the command being rendered as a comment. The same applies to the following text.）

#### Running RTIO from Source

Tools required:

- Golang: version 1.21.0 or higher (installation steps: <https://go.dev/doc/install>).
- GNU Make: version 4.3 or higher recommended.
- Git.

Get the code.

```sh
$ git clone https://github.com/mkrainbow/rtio.git
$ cd rtio/
$ make
$ ls ./out/
examples  rtio
```

Run the service using the following command, and use `./out/rtio -h` for help.

```sh
$ ./out/rtio -disable.deviceverify -disable.hubconfiger -log.level info
2024-12-19 17:07:14.198 INF rtio starting ...
```

### Running `rtio_demo_simple`

```sh
$ cd bin/
$ ./rtio_demo_simple
[INFO] [DEMO_RTIO] [1734767058] [rtio_demo_simple.c:64 main] RTIO server=localhost:17017.
[DEBUG] [DEMO_RTIO] [1734767058] [rtio_demo_simple.c:66 main] The fixed RAM used by RTIO, size=2072 bytes.
```

In another terminal, use `curl` to simulate a request to the device URI /rainbow. Here, the `curl` request sends "hello", and the response is "World!" (since the data field is binary, it is base64-encoded before HTTP transmission).

```sh
$ echo -n "Hello"| base64
SGVsbG8=

$ curl http://localhost:17917/cfa09baa-4913-4ad7-a936-3e26f9671b10 -d '{"method":"copost", "uri":"/rainbow","id":12668,"data":"aGVsbG8="}'
{"id":12668,"fid":0,"code":"OK","data":"V29ybGQh"}

$ echo -n "V29ybGQh"| base64 -d
World!
```

The device-side log shows the received request data "Hello".

```sh
[INFO] [DEMO_RTIO] [1734767083] [rtio_demo_simple.c:31 uriRainbow] Handling the copost req with uri=/rainbow, pReqData=hello.
```

The demo successfully sent a `copost` request to the device's `/rainbow` endpoint, and successfully transmitted the request and response data (the device doesn't need to base64 encode/decode, the `rtio service` handles the conversion).

### Running `rtio_demo_tls`

The RTIO server needs to enable TLS device access. The following example uses Docker.

First, enter interactive mode.

```sh
$ sudo docker run -it --entrypoint /bin/sh  --rm  -p 17017:17017 -p 17917:17917 registry.cn-guangzhou.aliyuncs.com/rtio/rtio:v0.8.0
/home/rainbow $ ls
certificates        rtio     # ...
```

Run RTIO and enable TLS.

```sh
/home/rainbow $ ./rtio -disable.deviceverify -disable.hubconfiger -enable.hub.tls -hub.tls.certfile ./certificates/demo_server.crt -hub.tls.keyfile ./certificates/demo_server.key -log.level info
2024-12-24 09:39:38.337 INF rtio starting ...
2024-12-24 09:39:38.337 INF TLS access enabled
```

In another terminal, run `rtio_demo_tls`. Note that it should be run from the `build/bin/` directory, as the code references the certificate location `certificates/RTIORootCA.crt`.

```sh
$ cd bin/
$ ./rtio_demo_tls
[INFO] [DEMO_RTIO_TLS] [1735034352] [rtio_demo_tls.c:77 main] RTIO server=localhost:17017.
[DEBUG] [DEMO_RTIO_TLS] [1735034352] [rtio_demo_tls.c:79 main] The fixed RAM used by RTIO, size=2072 bytes.
```

In another terminal, use `curl` to simulate a request to the device URI `/rainbow`. Here, the `curl` request sends "hello", and the response is "World!" (since the data field is binary, it is base64-encoded before HTTP transmission).

```sh
$ curl http://localhost:17917/cfa09baa-4913-4ad7-a936-3e26f9671b10 -d '{"method":"copost", "uri":"/rainbow","id":12668,"data":"aGVsbG8="}'
{"id":12668,"fid":0,"code":"OK","data":"V29ybGQh"}
```

The device-side log shows the received request data "Hello".

```sh
[INFO] [DEMO_RTIO_TLS] [1735034642] [rtio_demo_tls.c:37 uriRainbow] Handling the copost req with uri=/rainbow, pReqData=hello.
```

## SDK Integration

The following example is for integrating with the POSIX platform using `rtio_demo_simple`. Copy `rtio_demo_simple` and `rtio-device-sdk-c` into the same directory.

```sh
$ cp -r rtio-device-sdk-c/demos-posix/rtio/rtio_demo_simple ./ 
$ ls -d rtio-device-sdk-c rtio_demo_simple 
rtio-device-sdk-c  rtio_demo_simple
$ cd rtio_demo_simple/
$ ls
CMakeLists.txt  core_rtio_config.h  demo_config.h  rtio_demo_simple.c
```

Modify the `CMakeLists.txt` to reference the required cmake files.

```cmake
# ...
# Get root dirs and the cmake-files it depends on.
include( "../../demosRoot.cmake" )
include( "${RTIO_SDK_ROOT_DIR}/tools/cmake/rtioSDK_POSIX_Plaintext.cmake" )
include( "${PLATFORM_DEMOS_ROOT_DIR}/logging-stack/logging.cmake" )
```

Replace with the following:

```cmake
# ...
# Get root dirs and the cmake-files it depends on.
include( "../rtio-device-sdk-c/tools/cmake/rtioSDK_POSIX_Plaintext.cmake" )
include( "../rtio-device-sdk-c/demos-posix/logging-stack/logging.cmake" )
```

Use `cmake` and `make` to build the program:

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

For non-POSIX platforms, refer to the Demo in `demos-esp32` for setup instructions.

## Acknowledgements

This project is developed based on [aws-iot-device-sdk-embedded-C](https://github.com/aws/aws-iot-device-sdk-embedded-C/tree/202211.00?tab=readme-ov-file) (202211.00, MIT license), with the coreRTIO project added. Thanks to the project authors and contributors.