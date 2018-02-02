/* 
  ccs811.h - Library for the CCS811 digital gas sensor for monitoring indoor air quality from ams.
  Created by Maarten Pennings 2017 Dec 11
*/
#ifndef _CCS811_H_
#define _CCS811_H_


#include <stdint.h>


// I2C slave address
#define CCS811_SLAVADDR_0                  0x5A
#define CCS811_SLAVADDR_1                  0x5B


// The values for mode in ccs811_start()
#define CCS811_MODE_IDLE                   0
#define CCS811_MODE_1SEC                   1
#define CCS811_MODE_10SEC                  2
#define CCS811_MODE_60SEC                  3


// The flags for errstat in ccs811_read()
#define CCS811_ERRSTAT_ERROR               0x0001 // There is an error, the ERROR_ID register (0xE0) contains the error source
#define CCS811_ERRSTAT_I2CFAIL             0x0002 // Added by software: I2C transaction error
#define CCS811_ERRSTAT_DATA_READY          0x0008 // A new data sample is ready in ALG_RESULT_DATA
#define CCS811_ERRSTAT_APP_VALID           0x0010 // Valid application firmware loaded
#define CCS811_ERRSTAT_FW_MODE             0x0080 // Firmware is in application mode (not boot mode)
#define CCS811_ERRSTAT_WRITE_REG_INVALID   0x0100 // The CCS811 received an I²C write request addressed to this station but with invalid register address ID
#define CCS811_ERRSTAT_READ_REG_INVALID    0x0200 // The CCS811 received an I²C read request to a mailbox ID that is invalid
#define CCS811_ERRSTAT_MEASMODE_INVALID    0x0400 // The CCS811 received an I²C request to write an unsupported mode to MEAS_MODE
#define CCS811_ERRSTAT_MAX_RESISTANCE      0x0800 // The sensor resistance measurement has reached or exceeded the maximum range
#define CCS811_ERRSTAT_HEATER_FAULT        0x1000 // The Heater current in the CCS811 is not in range
#define CCS811_ERRSTAT_HEATER_SUPPLY       0x2000 // The Heater voltage is not being applied correctly


class CCS811 {
  public: // Main interface
    CCS811(int nwake=-1, int slaveaddr=CCS811_SLAVADDR_0);                    // Pin number connected to nWAKE (nWAKE can also be bound to GND, then pass -1), slave address (5A or 5B)
    bool begin( void );                                                       // Reset the CCS811, switch to app mode and check HW_ID. Returns false on problems.
    bool versions(uint8_t*hw, uint16_t*fw_boot, uint16_t*fw_app);             // Get Hardware Version, FW_Boot_Version, FW_App_Version
    bool start( int mode );                                                   // Switched CCS811 to `mode`, use constants CCS811_MODE_XXX. Returns false on I2C problems.
    void read( uint16_t*eco2, uint16_t*etvoc, uint16_t*errstat,uint16_t*raw); // Get measurement results from the CCS811, check status via errstat, e.g. ccs811_errstat(errstat)
    bool errstat_ok(int errstat );                                            // Returns true if errstat flags denote NEW&READY, false for OLD|ERROR
    const char * errstat_str(uint16_t errstat);                               // Returns a string version of an errstat. Note, each call, this string is updated.
  protected: // Helper interface: nwake pin
    void wake_init( void );                                                   // Initialize nwake pin
    void wake_up( void);                                                      // Assert nwake pin
    void wake_down( void);                                                    // Deassert nwake pin
  protected: // Helper interface: i2c wrapper
    bool i2cwrite0(int regaddr);                                              // Writes nothing to register at address `regaddr` in the CCS811. Returns false on I2C problems.
    bool i2cwrite1(int regaddr, uint8_t    regval);                           // Writes 8bit `regval` to register at address `regaddr` in the CCS811. Returns false on I2C problems.
    bool i2cwrite2(int regaddr, uint16_t   regval);                           // Writes 16bit `regval` to register at address `regaddr` in the CCS811. Returns false on I2C problems.
    bool i2cwrite4(int regaddr, uint32_t   regval);                           // Writes 32bit `regval` to register at address `regaddr` in the CCS811. Returns false on I2C problems.
    bool i2cread1 (int regaddr, uint8_t *  regval);                           // Reads 1 byte from register at address `regaddr`, and store it in `regval`. Returns false on I2C problems.
    bool i2cread2 (int regaddr, uint16_t * regval);                           // Reads 2 bytes from register at address `regaddr`, and stores them in `regval`. Returns false on I2C problems.
    bool i2cread4 (int regaddr, uint32_t * regval);                           // Reads 4 bytes from register at address `regaddr`, and stores them in `regval`. Returns false on I2C problems.
    bool i2cread8 (int regaddr, uint32_t * msb, uint32 * lsb);                // Reads 8 bytes from register at address `regaddr`, and stores them in `regval`. Returns false on I2C problems.
  private:
    int  _nwake;                                                              // Pin number for nWAKE pin (or -1)
    int  _slaveaddr;                                                          // Slave address of the CCS811
};

                                         
#endif


