/*
  ccs811.h - Library for the CCS811 digital gas sensor for monitoring indoor air quality from ams.
  2019 jan 22  v10  Maarten Pennings  Added F() on all strings, added get/set_baseline()
  2019 jan 15   v9  Maarten Pennings  Flash&i2cwrite now use const array
  2018 dec 06   v8  Maarten Pennings  Added firmware flash routine
  2018 Dec 04   v7  Maarten Pennings  Added support for older CCS811's (fw 1100)
  2018 Nov 11   v6  Maarten Pennings  uint16 -> uint16_t
  2018 Nov 02   v5  Maarten Pennings  Added clearing of ERROR_ID
  2018 Oct 23   v4  Maarten Pennings  Added envdata/i2cdelay
  2018 Oct 21   v3  Maarten Pennings  Added hw-version
  2018 Oct 21   v2  Maarten Pennings  Simplified I2C, added version mngt
  2017 Dec 11   v1  Maarten Pennings  Created
*/
#ifndef _CCS811_H_
#define _CCS811_H_


// To help diagnose problems, the begin() and flash() functions print diagnostic messages to Serial.
// If you do not want that, make the PRINT macros in ccs811.cpp empty.


#include <stdint.h>


// Version of this CCS811 driver
#define CCS811_VERSION                     10


// I2C slave address for ADDR 0 respectively 1
#define CCS811_SLAVEADDR_0                 0x5A
#define CCS811_SLAVEADDR_1                 0x5B


// The values for mode in ccs811_start()
#define CCS811_MODE_IDLE                   0
#define CCS811_MODE_1SEC                   1
#define CCS811_MODE_10SEC                  2
#define CCS811_MODE_60SEC                  3


// The flags for errstat in ccs811_read()
// ERRSTAT is a merge of two hardware registers: ERROR_ID (bits 15-8) and STATUS (bits 7-0)
// Also bit 1 (which is always 0 in hardware) is set to 1 when an I2C read error occurs
#define CCS811_ERRSTAT_ERROR               0x0001 // There is an error, the ERROR_ID register (0xE0) contains the error source
#define CCS811_ERRSTAT_I2CFAIL             0x0002 // Bit flag added by software: I2C transaction error
#define CCS811_ERRSTAT_DATA_READY          0x0008 // A new data sample is ready in ALG_RESULT_DATA
#define CCS811_ERRSTAT_APP_VALID           0x0010 // Valid application firmware loaded
#define CCS811_ERRSTAT_FW_MODE             0x0080 // Firmware is in application mode (not boot mode)
#define CCS811_ERRSTAT_WRITE_REG_INVALID   0x0100 // The CCS811 received an I²C write request addressed to this station but with invalid register address ID
#define CCS811_ERRSTAT_READ_REG_INVALID    0x0200 // The CCS811 received an I²C read request to a mailbox ID that is invalid
#define CCS811_ERRSTAT_MEASMODE_INVALID    0x0400 // The CCS811 received an I²C request to write an unsupported mode to MEAS_MODE
#define CCS811_ERRSTAT_MAX_RESISTANCE      0x0800 // The sensor resistance measurement has reached or exceeded the maximum range
#define CCS811_ERRSTAT_HEATER_FAULT        0x1000 // The heater current in the CCS811 is not in range
#define CCS811_ERRSTAT_HEATER_SUPPLY       0x2000 // The heater voltage is not being applied correctly

// These flags should not be set. They flag errors.
#define CCS811_ERRSTAT_HWERRORS            ( CCS811_ERRSTAT_ERROR | CCS811_ERRSTAT_WRITE_REG_INVALID | CCS811_ERRSTAT_READ_REG_INVALID | CCS811_ERRSTAT_MEASMODE_INVALID | CCS811_ERRSTAT_MAX_RESISTANCE | CCS811_ERRSTAT_HEATER_FAULT | CCS811_ERRSTAT_HEATER_SUPPLY )
#define CCS811_ERRSTAT_ERRORS              ( CCS811_ERRSTAT_I2CFAIL | CCS811_ERRSTAT_HWERRORS )
// These flags should normally be set - after a measurement. They flag data available (and valid app running).
#define CCS811_ERRSTAT_OK                  ( CCS811_ERRSTAT_DATA_READY | CCS811_ERRSTAT_APP_VALID | CCS811_ERRSTAT_FW_MODE )
// These flags could be set after a measurement. They flag data is not yet available (and valid app running).
#define CCS811_ERRSTAT_OK_NODATA           ( CCS811_ERRSTAT_APP_VALID | CCS811_ERRSTAT_FW_MODE )


class CCS811 {
  public: // Main interface
    CCS811(int nwake=-1, int slaveaddr=CCS811_SLAVEADDR_0);                   // Pin number connected to nWAKE (nWAKE can also be bound to GND, then pass -1), slave address (5A or 5B)
    bool begin( void );                                                       // Reset the CCS811, switch to app mode and check HW_ID. Returns false on problems.
    bool start( int mode );                                                   // Switched CCS811 to `mode`, use constants CCS811_MODE_XXX. Returns false on I2C problems.
    void read( uint16_t*eco2, uint16_t*etvoc, uint16_t*errstat,uint16_t*raw); // Get measurement results from the CCS811 (all args may be NULL), check status via errstat, e.g. ccs811_errstat(errstat)
    const char * errstat_str(uint16_t errstat);                               // Returns a string version of an errstat. Note, each call, this string is updated.
  public: // Extra interface
    int  hardware_version(void);                                              // Gets version of the CCS811 hardware (returns -1 on I2C failure)
    int  bootloader_version(void);                                            // Gets version of the CCS811 bootloader (returns -1 on I2C failure)
    int  application_version(void);                                           // Gets version of the CCS811 application (returns -1 on I2C failure)
    int  get_errorid(void);                                                   // Gets the ERROR_ID [same as 'err' part of 'errstat' in 'read'] (returns -1 on I2C failure)
    bool set_envdata(uint16_t t, uint16_t h);                                 // Writes t and h to ENV_DATA (see datasheet for format). Returns false on I2C problems.
    bool set_envdata210(uint16_t t, uint16_t h);                              // Writes t and h (in ENS210 format) to ENV_DATA. Returns false on I2C problems.
    bool get_baseline(uint16_t *baseline);                                    // Reads (encoded) baseline from BASELINE. Returns false on I2C problems. Get it, just before power down (but only when sensor was on at least 20min) - see CCS811_AN000370
    bool set_baseline(uint16_t baseline);                                     // Writes (encoded) baseline to BASELINE. Returns false on I2C problems. Set it, after power up (and after 20min)
    bool flash(const uint8_t * image, int size);                              // Flashes the firmware of the CCS811 with size bytes from image - image _must_ be in PROGMEM
  public: // Advanced interface: i2cdelay
    void set_i2cdelay(int us);                                                // Delay before a repeated start - needed for e.g. ESP8266 because it doesn't handle I2C clock stretch correctly
    int  get_i2cdelay(void);                                                  // Get current delay
  protected: // Helper interface: nwake pin
    void wake_init(void);                                                     // Initialize nwake pin
    void wake_up(void);                                                       // Wake up CCS811, i.e. pull nwake pin low
    void wake_down(void);                                                     // CCS811 back to sleep, i.e. pull nwake pin high
  protected: // Helper interface: i2c wrapper
    bool i2cwrite(int regaddr, int count, const uint8_t * buf);               // Writes `count` from `buf` to register at address `regaddr` in the CCS811. Returns false on I2C problems.
    bool i2cread (int regaddr, int count, uint8_t * buf);                     // Reads 'count` bytes from register at address `regaddr`, and stores them in `buf`. Returns false on I2C problems.
  private:
    int  _nwake;                                                              // Pin number for nWAKE pin (or -1)
    int  _slaveaddr;                                                          // Slave address of the CCS811
    int  _i2cdelay_us;                                                        // Delay in us just before an I2C repeated start condition
    int  _appversion;                                                         // Version of the app firmware inside the CCS811 (for workarounds)
};


#endif

