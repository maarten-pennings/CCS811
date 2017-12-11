/* 
  ccs811.cpp - Library for the CCS811 digital gas sensor for monitoring indoor air quality from ams.
  Created by Maarten Pennings 2017 Dec 11
*/


#include <arduino.h>
#include <Wire.h>
#include "ccs811.h"


// The CCS811 needs a wait between write and read
#define CCS811_REPEATEDSTART_WAIT() delayMicroseconds(100)


// Main interface =====================================================================================================


// CCS811 registers/mailboxes, all 1 byte except when stated otherwise
#define CCS811_STATUS           0x00 
#define CCS811_MEAS_MODE        0x01 
#define CCS811_ALG_RESULT_DATA  0x02 // up to 8 bytes
#define CCS811_RAW_DATA         0x03 // 2 bytes
#define CCS811_ENV_DATA         0x05 // 4 bytes
#define CCS811_THRESHOLDS       0x10 // 5 bytes
#define CCS811_BASELINE         0x11 // 2 bytes
#define CCS811_HW_ID            0x20 
#define CCS811_HW_VERSION       0x21
#define CCS811_FW_BOOT_VERSION  0x23 // 2 bytes
#define CCS811_FW_APP_VERSION   0x24 // 2 bytes
#define CCS811_ERROR_ID         0xE0 
#define CCS811_APP_ERASE        0xF1 // 4 bytes
#define CCS811_APP_DATA         0xF2 // 9 bytes
#define CCS811_APP_VERIFY       0xF3 // 0 bytes
#define CCS811_APP_START        0xF4 // 0 bytes
#define CCS811_SW_RESET         0xFF // 4 bytes


// Pin number connected to nWAKE (nWAKE can also be bound to GND, then pass -1), slave address (5A or 5B)
CCS811::CCS811(int nwake, int slaveaddr) {
  _nwake= nwake;
  _slaveaddr= slaveaddr;
  wake_init();
}


// Reset the CCS811, switch to app mode and check HW_ID. Returns false on problems.
bool CCS811::begin( void ) {
  uint8_t hw_id;
  uint8_t status;
  bool ok;
  wake_up();
  ok= i2cwrite4(CCS811_SW_RESET, 0x11E5728A); if( !ok ) { Serial.println("ccs811: begin: reset failed"); return false; }
  delayMicroseconds(1000);
  ok= i2cread1(CCS811_STATUS, &status);       if( !ok ) { Serial.println("ccs811: begin: STATUS read (boot mode) failed"); return false; }
  if( status!=0x10 )                                    { Serial.println("ccs811: begin: Not in boot mode, or no valid app"); return false; }   
  ok= i2cwrite0(CCS811_APP_START);            if( !ok ) { Serial.println("ccs811: begin: Goto app mode failed"); return false; } 
  delayMicroseconds(100);
  ok= i2cread1(CCS811_STATUS, &status);       if( !ok ) { Serial.println("ccs811: begin: STATUS read (app mode) failed"); return false; }
  if( status!=0x90 )                                    { Serial.println("ccs811: begin: Not in app mode, or no valid app"); return false; }  
  ok= i2cread1(CCS811_HW_ID, &hw_id);         if( !ok ) { Serial.println("ccs811: begin: HW_ID read failed"); return false; } 
  if( hw_id!=0x81 )                                     { Serial.println("ccs811: begin: Wrong HW_ID"); return false; }
  wake_down();
  return true;
}


// Switched CCS811 to `mode`, use constants CCS811_MODE_XXX. Returns false on problems.
bool CCS811::start( int mode ) {
  wake_up();
  bool ok = i2cwrite1(CCS811_MEAS_MODE, mode<<4);
  wake_down();
  return ok;
}


// Get measurement results from the CCS811, check status via errstat, e.g. ccs811_errstat(errstat)
void CCS811::read( uint16_t*eco2, uint16_t*etvoc, uint16_t*errstat,uint16_t*raw) {
  uint32_t msb,lsb;
  wake_up();
  bool ok = i2cread8(CCS811_ALG_RESULT_DATA,&msb,&lsb);
  wake_down();
  // Status and error management
  uint8_t error_id= ((lsb>>16) & 0xFF);
  uint8_t status  = ((lsb>>24) & 0xFF);
  if( !ok ) status |= CCS811_ERRSTAT_I2CFAIL;
  // Outputs
  if( eco2   ) *eco2   = (msb>>16) & 0xFFFF;
  if( etvoc  ) *etvoc  = (msb>>0) & 0xFFFF;
  if( errstat) *errstat= (error_id<<8) | (status<<0);
  if( raw    ) *raw    = (lsb>> 0) & 0xFFFF;
}


// Returns true if errstat flags denote NEW&READY, false for OLD|ERROR
bool CCS811::errstat_ok(int errstat ) {
  uint16_t zeros = CCS811_ERRSTAT_ERROR | CCS811_ERRSTAT_I2CFAIL 
                 | CCS811_ERRSTAT_WRITE_REG_INVALID | CCS811_ERRSTAT_READ_REG_INVALID | CCS811_ERRSTAT_MEASMODE_INVALID 
                 | CCS811_ERRSTAT_MAX_RESISTANCE | CCS811_ERRSTAT_HEATER_FAULT | CCS811_ERRSTAT_HEATER_SUPPLY ;
  uint16_t ones  = CCS811_ERRSTAT_DATA_READY | CCS811_ERRSTAT_APP_VALID | CCS811_ERRSTAT_FW_MODE;
  return ( (errstat&ones)==ones ) &&  ( (errstat&zeros)==0 );
}

                                         
// Returns a string version of an errstat. Note, each call, this string is updated.
const char * CCS811::errstat_str(uint16_t errstat) {
  static char s[17]; // 16 bits plus terminating zero
                                                  s[16]='\0';
  if( errstat & CCS811_ERRSTAT_ERROR            ) s[15]='E'; else s[15]='e';
  if( errstat & CCS811_ERRSTAT_I2CFAIL          ) s[14]='I'; else s[14]='i';
                                                  s[13]='-';
  if( errstat & CCS811_ERRSTAT_DATA_READY       ) s[12]='D'; else s[12]='d';
  if( errstat & CCS811_ERRSTAT_APP_VALID        ) s[11]='A'; else s[11]='a';
                                                  s[10]='-';
                                                  s[ 9]='-';
  if( errstat & CCS811_ERRSTAT_FW_MODE          ) s[ 8]='F'; else s[8]='f';
  if( errstat & CCS811_ERRSTAT_WRITE_REG_INVALID) s[ 7]='W'; else s[7]='w';
  if( errstat & CCS811_ERRSTAT_READ_REG_INVALID ) s[ 6]='R'; else s[6]='r';
  if( errstat & CCS811_ERRSTAT_MEASMODE_INVALID ) s[ 5]='M'; else s[5]='m';
  if( errstat & CCS811_ERRSTAT_MAX_RESISTANCE   ) s[ 4]='X'; else s[4]='x';
  if( errstat & CCS811_ERRSTAT_HEATER_FAULT     ) s[ 3]='H'; else s[3]='h';
  if( errstat & CCS811_ERRSTAT_HEATER_SUPPLY    ) s[ 2]='V'; else s[2]='v';
                                                  s[ 1]='-';
                                                  s[ 0]='-';
  return s;
}


// Helper interface: nwake pin ========================================================================================


void CCS811::wake_init( void ) {
  if( _nwake>=0 ) pinMode(_nwake, OUTPUT);  
}

void CCS811::wake_up( void) {
  if( _nwake>=0 ) { digitalWrite(_nwake, LOW); delayMicroseconds(50);  }
}

void CCS811::wake_down( void) {
  if( _nwake>=0 ) digitalWrite(_nwake, HIGH);
}


// Helper interface: i2c wrapper ======================================================================================


// Writes nothing to register at address `regaddr` in the CCS811. Returns false on I2C problems.
bool CCS811::i2cwrite0(int regaddr) {
  Wire.beginTransmission(_slaveaddr);       // START, SLAVEADDR
  Wire.write(regaddr);                     // Register address
  int r= Wire.endTransmission(true);       // STOP
  //Serial.printf("ccs811: write0: %02x (%d)\n",regaddr,r);
  return r==0;
}

// Writes 8bit `regval` to register at address `regaddr` in the CCS811. Returns false on I2C problems.
bool CCS811::i2cwrite1(int regaddr, uint8_t regval) {
  Wire.beginTransmission(_slaveaddr);       // START, SLAVEADDR
  Wire.write(regaddr);                     // Register address
  Wire.write(regval);                      // New register value
  int r= Wire.endTransmission(true);       // STOP
  //Serial.printf("ccs811: write1: %02x <- %02x (%d)\n",regaddr,regval,r);
  return r==0;
}

// Writes 16bit `regval` to register at address `regaddr` in the CCS811. Returns false on I2C problems.
bool CCS811::i2cwrite2(int regaddr, uint16_t regval) {
  Wire.beginTransmission(_slaveaddr);       // START, SLAVEADDR
  Wire.write(regaddr);                     // Register address
  Wire.write((regval>>8)&0xFF);            // New register value MSB
  Wire.write((regval>>0)&0xFF);            // New register value LSB
  int r= Wire.endTransmission(true);       // STOP
  //Serial.printf("ccs811: write2: %02x <- %04x (%d)\n",regaddr,regval,r);
  return r==0;
}

// Writes 32bit `regval` to register at address `regaddr` in the CCS811. Returns false on I2C problems.
bool CCS811::i2cwrite4(int regaddr, uint32_t regval) {
  Wire.beginTransmission(_slaveaddr);       // START, SLAVEADDR
  Wire.write(regaddr);                     // Register address
  Wire.write((regval>>24)&0xFF);           // New register value MSB
  Wire.write((regval>>16)&0xFF);           // New register value 
  Wire.write((regval>>8)&0xFF);            // New register value 
  Wire.write((regval>>0)&0xFF);            // New register value LSB
  int r= Wire.endTransmission(true);       // STOP
  //Serial.printf("ccs811: write4: %02x <- %08x (%d)\n",regaddr,regval,r);
  return r==0;
}

// Reads 1 byte from register at address `regaddr`, and store it in `regval`. Returns false on I2C problems.
bool CCS811::i2cread1(int regaddr, uint8_t * regval) {
  Wire.beginTransmission(_slaveaddr);                            // START, SLAVEADDR
  Wire.write(regaddr);                                          // Register address
  int wres= Wire.endTransmission(false);                        // Repeated START
  CCS811_REPEATEDSTART_WAIT();                                  // Wait 
  int rres=Wire.requestFrom((uint8_t)_slaveaddr,(size_t)1,true); // From CCS811, read bytes, STOP
  uint8_t byte0= Wire.read();
  *regval= (byte0<<0); 
  //Serial.printf("ccs811: read1: %02x -> %02x (%d,%d)\n",regaddr,*regval,wres,rres);
  return (wres==0) && (rres==1);
}

// Reads 2 bytes from register at address `regaddr`, and stores them in `regval`. Returns false on I2C problems.
bool CCS811::i2cread2(int regaddr, uint16_t * regval) {
  Wire.beginTransmission(_slaveaddr);                            // START, SLAVEADDR
  Wire.write(regaddr);                                          // Register address
  int wres= Wire.endTransmission(false);                        // Repeated START
  CCS811_REPEATEDSTART_WAIT();                                  // Wait 
  int rres=Wire.requestFrom((uint8_t)_slaveaddr,(size_t)2,true); // From CCS811, read bytes, STOP
  uint8_t byte0= Wire.read();
  uint8_t byte1= Wire.read();
  *regval= (byte0<<8) | (byte1<<0); 
  //Serial.printf("ccs811: read2: %02x -> %04x (%d,%d)\n",regaddr,*regval,wres,rres);
  return (wres==0) && (rres==2);
}

// Reads 4 bytes from register at address `regaddr`, and stores them in `regval`. Returns false on I2C problems.
bool CCS811::i2cread4(int regaddr, uint32_t * regval) {
  Wire.beginTransmission(_slaveaddr);                            // START, SLAVEADDR
  Wire.write(regaddr);                                          // Register address
  int wres= Wire.endTransmission(false);                        // Repeated START
  CCS811_REPEATEDSTART_WAIT();                                  // Wait 
  int rres=Wire.requestFrom((uint8_t)_slaveaddr,(size_t)4,true); // From CCS811, read bytes, STOP
  uint8_t byte0= Wire.read();
  uint8_t byte1= Wire.read();
  uint8_t byte2= Wire.read();
  uint8_t byte3= Wire.read();
  *regval= ((uint32_t)byte0<<24) | ((uint32_t)byte1<<16) | (byte2<<8) | (byte3<<0); 
  //Serial.printf("ccs811: read4: %02x -> %08x (%d,%d)\n",regaddr,*regval,wres,rres);
  return (wres==0) && (rres==4);
}

// Reads 8 bytes from register at address `regaddr`, and stores them in `regval`. Returns false on I2C problems.
bool CCS811::i2cread8(int regaddr, uint32_t * msb, uint32 * lsb) {
  Wire.beginTransmission(_slaveaddr);                            // START, SLAVEADDR
  Wire.write(regaddr);                                          // Register address
  int wres= Wire.endTransmission(false);                        // Repeated START
  CCS811_REPEATEDSTART_WAIT();                                  // Wait 
  int rres=Wire.requestFrom((uint8_t)_slaveaddr,(size_t)8,true); // From CCS811, read bytes, STOP
  uint32_t byte0= Wire.read();
  uint32_t byte1= Wire.read();
  uint32_t byte2= Wire.read();
  uint32_t byte3= Wire.read();
  *msb= (byte0<<24) | (byte1<<16) | (byte2<<8) | (byte3<<0); 
  byte0= Wire.read();
  byte1= Wire.read();
  byte2= Wire.read();
  byte3= Wire.read();
  *lsb= ((uint32_t)byte0<<24) | ((uint32_t)byte1<<16) | (byte2<<8) | (byte3<<0); 
  // Serial.printf("ccs811: read8: %02x -> %08x %08x (%d,%d)\n",regaddr,*msb,*lsb,wres,rres);
  return (wres==0) && (rres==8);
}


