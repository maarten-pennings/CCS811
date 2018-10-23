/*
  ccs811.cpp - Library for the CCS811 digital gas sensor for monitoring indoor air quality from ams.
  2018 Oct 23  v4  Maarten Pennings  Added envdata/i2cdelay
  2018 Oct 21  v3  Maarten Pennings  Fixed bug in begin(), added hw-version
  2018 Oct 21  v2  Maarten Pennings  Simplified I2C, added version mngt
  2017 Dec 11  v1  Maarten Pennings  Created
*/


#include <Arduino.h>
#include <Wire.h>
#include "ccs811.h"


// begin() prints errors to help diagnose startup problems.
// Change these macro's to empty to suppress those prints.
#define PRINTLN Serial.println
#define PRINT   Serial.print


// Timings
#define CCS811_WAIT_AFTER_RESET_US     2000 // The CCS811 needs a wait after reset
#define CCS811_WAIT_AFTER_APPSTART_US  1000 // The CCS811 needs a wait after app start
#define CCS811_WAIT_AFTER_WAKE_US        50 // The CCS811 needs a wait after WAKE signal


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
  _i2cdelay_us= 0;
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

  // Wakeup CCS811
  wake_up();

    // Try to ping CCS811 (can we reach CCS811 via I2C?)
    ok= i2cwrite(0,0,0);
    if( !ok ) ok= i2cwrite(0,0,0); // retry
    if( !ok ) {
      // Try the other slave address
      _slaveaddr= CCS811_SLAVEADDR_0 + CCS811_SLAVEADDR_1 - _slaveaddr; // swap address
      ok= i2cwrite(0,0,0);
      _slaveaddr= CCS811_SLAVEADDR_0 + CCS811_SLAVEADDR_1 - _slaveaddr; // swap back
      if( ok )
        PRINTLN("ccs811: begin: wrong slave address, ping successful on other address");
      else
        PRINTLN("ccs811: begin: ping failed (VDD/GNF connected? SDA/SCL connected?)");
      goto abort_begin;
    }

    // Invoke a SW reset (bring CCS811 in a know state)
    ok= i2cwrite(CCS811_SW_RESET,4,sw_reset);
    if( !ok ) {
      PRINTLN("ccs811: begin: reset failed");
      goto abort_begin;
    }
    delayMicroseconds(CCS811_WAIT_AFTER_RESET_US);

    // Check that HW_ID is 0x81
    ok= i2cread(CCS811_HW_ID,1,&hw_id);
    if( !ok ) {
      PRINTLN("ccs811: begin: HW_ID read failed");
      goto abort_begin;
    }
    if( hw_id!=0x81 ) {
      PRINT("ccs811: begin: Wrong HW_ID: ");
      PRINTLN(hw_id,HEX);
      goto abort_begin;
    }

    // Check that HW_VERSION is 0x1X
    ok= i2cread(CCS811_HW_VERSION,1,&hw_version);
    if( !ok ) {
      PRINTLN("ccs811: begin: HW_VERSION read failed");
      goto abort_begin;
    }
    if( hw_version&0xF0!=0x10 ) {
      PRINT("ccs811: begin: Wrong HW_VERSION: ");
      PRINTLN(hw_version,HEX);
      goto abort_begin;
    }

    // Check status (after reset, CCS811 should be in boot mode with valid app)
    ok= i2cread(CCS811_STATUS,1,&status);
    if( !ok ) {
      PRINTLN("ccs811: begin: STATUS read (boot mode) failed");
      goto abort_begin;
    }
    if( status!=0x10 ) {
      PRINT("ccs811: begin: Not in boot mode, or no valid app: ");
      PRINTLN(status,HEX);
      goto abort_begin;
    }

    // Switch CCS811 from boot mode into app mode
    ok= i2cwrite(CCS811_APP_START,0,app_start);
    if( !ok ) {
      PRINTLN("ccs811: begin: Goto app mode failed");
      goto abort_begin;
    }
    delayMicroseconds(CCS811_WAIT_AFTER_APPSTART_US);

    // Check if the switch was successful
    ok= i2cread(CCS811_STATUS,1,&status);
    if( !ok ) {
      PRINTLN("ccs811: begin: STATUS read (app mode) failed");
      goto abort_begin;
    }
    if( status!=0x90 ) {
      PRINT("ccs811: begin: Not in app mode, or no valid app: ");
      PRINTLN(status,HEX);
      goto abort_begin;
    }

  // CCS811 back to sleep
  wake_down();
  // Return success
  return true;

abort_begin:
  // CCS811 back to sleep
  wake_down();
  // Return failure
  return false;
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
  if( combined & ~(CCS811_ERRSTAT_HWERRORS|CCS811_ERRSTAT_OK) ) ok= false; // Unused bits are 1: I2C transfer error
  combined &= CCS811_ERRSTAT_HWERRORS|CCS811_ERRSTAT_OK; // Clear all unused bits
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
  // First the ERROR_ID flags
                                                  s[ 0]='-';
                                                  s[ 1]='-';
  if( errstat & CCS811_ERRSTAT_HEATER_SUPPLY    ) s[ 2]='V'; else s[2]='v';
  if( errstat & CCS811_ERRSTAT_HEATER_FAULT     ) s[ 3]='H'; else s[3]='h';
  if( errstat & CCS811_ERRSTAT_MAX_RESISTANCE   ) s[ 4]='X'; else s[4]='x';
  if( errstat & CCS811_ERRSTAT_MEASMODE_INVALID ) s[ 5]='M'; else s[5]='m';
  if( errstat & CCS811_ERRSTAT_READ_REG_INVALID ) s[ 6]='R'; else s[6]='r';
  if( errstat & CCS811_ERRSTAT_WRITE_REG_INVALID) s[ 7]='W'; else s[7]='w';
  // Then the STATUS flags
  if( errstat & CCS811_ERRSTAT_FW_MODE          ) s[ 8]='F'; else s[8]='f';
                                                  s[ 9]='-';
                                                  s[10]='-';
  if( errstat & CCS811_ERRSTAT_APP_VALID        ) s[11]='A'; else s[11]='a';
  if( errstat & CCS811_ERRSTAT_DATA_READY       ) s[12]='D'; else s[12]='d';
                                                  s[13]='-';
  // Next bit is used by SW to signal I2C transfer error
  if( errstat & CCS811_ERRSTAT_I2CFAIL          ) s[14]='I'; else s[14]='i';
  if( errstat & CCS811_ERRSTAT_ERROR            ) s[15]='E'; else s[15]='e';
                                                  s[16]='\0';
  return s;
}


// Extra interface ========================================================================================


// Gets version of the CCS811 hardware (returns 0 on I2C failure)
int CCS811::hardware_version(void) {
  uint8_t buf[1];
  wake_up();
  bool ok = i2cread(CCS811_HW_VERSION,1,buf);
  wake_down();
  int version= 0;
  if( ok ) version= buf[0];
  return version;
}


// Gets version of the CCS811 boot loader (returns 0 on I2C failure)
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


// Writes t and h to ENV_DATA (see datasheet for format). Returns false on I2C problems.
bool CCS811::set_envdata(uint16 t, uint16 h) {
  uint8_t envdata[]= { (h>>8)&0xff, (h>>0)&0xff, (t>>8)&0xff, (t>>0)&0xff };
  wake_up();
  // Serial.print(" [T="); Serial.print(t); Serial.print(" H="); Serial.print(h); Serial.println("] ");
  bool ok = i2cwrite(CCS811_ENV_DATA,2,envdata);
  wake_down();
  return ok;
}


// Writes t and h (in ENS210 format) to ENV_DATA. Returns false on I2C problems.
bool CCS811::set_envdata210(uint16 t, uint16 h) {
  uint16_t offset= (273.15-25)*64;
  if( t<offset ) t=offset;
  if( t>UINT16_MAX/8+offset ) t= UINT16_MAX/8+offset;
  return set_envdata( (t-offset)*8 , h);
}
 
 
// Advanced interface: i2cdelay ========================================================================================


// Delay before a repeated start - needed for e.g. ESP8266 because it doesn't handle I2C clock stretch correctly
void CCS811::set_i2cdelay(int us) {
  if( us<0 ) us=0;
  _i2cdelay_us= us;
}


// Get current delay
int  CCS811::get_i2cdelay(void) {
  return _i2cdelay_us;
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


// Writes `count` from `buf` to register at address `regaddr` in the CCS811. Returns false on I2C problems.
bool CCS811::i2cwrite(int regaddr, int count, uint8_t * buf) {
  Wire.beginTransmission(_slaveaddr);              // START, SLAVEADDR
  Wire.write(regaddr);                             // Register address
  for( int i=0; i<count; i++) Wire.write(buf[i]);  // Write bytes
  int r= Wire.endTransmission(true);               // STOP
  return r==0;
}

// Reads 'count` bytes from register at address `regaddr`, and stores them in `buf`. Returns false on I2C problems.
bool CCS811::i2cread(int regaddr, int count, uint8_t * buf) {
  Wire.beginTransmission(_slaveaddr);              // START, SLAVEADDR
  Wire.write(regaddr);                             // Register address
  int wres= Wire.endTransmission(false);           // Repeated START
  delayMicroseconds(_i2cdelay_us);                 // Wait
  int rres=Wire.requestFrom(_slaveaddr,count);     // From CCS811, read bytes, STOP
  for( int i=0; i<count; i++ ) buf[i]=Wire.read();
  return (wres==0) && (rres==count);
}


