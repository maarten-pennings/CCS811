/* 
  ccs811.cpp - Library for the CCS811 digital gas sensor for monitoring indoor air quality from ams.
  2018 Oct 21  v2  Maarten Pennings  Simplified I2C, added version mngt
  2017 Dec 11  v1  Maarten Pennings  Created
*/


#include <Arduino.h>
#include <Wire.h>
#include "ccs811.h"


// Timings
#define CCS811_WAIT_AFTER_RESET_US     2000 // The CCS811 needs a wait after reset
#define CCS811_WAIT_AFTER_APPSTART_US  1000 // The CCS811 needs a wait after app start
#define CCS811_WAIT_AFTER_WAKE_US        50 // The CCS811 needs a wait after WAKE signal
#define CCS811_WAIT_REPSTART_US         100 // The CCS811 needs a wait between write and read I2C segment


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
  uint8_t sw_reset[]= {0x11,0xE5,0x72,0x8A};
  uint8_t app_start[]= {};
  uint8_t hw_id;
  uint8_t hw_version;
  uint8_t status;
  bool ok;
  wake_up();
  ok= i2cwrite(CCS811_SW_RESET, 4,sw_reset);   if( !ok ) { Serial.println("ccs811: begin: reset failed"); return false; }
  delayMicroseconds(CCS811_WAIT_AFTER_RESET_US);
  ok= i2cread(CCS811_STATUS,1,&status);        if( !ok ) { Serial.println("ccs811: begin: STATUS read (boot mode) failed"); return false; }
  if( status!=0x10 )                                     { Serial.print  ("ccs811: begin: Not in boot mode, or no valid app: "); Serial.println(status,HEX); return false; }   
  ok= i2cwrite(CCS811_APP_START,0,app_start);  if( !ok ) { Serial.println("ccs811: begin: Goto app mode failed"); return false; } 
  delayMicroseconds(CCS811_WAIT_AFTER_APPSTART_US);
  ok= i2cread(CCS811_STATUS,1,&status);        if( !ok ) { Serial.println("ccs811: begin: STATUS read (app mode) failed"); return false; }
  if( status!=0x90 )                                     { Serial.print  ("ccs811: begin: Not in app mode, or no valid app: ");  Serial.println(status,HEX); return false; }  
  ok= i2cread(CCS811_HW_ID,1,&hw_id);          if( !ok ) { Serial.println("ccs811: begin: HW_ID read failed"); return false; } 
  if( hw_id!=0x81 )                                      { Serial.print  ("ccs811: begin: Wrong HW_ID: ");  Serial.println(hw_id,HEX); return false; }
  ok= i2cread(CCS811_HW_VERSION,1,&hw_version);if( !ok ) { Serial.println("ccs811: begin: HW_VERSION read failed"); return false; } 
  if( hw_version&0xF0!=0x10 )                            { Serial.print  ("ccs811: begin: Wrong HW_VERSION: ");  Serial.println(hw_version,HEX); return false; }
  wake_down();
  return true;
}


// Switched CCS811 to `mode`, use constants CCS811_MODE_XXX. Returns false on problems.
bool CCS811::start( int mode ) {
  uint8_t meas_mode[]= {(uint8_t)(mode<<4)};
  wake_up();
  bool ok = i2cwrite(CCS811_MEAS_MODE,1,meas_mode);
  wake_down();
  return ok;
}


// Get measurement results from the CCS811, check status via errstat, e.g. ccs811_errstat(errstat)
void CCS811::read( uint16_t*eco2, uint16_t*etvoc, uint16_t*errstat,uint16_t*raw) {
  uint8_t buf[8];
  wake_up();
  bool ok = i2cread(CCS811_ALG_RESULT_DATA,8,buf);
  wake_down();
  // Status and error management
  uint16_t combined = buf[5]*256+buf[4];
  if( combined & ~(CCS811_ERRSTAT_HWERRORS|CCS811_ERRSTAT_NEEDS) ) ok= false; // Unused bits are 1: I2C transfer error
  combined &= CCS811_ERRSTAT_HWERRORS|CCS811_ERRSTAT_NEEDS; // Clear all unused bits
  if( !ok ) combined |= CCS811_ERRSTAT_I2CFAIL;
  // Outputs
  if( eco2   ) *eco2   = buf[0]*256+buf[1];
  if( etvoc  ) *etvoc  = buf[2]*256+buf[3];
  if( errstat) *errstat= combined; 
  if( raw    ) *raw    = buf[6]*256+buf[7];;
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


// Gets version of the CCS811 bootloader (returns 0 on I2C failure)
int CCS811::bootloader_version(void) {
  uint8_t buf[2];
  wake_up();
  bool ok = i2cread(CCS811_FW_BOOT_VERSION,2,buf);
  wake_down();  
  int version= 0;
  if( ok ) version= buf[0]*256+buf[1];
  return version;
}
    

// Gets version of the CCS811 application (returns 0 on I2C failure)    
int CCS811::application_version(void) {
  uint8_t buf[2];
  wake_up();
  bool ok = i2cread(CCS811_FW_APP_VERSION,2,buf);
  wake_down();  
  int version= 0;
  if( ok ) version= buf[0]*256+buf[1];
  return version;
}


// Helper interface: nwake pin ========================================================================================


void CCS811::wake_init( void ) {
  if( _nwake>=0 ) pinMode(_nwake, OUTPUT);  
}

void CCS811::wake_up( void) {
  if( _nwake>=0 ) { digitalWrite(_nwake, LOW); delayMicroseconds(CCS811_WAIT_AFTER_WAKE_US);  }
}

void CCS811::wake_down( void) {
  if( _nwake>=0 ) digitalWrite(_nwake, HIGH);
}


// Helper interface: i2c wrapper ======================================================================================


// Writes `count` to register at address `regaddr` in the CCS811. Returns false on I2C problems.
bool CCS811::i2cwrite(int regaddr, int count, uint8_t * buf) {
  Wire.beginTransmission(_slaveaddr);              // START, SLAVEADDR
  Wire.write(regaddr);                             // Register address
  for( int i=0; i<count; i++) Wire.write(buf[i]);  // Write bytes
  int r= Wire.endTransmission(true);               // STOP
  //Serial.printf("ccs811: write4: %02x <- %08x (%d)\n",regaddr,regval,r);
  return r==0;
}

// Reads 'count` bytes from register at address `regaddr`, and stores them in `buf`. Returns false on I2C problems.
bool CCS811::i2cread(int regaddr, int count, uint8_t * buf) {
  Wire.beginTransmission(_slaveaddr);              // START, SLAVEADDR
  Wire.write(regaddr);                             // Register address
  int wres= Wire.endTransmission(false);           // Repeated START
  delayMicroseconds(CCS811_WAIT_REPSTART_US);      // Wait
  int rres=Wire.requestFrom(_slaveaddr,count);     // From CCS811, read bytes, STOP
  for( int i=0; i<count; i++ ) buf[i]=Wire.read();
  return (wres==0) && (rres==count);
}


