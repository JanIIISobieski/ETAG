//
// ADS1299_Definitions.h
//

#ifndef _ADS1299_Definitions_h
#define _ADS1299_Definitions_h

/**** SPI Command Defintions (Datasheet, pg 40) ****/
// System Commands
#define _WAKEUP     0x02    // Wake-up from standby mode
#define _STANDBY    0x04    // Enter standby mode
#define _RESET      0x06    // Reset the device
#define _START      0x08    // Start and restart (synchronize) conversions
#define _STOP       0x0A    // Stop conversion

// Data Read Commands
#define _RDATAC     0x10    // Enable Read Data Continuous mode (default at powerup)
#define _SDATAC     0x11    // Stop Read Data Continuously mode
#define _RDATA      0x12    // Read data by command

// Register Read/Write Commands
#define _RREG       0x20    // Read registers (see Datasheet pg 43 for more details)
#define _WREG       0x40    // Write registers (see Datasheet pg 43 for more details)
/***************************************************/

/**** Register Maps (Datasheet, pg 44) ****/
// Read Only ID Register
#define EEG_ID      0x00

// Global Settings Across Channels
#define CONFIG1     0x01
#define CONFIG2     0x02
#define CONFIG3     0x03
#define LOFF        0x04

// Channel Specific Settings
#define CH1SET      0x05
#define CH2SET      0x06
#define CH3SET      0x07
#define CH4SET      0x08
#define CH5SET      0x09
#define CH6SET      0x0A
#define CH7SET      0x0B
#define CH8SET      0x0C
#define BIAS_SENSP  0x0D
#define BIAS_SENSN  0x0E
#define LOFF_SENSP  0x0F
#define LOFF_SENSN  0x10
#define LOFF_FLIP   0x11

// Lead-Off Status Registers (Read Only Registers)
#define LOFF_STATP  0x12
#define LOFF_STATN  0x13

// GPIO and OTHER Registers
#define GPIO        0x14
#define MISC1       0x15
#define MISC2       0x16
#define CONFIG4     0x17
/******************************************/

#define CHANNELS_PER_SAMPLE  8
#define BYTES_PER_CHANNEL    3
#define BYTES_PER_SAMPLE    27

#define V_REF       4.5

#endif
