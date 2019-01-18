# CCS811 flash
Arduino sketch to flash an CCS811 with firmware 2.0.0.

DISCLAIMER: FLASHING FIRMWARE CARRIES THE RISK OF BRICKING YOUR CCS811. 
I CANNOT BE HELD RESPONSIBLE IN THE EVENT SOMETHING GOES WRONG WITH 
YOUR DEVICE DURING THE PROCESS OF FOLLOWING/USING THIS EXAMPLE. 

The risk is low, since you only flash the application firmware, not the boot loader.


## Introduction
The CCS811 has an internal micro controller with a boot loader and the gas application.
As the [datasheet](http://ams.com/ccs811) explains the boot loader can be used to replace the gas application.
Since most Chinese boards come with firmware 1.1.0, and ams has released firmware 2.0.0, I have written 
this example: it flashes 2.0.0 into a CCS811.


## Process
I have downloaded the 2.0.0 firmware from the ams pages
[CCS811_SW000246_1-00.bin](http://ams.com/ccs811#tab/tools).
Note that this file has `1-00` as version, but that the description 
specifies `CCS811 Application firmware version 2.0.0`.

I have written a Python script [hex.py](hex.py).
I used it to convert the binary firmware file to a C-array as follows
```
python hex.py CCS811_SW000246_1-00.bin > CCS811_SW000246_1-00.h
```
The resulting text file `CCS811_SW000246_1-00.h` starts like this
```
// Hex dump of 'CCS811_SW000246_1-00.bin' created at 2018-12-07 17:18:12.111495

#include <stdint.h>

const char * image_name="CCS811_SW000246_1-00.bin";
const uint8_t image_data[] PROGMEM = {
  0x5c, 0x1a, 0xdd, 0xff, 0x15, 0x25, 0xdd, 0x66,   0xfc, 0x88, 0x80, 0x49, 0x02, 0xdc, 0x17, 0x10, 
  0x1f, 0x73, 0x60, 0xae, 0xf4, 0xa0, 0x5d, 0xda,   0xcd, 0xca, 0x94, 0xc6, 0x6b, 0x96, 0x4f, 0xea, 
  0xed, 0xbd, 0xde, 0xc2, 0xea, 0xc1, 0x44, 0x8b,   0xf3, 0x7a, 0x4d, 0x61, 0x22, 0x83, 0xe5, 0xac, 
  0x3c, 0xb1, 0x44, 0x5f, 0x91, 0xa2, 0xdc, 0xd2,   0x40, 0x63, 0x1c, 0x11, 0x60, 0x10, 0x33, 0xe7, 
```

Finally, I have written a sketch [ccs811flash.ino](ccs811flash.ino) that uses this array to 
flash the CSS811.


## Example
I hooked CCS811 with 1.1.0 firmware to the ESP8266.
The script gave this output:
```
Starting CCS811 flasher
setup: library version: 9
setup: hardware    version: 12
setup: bootloader  version: 1000
setup: application version: 1100
setup: starting flash of 'CCS811_SW000246_1-00.bin' in 5 seconds

ccs811: ping ok
ccs811: reset ok
ccs811: status (reset1) 10 ok
ccs811: app-erase ok
ccs811: status (app-erase) 40 ok
ccs811: writing 5120 ................................................................ 4608
ccs811: writing 4608 ................................................................ 4096
ccs811: writing 4096 ................................................................ 3584
ccs811: writing 3584 ................................................................ 3072
ccs811: writing 3072 ................................................................ 2560
ccs811: writing 2560 ................................................................ 2048
ccs811: writing 2048 ................................................................ 1536
ccs811: writing 1536 ................................................................ 1024
ccs811: writing 1024 ................................................................ 512
ccs811: writing 512 ................................................................ 0
ccs811: app-verify ok
ccs811: status (app-verify) 30 ok
ccs811: reset2 ok
ccs811: status (reset2) 10 ok

loop: ended ...
loop: ended ...
```

Then I pressed reset, and this was the output
```
Starting CCS811 flasher
init: hardware    version: 12
init: bootloader  version: 1000
init: application version: 2000
init: already has 2.0.0
loop: ended ...
loop: ended ...
```

## A brick?
I once aborted flashing half way, which means you end up with a CCS811 without a valid app.
The bootloader is still there, so no worries. 
This is what a refresh looks like.

```
Starting CCS811 flasher
setup: library version: 9
ccs811: Wrong HW_ID: FD
setup: CCS811 begin FAILED
setup: hardware version: FD
setup: bootloader version: FDFD
setup: application version: FDFD
setup: starting flash of 'CCS811_SW000246_1-00.bin' in 5 seconds

ccs811: ping ok
ccs811: reset ok
ccs811: status (reset1) 20 ERROR - ignoring
ccs811: app-erase ok
ccs811: status (app-erase) 40 ok
ccs811: writing 5120 ................................................................ 4608
ccs811: writing 4608 ................................................................ 4096
ccs811: writing 4096 ................................................................ 3584
ccs811: writing 3584 ................................................................ 3072
ccs811: writing 3072 ................................................................ 2560
ccs811: writing 2560 ................................................................ 2048
ccs811: writing 2048 ................................................................ 1536
ccs811: writing 1536 ................................................................ 1024
ccs811: writing 1024 ................................................................ 512
ccs811: writing 512 ................................................................ 0
ccs811: app-verify ok
ccs811: status (app-verify) 30 ok
ccs811: reset2 ok
ccs811: status (reset2) 10 ok

loop: ended ...
loop: ended ...
```


## Success rate
Table with successful flashes.

| # | Person            | When         | Lib      | Board           | Host                | IDE         |
|:-:|:-----------------:|:------------:|:--------:|:---------------:|:-------------------:|:-----------:|
| 1 | @maarten-pennings | 2018 Dec  5  | v8       | ams eval kit    | ESP8266 (Robotdyn)  |  Arduino    |
| 2 | @maarten-pennings | 2018 Dec  7  | v8       | CJMCU-811       | ESP8266 (Robotdyn)  |  Arduino    |
| 3 | @bfaliszek        | 2018 Dec 17  | v8       | CJMCU-811       | ESP8266             |  Arduino    |
| 4 | @rovale           | 2018 Dec 28  | v8       | CJMCU-811       | ESP32 (LOLIN D32)   |  Arduino    |
| 5 | @bertrik          | 2019 Jan 03  | v8       | CJMCU-811       | ESP8266 (d1 mini)   |  platformio |
| 6 | @bfaliszek        | 2019 Jan 04  | v8       | CCS811/HDC1080  | ESP8266             |  Arduino    |
| 7 | Kees M            | 2019 Jan 13  | v8       | CJMCU-811       | Arduino nano        |  Arduino    |
| 8 | @maarten-pennings | 2019 Jan 18  | v9       | ams eval kit    | Arduino nano        |  Arduino    |
| 9 | @maarten-pennings | 2019 Jan 18  | v9       | CJMCU-811       | ESP8266 (Wemos)     |  Arduino    |


(end of doc)
