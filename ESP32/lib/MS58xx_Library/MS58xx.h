//
// TE 58xx Pressure Sensor
// MS58xx.h
//

#ifndef _MS58xx_
#define _MS58xx_

#include <Arduino.h>
#include <SPI.h>
#include "MS58xx_Definitions.h"

class MS58xx {
  public:
    MS58xx();

    uint8_t cs;        // Chip Select pin number
    SPISettings MS58xx_SPI_settings;

    uint16_t PSENS;     // C1 - pressure sensitivity
    uint16_t POFF;      // C2 - pressure offset
    uint16_t TCSENS;    // C3 - temperature coefficient of pressure sensitivity
    uint16_t TCOFF;     // C4 - temperature coefficient of pressure offset
    uint16_t TREF;      // C5 - reference temperature
    uint16_t TEMPSENS;  // C6 - temperature sensitivity coefficient

    uint8_t Pbuff[3];
    uint8_t Tbuff[3];

    volatile float pressure_mbar;
    volatile float temperature;

    bool data_ready;

    void init(uint8_t _CS);
    void spi_start(uint8_t _CS, SPISettings _SPI_settings);
    void spi_end(uint8_t _CS);
    void calc_press_temp();    

    void update_pt_data();

    /**** Command Defintions (Datasheet, pg 8) ****/
    void reset();
    void get_PSENS();
    void get_POFF();
    void get_TCSENS();
    void get_TCOFF();
    void get_TREF();
    void get_TEMPSENS();
    void update_press();
    void update_temp();
    void read_press();
    void read_temp();
    /**********************************************/

    uint32_t time_start;
    uint8_t ind;
};

extern MS58xx press_sens;

#endif
