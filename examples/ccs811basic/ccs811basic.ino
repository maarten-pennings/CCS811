/*
  ccs811basic.ino - Demo sketch printing results of the CCS811 digital gas sensor for monitoring indoor air quality from ams.
  Created by Maarten Pennings 2017 Dec 11
*/


#include <Wire.h>    // I2C library
#include "ccs811.h"  // CCS811 library


CCS811 ccs811(D3); // nWAKE on D3


void setup() {
  // Enable serial
  delay(1000);
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Starting CCS811 basic demo");

  // Enable I2C for ESP8266 NodeMCU boards [VDD to 3V3, GND to GND, SDA to D2, SCL to D1, nWAKE to D3 (or GND)]
  Wire.begin(); 
  
  // Enable CCS811
  bool ok= ccs811.begin();
  if( !ok ) Serial.println("init: CCS811 begin FAILED");

  // Print CCS811 versions
  Serial.print("init: hardware    version: "); Serial.println(ccs811.hardware_version(),HEX);
  Serial.print("init: bootloader  version: "); Serial.println(ccs811.bootloader_version(),HEX);
  Serial.print("init: application version: "); Serial.println(ccs811.application_version(),HEX);
  
  // Start measuring
  ok= ccs811.start(CCS811_MODE_1SEC);
  if( !ok) Serial.println("init: CCS811 start FAILED");
}


void loop() {
  // Wait
  delay(3000); 

  // Read
  uint16_t eco2;
  uint16_t etvoc;
  uint16_t errstat;
  uint16_t raw;
  ccs811.read(&eco2,&etvoc,&errstat,&raw); 
  
  // Note, I2C errors are also in errstat (CCS811_ERRSTAT_I2CFAIL)
  if( errstat & CCS811_ERRSTAT_I2CFAIL ) { Serial.println("CCS811: I2C error"); return; } 
  
  // Check if errstat flags indicates there is valid and new data, or whether data is old or has errors
  bool valid_and_new = ( (errstat&CCS811_ERRSTAT_NEEDS) == CCS811_ERRSTAT_NEEDS )  &&  ( (errstat&CCS811_ERRSTAT_ERRORS)==0 );
  
  // Print
  Serial.print("CCS811: ");
  Serial.print("eco2=");    Serial.print(eco2);        Serial.print(" ppm,  ");
  Serial.print("etvoc=");   Serial.print(etvoc);       Serial.print(" ppb,  ");
  Serial.print("errstat="); Serial.print(errstat,HEX); Serial.print("="); Serial.print(ccs811.errstat_str(errstat)); Serial.print( valid_and_new?"=valid&new,  ":"=ERROR|OLD,  ");
  Serial.print("raw6=");    Serial.print(raw/1024);    Serial.print(" uA,  "); 
  Serial.print("raw10=");   Serial.print(raw%1024);    Serial.print(" ADC");
  Serial.println();
}

