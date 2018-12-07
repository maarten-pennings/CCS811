/*
  ccs811flash.ino - Sketch to flash a CCS811 with 2.0.0 firmware.
  Created by Maarten Pennings 2017 Dec 11
*/


#include <Wire.h>    // I2C library
#include "ccs811.h"  // CCS811 library


// The firmware image as byte array in C
#include "CCS811_SW000246_1-00.h"


// Wiring for ESP8266 NodeMCU boards: VDD to 3V3, GND to GND, SDA to D2, SCL to D1, nWAKE to D3 (or GND)
CCS811 ccs811(D3); // nWAKE on D3


void setup() {
  // Enable serial
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Starting CCS811 flasher");

  // Enable I2C
  Wire.begin(); 
  
  // Enable CCS811
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  bool ok= ccs811.begin();
  if( !ok ) Serial.println("init: CCS811 begin FAILED"); 

  // Print CCS811 versions
  Serial.print("init: hardware    version: "); Serial.println(ccs811.hardware_version(),HEX);
  Serial.print("init: bootloader  version: "); Serial.println(ccs811.bootloader_version(),HEX);
  Serial.print("init: application version: "); Serial.println(ccs811.application_version(),HEX);

  // Flash
  if( ccs811.application_version()==0x2000 ) { Serial.println("init: already has 2.0.0"); return; } // Do not reflash
  Serial.print("init: starting flash of '");
  Serial.print(image_name);
  Serial.println("' in 5 seconds");
  delay(5000);
  Serial.println("");
  ok= ccs811.flash(image_data, sizeof(image_data));
  if( !ok ) Serial.println("init: CCS811 flash FAILED");
  Serial.println("");
}


void loop() {
  Serial.println("loop: ended ...");
  delay(10000); 
}
