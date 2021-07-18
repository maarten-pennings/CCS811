# Environmental data

Configuring the built-in algorithm for environmental conditions.

## Introduction

The CSS811 has a built-in algorithm that converts the sensor reading (the resistance of the MoX layer)
to a CO2 and TVOC equivalent. This mapping depends on the environmental conditions: 
the air temperature and the relative humidity.
Presumably, water vapor is registered as gas.

If the actual temperature and relative humidity figures are available, they can be written to the CCS811 chip.
If not, the sensor assumes defaults 25°C and 50%RH.

## Native

To configure the CCS811 for the actual values, we need to write to register "ENV_DATA (Environment Data) Register (0x05)".
The temperature is stored as an unsigned 16 bits integer in 1/512 degrees; there is an offset: 0 maps to -25°C. 
The humidity is stored as an unsigned 16 bits in 1/512%RH. 

The library has a function `set_envdata(uint16_t t, uint16_t h)` that allows the host to 
configure temperature and relative humidity using the values (units) native to the CCS811.

## ENS210

Since the ENS210 temperature and relative humidity sensor is produced by the same company as the CCS811,
there are boards that have both chips. Unfortunately, the data coming from the ENS210 does not have the exact format
as the data used by the CCS811.

The library has a function `set_envdata210(uint16_t t, uint16_t h)` that allows the host to 
configure temperature and relative humidity using the raw values coming from the ENS210.
This prevents going from int (ENS210) via float (Celsius) back to int (CCS811).

## Generic

In case the data is only available as float numbers (Celsius and percentage relative humidity), 
there is a function `set_envdata_Celsius_percRH(float t, float h)`. This uses floating point computations.

Until version 11 of the library, this floating point version was missing, because I try to avoid 
using `float` in embedded software. But when I tried it was not that bad.


| Board    |  without implementation  | with implementation, without call | with implementation and call |
|:--------:|:------------------------:|:---------------------------------:|:----------------------------:|
| ESP8266  | Sketch uses 270364 bytes | Sketch uses 270508 bytes          | Sketch uses 270380 bytes     |
| Pro mini | Sketch uses 8250 bytes   | Sketch uses 8250 bytes            | Sketch uses 8262 bytes       |

(I did not yet try if using `printf` with floats increases sketch size considerably).

(end of doc)




