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

char * image_name="CCS811_SW000246_1-00.bin";
uint8_t image_data[]= {
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
init: hardware    version: 12
init: bootloader  version: 1000
init: application version: 1100
init: starting flash of 'CCS811_SW000246_1-00.bin' in 5 seconds

ccs811: flash: successful ping
ccs811: flash: successful reset
ccs811: flash: status ok (boot mode): 10
ccs811: flash: successful erase
ccs811: flash: status ok (erased): 40
ccs811: flash: writing ...................................................................
ccs811: flash: writing ...................................................................
ccs811: flash: writing ...................................................................
ccs811: flash: writing ...................................................................
ccs811: flash: writing ...................................................................
ccs811: flash: writing ...................................................................
ccs811: flash: writing ...................................................................
ccs811: flash: writing ...................................................................
ccs811: flash: writing ...................................................................
ccs811: flash: writing ...................................................................
ccs811: flash: writing ... done
ccs811: flash: successful verify
ccs811: flash: status ok (verified): 30

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

(end of doc)
