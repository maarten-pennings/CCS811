/*
  ccs811demo.ino - Demo sketch printing results of the CCS811 digital gas sensor for monitoring indoor air quality from ams.
  Created by Maarten Pennings 2017 Dec 11
*/


#include <Wire.h>    // I2C library
#include "ccs811.h"  // CCS811 library

CCS811 ccs811(D3);

void print_versions() {
  uint8_t hw_version;
  uint16_t fw_boot_version;
  uint16_t fw_app_version;
  bool ok= ccs811.versions(&hw_version, &fw_boot_version, &fw_app_version);
  if( ok ) {
    Serial.print("versions: "); 
    Serial.print("hw "); Serial.print(hw_version,HEX);
    Serial.print(", fw_boot "); Serial.print(fw_boot_version,HEX); 
    Serial.print(", fw_app "); Serial.print(fw_app_version,HEX);
    Serial.println("");
  } else {
    Serial.println("versions: failed");
  }
}

void setup() {
  // Enable serial
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Starting CCS811 simple demo");

  // Enable I2C for ESP8266 NodeMCU boards [VDD to 3V3, GND to GND, nWAKE to GND, SDA to D2, SCL to D1]
  Wire.begin(/*SDA*/D2,/*SCL*/D1); 
  Serial.println("init: I2C up");
  
  // Enable CCS811
  bool ok= ccs811.begin();
  Serial.println(ok?"init: CCS811 up":"init: CCS811 begin ERROR");
  ok= ccs811.start(CCS811_MODE_1SEC);
  Serial.println(ok?"init: CCS811 started":"init: start ERROR");

  // Check version
  print_versions();
}

void loop() {
  // Read
  uint16_t eco2;
  uint16_t etvoc;
  uint16_t errstat;
  uint16_t raw;
  ccs811.read(&eco2,&etvoc,&errstat,&raw); // Note, I2C errors are also in errstat
  
  // Print
  Serial.print("CCS811: ");
  Serial.print("eco2=");    Serial.print(eco2);        Serial.print(" ppm,  ");
  Serial.print("tvoc=");    Serial.print(etvoc);       Serial.print(" ppb,  ");
  Serial.print("errstat="); Serial.print(errstat,HEX); Serial.print(ccs811.errstat_str(errstat)); Serial.print( ccs811.errstat_ok(errstat) ? "=VALID&NEW,  " : "=ERROR|OLD,  " );
  Serial.print("raw=");     Serial.print(raw);         Serial.print(" 6uA/10ADC");
  Serial.println();

  // Wait
  delay(5000); 
}

