# lib

Custom libraries

## SimpleFTPServer

Modified the following:

### `FtpServerKey.h`

```diff
// esp32 configuration
#ifndef DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP32
-   #define DEFAULT_STORAGE_TYPE_ESP32 STORAGE_FFAT
+   #define DEFAULT_STORAGE_TYPE_ESP32 STORAGE_SD
```

### `FtpServer.h`

```diff
- <SPI.h>
- <SD.h>
+ <SD_MMC.h>

- STORAGE_MANAGER SD
+ STORAGE_MANAGER SD_MMC
```


### `main.cpp`
Ah yes i use 1-bit mode by setting SD_MMC.begin("/sdcard", false);

See [xreef/SimpleFTPServer#28](https://github.com/xreef/SimpleFTPServer/issues/28#issuecomment-1202299645).
