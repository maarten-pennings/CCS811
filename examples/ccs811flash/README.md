# CCS811 flash
Arduino sketch to flash an CCS811 with firmware 2.0.0.

DISCLAIMER: FLASHING FIRMWARE CARRIES THE RISK OF BRICKING YOUR CCS811. 
I CANNOT BE HELD RESPONSIBLE IN THE EVENT SOMETHING GOES WRONG WITH 
YOUR DEVICE DURING THE PROCESS OF FOLLOWING/USING THIS EXAMPLE. 

The risk is low, since you only flash the application firmware, not the boot loader (see e.g. [brick](#a-brick) below).

Note that from version 2.0.0 ams has this change note "Removed NTC functionality. Pin 8 not measured and left undriven.".
So if your application uses the onboard thermistor, do not upgrade from 1.x.x to 2.x.x.


## Introduction
The CCS811 has an internal micro controller with a boot loader and the gas application.
As the [datasheet](http://ams.com/ccs811) explains the boot loader can be used to replace the gas application.
Since most Chinese boards come with firmware 1.1.0, and ams has released firmware 2.0.0 (around april 2018), I have written 
this example: it flashes 2.0.0 into a CCS811.


## Firmware
The firmware was available from the [ams web](http://ams.com/ccs811#tab/tools).
I have downloaded the "CCS811 Application firmware version 2.0.0" zip file.
It contains two binaries (now, 2020 March 7)
 - [CCS811_FW_App_v2-0-0.bin](CCS811_FW_App_v2-0-0.bin) converted to [CCS811_FW_App_v2-0-0.h](CCS811_FW_App_v2-0-0.h)
 - [CCS811_FW_App_v2-0-1.bin](CCS811_FW_App_v2-0-1.bin) converted to [CCS811_FW_App_v2-0-1.h](CCS811_FW_App_v2-0-1.h)

As the readme in the zip explains, new fresh sensors should use firmware 2-0-0, and sensors that have run 
for a number of days should use firmware 2-0-1. As the revision history in the zip explains, the 2-0-1 has
all the features of the 2-0-0, except that the burn-in compensation is disabled (hence it should be used
on sensors that are already burned in).

Note that after a couple of days (i.e. at the end of the burn-in period), the burn-in compensation of 
2-0-0 also stops. From this moment onwards, 2-0-0 and 2-0-1 are identical.

I explained how I believe it works in more detail in an 
[issue](https://github.com/maarten-pennings/CCS811/issues/8#issuecomment-580410288).

One of the users of this flash tool ([daferdur](https://github.com/daferdur)), 
[found](https://github.com/ryankurte/efm32-base/blob/master/hardware/kit/common/bsp/thunderboard/ccs811_fw_app_1v0p0.bin)
the binary of 1.0.0, and published that in an 
[issue](https://github.com/maarten-pennings/CCS811/issues/49#issue-619413200).
I have included that [CCS811_FW_App_v1-0-0.bin](CCS811_FW_App_v1-0-0.bin) and the converted 
[CCS811_FW_App_v1-0-0.h](CCS811_FW_App_v1-0-0.h) header in this repo, in case you want to downgrade.
I have not tried this myself!

## Process
I have downloaded the 2.0.x firmware from the ams pages.
I have written a Python script [hex.py](hex.py), Python 3.x - not Python 2.x.
It is used to convert a binary firmware file to a C-array as follows
```
python hex.py CCS811_FW_App_v2-0-0.bin > CCS811_FW_App_v2-0-0.h
```

The resulting text file `CCS811_FW_App_v2-0-0.h` starts like this
```
// Hex dump of 'CCS811_FW_App_v2-0-0.bin' created at 2020-03-07 17:59:19.992906

#include <stdint.h>

const char * image_name="CCS811_FW_App_v2-0-0.bin";
const uint8_t image_data[] PROGMEM = {
  0x5c, 0x1a, 0xdd, 0xff, 0x15, 0x25, 0xdd, 0x66,   0xfc, 0x88, 0x80, 0x49, 0x02, 0xdc, 0x17, 0x10, 
  0x1f, 0x73, 0x60, 0xae, 0xf4, 0xa0, 0x5d, 0xda,   0xcd, 0xca, 0x94, 0xc6, 0x6b, 0x96, 0x4f, 0xea, 
  0xed, 0xbd, 0xde, 0xc2, 0xea, 0xc1, 0x44, 0x8b,   0xf3, 0x7a, 0x4d, 0x61, 0x22, 0x83, 0xe5, 0xac, 
  0x3c, 0xb1, 0x44, 0x5f, 0x91, 0xa2, 0xdc, 0xd2,   0x40, 0x63, 0x1c, 0x11, 0x60, 0x10, 0x33, 0xe7, 
```

Finally, I have written a sketch [ccs811flash.ino](ccs811flash.ino) that uses this array to 
flash the CSS811.

If you prefer 2-0-1 instead of 2-0-0, please change the line 12
```
// The firmware image as byte array in C
#include "CCS811_FW_App_v2-0-0.h"
```

to

```
// The firmware image as byte array in C
#include "CCS811_FW_App_v2-0-1.h"
```




## Example
I hooked CCS811 with 1.1.0 firmware to the ESP8266.
The script gave this output:
```
Starting CCS811 flasher
setup: library version: 9
setup: hardware    version: 12
setup: bootloader  version: 1000
setup: application version: 1100
setup: starting flash of 'CCS811_FW_App_v2-0-0.bin' in 5 seconds

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
Just run the flash app again.

```
Starting CCS811 flasher
setup: library version: 9
ccs811: Wrong HW_ID: FD
setup: CCS811 begin FAILED
setup: hardware version: FD
setup: bootloader version: FDFD
setup: application version: FDFD
setup: starting flash of CCS811_FW_App_v2-0-0.bin' in 5 seconds

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

|  # | Person            | When         | Lib      | Board                      | Host                | IDE         |
|:--:|:-----------------:|:------------:|:--------:|:--------------------------:|:-------------------:|:-----------:|
|  1 | @maarten-pennings | 2018 Dec  5  | v8       | ams eval kit               | ESP8266 (Robotdyn)  |  Arduino    |
|  2 | @maarten-pennings | 2018 Dec  7  | v8       | CJMCU-811                  | ESP8266 (Robotdyn)  |  Arduino    |
|  3 | @bfaliszek        | 2018 Dec 17  | v8       | CJMCU-811                  | ESP8266             |  Arduino    |
|  4 | @rovale           | 2018 Dec 28  | v8       | CJMCU-811                  | ESP32 (LOLIN D32)   |  Arduino    |
|  5 | @bertrik          | 2019 Jan 03  | v8       | CJMCU-811                  | ESP8266 (d1 mini)   |  platformio |
|  6 | @bfaliszek        | 2019 Jan 04  | v8       | CCS811/HDC1080             | ESP8266             |  Arduino    |
|  7 | Kees M            | 2019 Jan 13  | v8       | CJMCU-811                  | Arduino nano        |  Arduino    |
|  8 | @maarten-pennings | 2019 Jan 18  | v9       | ams eval kit               | Arduino nano        |  Arduino    |
|  9 | @maarten-pennings | 2019 Jan 18  | v9       | CJMCU-811                  | ESP8266 (Wemos)     |  Arduino    |
| 10 | @hmax42           | 2019 Mar 14  | v10      | CJMCU-811                  | ESP8266             |  unknown    |
| 11 | @bill-orange      | 2019 May 31  | v10      | Adafruit CCS811            | unknown             |  unknown    |
| 12 | @msbjx5           | 2019 Jun 26  | v10      | CJMCU-811                  | WEMOS D1            |  Arduino    |
| 13 | @regnerus         | 2019 Jul  4  | v10      | CJMCU-8118 (CCS811+HDC1080)| WEMOS D1            |  unknown    |
| 14 | @Phycheplabowa    | 2019 Nov 5   | v10      | CJMCU-811 + CJMCU-8128 (5x)| ESP8266             | platformio  |
| 15 | @SFeli            | 2019 Dec 29  | v10      | CJMCU-8128                 | ESP32               | Arduino     |
| 16 | @ciphercore       | 2020 Jan 10  | v10      | CJMCU-811                  | Arduino Uno         | Arduino     |
| 17 | @luytendries      | 2020 Feb 10  | v10      | CJMCU-811                  | ESP32               | Arduino     |
| 18 | @soczkers         | 2020 Apr  1  | v10      | CJMCU-8128                 | ESP32 (lolin lite)  | Arduino     |
| 19 | @dadosch          | 2020 Apr 10  | v10      | ?                          | ?                   | ?           |
| 20 | @Roelski          | 2020 Apr 11  | v10      | HDC1080                    | EPS32               | ?           |
| 21 | @fcar12           | 2021 Feb 11  | v10      | CJMCU-811                  | Arduino Uno         | Arduino     |
| 22 | @piecol           | 2021 Jun 11  | v10      | HELTEC CCS811 capsule addon| HTCC-AM02           | Arduino     |

(end of doc)
