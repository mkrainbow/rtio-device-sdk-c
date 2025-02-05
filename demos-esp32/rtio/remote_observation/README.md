## Overview

Test the following content with ESP32-C6-DevKitC-1:

- Turn on/off an LED remotely via copost
-  Observe the specified GPIO level via obget

## Build Steps

```sh
idf.py set-target esp32c6
```
```sh
idf.py menuconfig
```

Modify following tent.

```text
(myssid) WiFi SSID
(mypassword) WiFi Password
(demo.rtio.mkrainbow.com) RTIO Service HOST
```
```sh
idf.py build
idf.py flash
idf.py monitor
```