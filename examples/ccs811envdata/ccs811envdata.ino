/*
  ccs811envdata.ino - Reading temperature and humidity data from the ENS210, passing that to the CCS811, to get better CO2 readings from CCS811
  Created by Maarten Pennings 2018 oct 23
*/


#include <Wire.h>    // I2C library
#include "ens210.h"  // ENS210 library
#include "ccs811.h"  // CCS811 library


ENS210 ens210;
CCS811 ccs811(D3); // nWAKE on D3


void setup() {
  // Enable serial
  Serial.begin(115200);
  Serial.println("");
  Serial.println("setup: Starting CCS811 envdata demo");
  Serial.print("setup: library     version: "); Serial.println(CCS811_VERSION);

  // Enable I2C, e.g. for ESP8266 NodeMCU boards: VDD to 3V3, GND to GND, SDA to D2, SCL to D1, nWAKE to D3 (or GND)
  Serial.print("setup: I2C ");
  Wire.begin(); 
  Serial.println("ok");
  
  // Enable ENS210
  Serial.print("setup: ENS210 ");
  bool ok;
  ok= ens210.begin();
  if( ok ) Serial.println("ok"); else Serial.println("FAILED");

  // Enable CCS811
  Serial.print("setup: CCS811 ");
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  ok= ccs811.begin();
  if( ok ) Serial.println("ok"); else Serial.println("FAILED");

  // Start CCS811 (measure every 1 second)
  Serial.print("setup: CCS811 start ");
  ok= ccs811.start(CCS811_MODE_1SEC);
  if( ok ) Serial.println("ok"); else Serial.println("FAILED");
}


void loop() {
  // Read ENS210
  int t_data, t_status, h_data, h_status;
  ens210.measure(&t_data, &t_status, &h_data, &h_status );

  // Process ENS210
  Serial.print( "ENS210: " );
  if( t_status==ENS210_STATUS_OK && h_status==ENS210_STATUS_OK ) {
    Serial.print("T="); Serial.print( ens210.toCelsius(t_data,1000) );     Serial.print(" mC  ");
    Serial.print("H="); Serial.print( ens210.toPercentageH(h_data,1000) ); Serial.print(" m%RH  ");
    // Pass environmental data from ENS210 to CCS811
    ccs811.set_envdata210(t_data, h_data);
  } else {
    Serial.print( "T=" ); Serial.print( ens210.status_str(t_status) ); Serial.print("  ");
    Serial.print( "H=" ); Serial.print( ens210.status_str(h_status) ); Serial.print("  ");
  }
  Serial.print("  ");

  // Read CCS811
  uint16_t eco2, etvoc, errstat, raw;
  ccs811.read(&eco2,&etvoc,&errstat,&raw); 

  // Process CCS811
  Serial.print("CCS811: ");
  if( errstat==CCS811_ERRSTAT_OK ) {
    Serial.print("eco2=");  Serial.print(eco2);  Serial.print(" ppm  ");
    Serial.print("etvoc="); Serial.print(etvoc); Serial.print(" ppb  ");  
  } else if( errstat==CCS811_ERRSTAT_OK_NODATA ) {
    Serial.print("waiting for (new) data");
  } else if( errstat & CCS811_ERRSTAT_I2CFAIL ) { 
    Serial.print("I2C error");
  } else {
    Serial.print( "error: " );
    Serial.print( ccs811.errstat_str(errstat) ); 
  }
  Serial.println();

  // Wait
  delay(1000); 
}

