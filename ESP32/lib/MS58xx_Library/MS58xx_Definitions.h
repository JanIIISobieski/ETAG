//
// TE 58xx Pressure Sensor
// MS58xx_Definitions.h
//

#ifndef _MS58xx_Definitions_h
#define _MS58xx_Definitions_h

#define POW2_6  64
#define POW2_7  128
#define POW2_8  256
#define POW2_13 8192
#define POW2_15 32768
#define POW2_16 65536
#define POW2_17 131072
#define POW2_21 2097152
#define POW2_23 8388608

/**** SPI Command Defintions (Datasheet, pg 8) ****/
#define _RESET    0x1E  // reset device
#define _PSENS    0xA2  // read pressure sensitivity
#define _POFF     0xA4  // read pressure offset
#define _TCSENS   0xA6  // read temperature coefficient of pressure sensitivity
#define _TCOFF    0xA8  // read temperature coefficient of pressure offset
#define _TREF     0xAA  // read reference temperature
#define _TEMPSENS 0xAC  // read temperature sensitivity coefficient
#define _D1       0x48  // initiate pressure conversion OSR-4096
#define _D2       0x58  // initiate temperature conversion OSR-4096
#define _ADC      0x00  // read ADC result (24 bit pressure or temperature)
/**************************************************/

#endif
