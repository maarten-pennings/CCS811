/*
  ccs811flash.ino - Sketch to flash a CCS811 with 2.0.0 firmware.
  Created by Maarten Pennings 2018 Dec 10, updated 2020-03-07 change on ams website
*/


#include <Wire.h>    // I2C library
#include "ccs811.h"  // CCS811 library


// The firmware image as byte array in C
#include "CCS811_FW_App_v2-0-0.h"


// Wiring for ESP8266 NodeMCU boards: VDD to 3V3, GND to GND, SDA to D2, SCL to D1, nWAKE to D3 (or GND)
CCS811 ccs811(D3); // nWAKE on D3

// Wiring for Nano: VDD to 3v3, GND to GND, SDA to A4, SCL to A5, nWAKE to 13
//CCS811 ccs811(13); 

// nWAKE not controlled via Arduino host, so connect CCS811.nWAKE to GND
//CCS811 ccs811; 


void setup() {
  // Enable serial
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Starting CCS811 flasher");
  Serial.print("setup: library     version: "); Serial.println(CCS811_VERSION);

  // Enable I2C
  Wire.begin(); 
  
  // Enable CCS811
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  bool ok= ccs811.begin();
  if( !ok ) Serial.println("setup: CCS811 begin FAILED"); 

  // Print CCS811 versions
  Serial.print("setup: hardware    version: "); Serial.println(ccs811.hardware_version(),HEX);
  Serial.print("setup: bootloader  version: "); Serial.println(ccs811.bootloader_version(),HEX);
  Serial.print("setup: application version: "); Serial.println(ccs811.application_version(),HEX);

  // Check if flashing should be executed
  if( ccs811.application_version()==0x2000 ) { Serial.println("init: already has 2.0.0"); return; } // Do not reflash
  Serial.println("setup: comment-out this code line if you want to flash"); return; // Extra precaution

  // Flash
  Serial.print("setup: starting flash of '");
  Serial.print(image_name);
  Serial.println("' in 5 seconds");
  delay(5000);
  Serial.println("");
  ok= ccs811.flash(image_data, sizeof(image_data));
  if( !ok ) Serial.println("setup: CCS811 flash FAILED");
  Serial.println("");
}


void loop() {
  Serial.println("loop: ended ...");
  delay(10000); 
}
