# CCS811
Arduino library for the CCS811 digital gas sensor for monitoring indoor air quality from ams.


## Introduction
This project is an Arduino *library*. It implements a driver for the CCS811.
This chip is a indoor air quality sensor module with an I2C interface.

The code has been tested with
 - [NodeMCU (ESP8266)](https://www.aliexpress.com/item/NodeMCU-V3-Lua-WIFI-module-integration-of-ESP8266-extra-memory-32M-flash-USB-serial-CP2102/32779738528.html)
 - [Arduino pro mini](https://www.aliexpress.com/item/ProMini-ATmega328P-3-3V-Compatible-for-Arduino-Pro-Mini/32525927539.html)
 - [Arduino nano](https://www.aliexpress.com/item/Nano-CH340-ATmega328P-MicroUSB-Compatible-for-Arduino-Nano-V3/32572612009.html)

Note that the CCS811 requires a supply voltage of 1.8V .. 3.6V.
So, 3.3V is ok, but *do not use a 5V board*.
The Nano has 3v3 supply, but runs I2C on 5V. This does seem to work.
Also note that the minimum supply voltage is 1.8V and should not drop below this value for reliable device operation.


## Links
The CCS811 is made by [ams](https://www.ams.com). This library is compatible with the following variants.
 - Find the datasheet of the CCS811 on the
   [product page](https://ams.com/ccs811).
 - Find application notes and software on the
   same page (see Tools&Support or Technical Documents).


## Prerequisites
It is assumed that
 - The Arduino IDE has been installed.
   If not, refer to "Install the Arduino Desktop IDE" on the
   [Arduino site](https://www.arduino.cc/en/Guide/HomePage).
 - The library directory is at its default location.
   For me, Maarten, that is `C:\Users\maarten\Documents\Arduino\libraries`.


## Installation
Installation steps
 - Visit the [project page](https://github.com/maarten-pennings/CCS811) for the Arduino CCS811 library.
 - Click the green button `Clone or download` on the right side.
 - From the pop-up choose `Download ZIP`.
 - In Arduino IDE, select Sketch > Include Library > Manage Libraries ...
   and browse to the just downloaded ZIP file.
 - When the IDE is ready this `README.md` should be located at e.g.
   `C:\Users\maarten\Documents\Arduino\libraries\CCS811\README.md`.


## Build an example
To build an example sketch
 - (Re)start Arduino.
 - Open File > Example > Examples from Custom Libraries > CCS811 > CCS811basic.
 - Make sure Tools > Board lists the correct board.
 - Select Sketch > Verify/Compile.


## Wiring
The CCS811 has several PINS:
 - VDD must be connected to 3V3.
 - GND must be connected to GND.
 - SDA must be connected to SDA of micro (and maybe a pull-up, but the below micros have that internally).
 - SCL must be connected to SCL of micro (and maybe a pull-up, but the below micros have that internally).
 - nWAKE can either be tied to GND, in which case the CCS811 is always awake (using more power).
   Alternatively nWAKE can be tied to a GPIO pin of the micro, the CCS library will then use this pin to wake-up the micro when needed.
   In the former case pass -1 to the constructor `CCS811 ccs811(-1)`, in the latter case, pass the pin number, e.g. `CCS811 ccs811(D3)`.
 - nINT can be left dangling, interrupts are not used by the library.
 - nRESET can be left dangling, it is not used by the library (the library employs a software reset).
 - When ADDR is connected the GND, the CCS811 has slave address 0x5A (constant `CCS811_SLAVEADDR_0`).
   When ADDR is connected the VDD, the CCS811 has slave address 0x5B (constant `CCS811_SLAVEADDR_1`).
   If your board has an ADDR pin, then likely it has a pull-up, so leaving it dangling selects 0x5B.
   If your board has no ADDR pin, then likely it has the pin tied to GND, selecting 0x5A.
   The `ccs811.begin()` uses the address passed in the constructor, but if the other address does work, 
   it prints this messages on Serial.


## Tested boards
This library has been tested with three boards.

Most micro controllers seem to have built-in pull-ups for I2C.
However, those pull-ups are typically activated by `Wire.begin()`.
Therefore, between power-up (or reset) and `Wire.begin()`, the I2C lines might not be high.
This might cause unwanted behavior on the slaves.

It is recommended to add 10k pull-ups on both SDA and SCL.


### ESP8266
For the NodeMCU (ESP8266), connect as follows (I did not use pull-ups, presumably they are inside the MCU)

| CCS811  |  ESP8266  |
|:-------:|:---------:|
|   VDD   |    3V3    |
|   GND   |    GND    |
|   SDA   |    D2     |
|   SCL   |    D1     |
| nWAKE   | D3 or GND |

![wiring ESP8266 NodeMCU](wire-esp.jpg)

Unfortunately, the CCS811 uses clock stretching, and the I2C sw library in the ESP8266
can not handle this in all cases, therefore this library add waits (see `CCS811_WAIT_REPSTART_US` in `ccs811.cpp`).


### Pro Mini
For the Pro mini (do *not* use a 5V board), connect as follows  (I did not use pull-ups, presumably they are inside the MCU)

| CCS811  |  Pro mini |
|:-------:|:---------:|
|   VDD   |    VCC    |
|   GND   |    GND    |
|   SDA   |     A4    |
|   SCL   |     A5    |
| nWAKE   | D3 or GND |

![wiring pro mini](wire-promini.jpg)


### Arduino Nano
For the Arduino Nano, connect as follows  (I did not use pull-ups, presumably they are inside the MCU)

| CCS811  |    Nano   |
|:-------:|:---------:|
|   VDD   |    3V3    |
|   GND   |    GND    |
|   SDA   |     A4    |
|   SCL   |     A5    |
| nWAKE   | D3 or GND |

![wiring nano](wire-nanov3.jpg)


### CCS811
Connect the CCS811 module as follows

![wiring CCS811](wire-ccs811.jpg)


## Flash an example
To build an example sketch
 - (Re)start Arduino.
 - Open File > Example > Examples from Custom Libraries > CCS811 > CCS811basic.
 - Make sure Tools > Board lists the correct board.
 - Select Sketch > Upload.
 - Select Tools > Serial Monitor.
 - Enjoy the output, which should be like this for `CCS811basic`:

     ```Text
     Starting CCS811 basic demo
     init: hardware    version: 12
     init: bootloader  version: 1000
     init: application version: 2000
     CCS811: waiting for (new) data
     CCS811: waiting for (new) data
     CCS811: waiting for (new) data
     CCS811: waiting for (new) data
     CCS811: eco2=400 ppm  etvoc=0 ppb  
     CCS811: eco2=405 ppm  etvoc=0 ppb  
     CCS811: eco2=405 ppm  etvoc=0 ppb  
     CCS811: eco2=407 ppm  etvoc=1 ppb  
     CCS811: eco2=405 ppm  etvoc=0 ppb  
     CCS811: eco2=400 ppm  etvoc=0 ppb  
     CCS811: eco2=400 ppm  etvoc=0 ppb  
     CCS811: eco2=405 ppm  etvoc=0 ppb  
     CCS811: eco2=405 ppm  etvoc=0 ppb  
     CCS811: eco2=405 ppm  etvoc=0 ppb
     ```
 - It is normal that early measurements have `errstat=90=--vhxmrwF--Ad-ie`, i.e. STATUS.DATA_READY clear
   (the lowercase `d`); the internal gas library needs some data points to startup.

 - At the time of writing this application, `application version: 2000` is available on the ams.com website.
   You might still have version 1100. To Flash it, you need the [CCS811 eval kit](https://ams.com/ccs811evalkit).
 
(end of doc)
