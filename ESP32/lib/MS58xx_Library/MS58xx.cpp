//
// TE 58xx Pressure Sensor
// MS58xx.cpp
//

#include "MS58xx.h"

#define MS58xx_02
//#define MS58xx_30

#define REQUEST_READ_DELAY 10

void MS58xx::init(uint8_t _CS) {
  cs = _CS;
  MS58xx_SPI_settings = SPISettings(4000000, MSBFIRST, SPI_MODE0);
  SPI.begin(18, 19, 23, cs);

  reset();
  get_PSENS();
  get_POFF();
  get_TCSENS();
  get_TCOFF();
  get_TREF();
  get_TEMPSENS();

  ind = 0;
  data_ready = false;
}

void MS58xx::spi_start(uint8_t _CS, SPISettings _SPI_settings) {
  SPI.beginTransaction(_SPI_settings);
  digitalWrite(_CS, LOW);   // Start communication
}

void MS58xx::spi_end(uint8_t _CS) {
  digitalWrite(_CS, HIGH);
  SPI.endTransaction();
}

void MS58xx::update_pt_data() {
  switch (ind) {
    case (0):
      update_press();
      dt = 0;
      ++ind;
      break;
    case (1):
      if (dt >= REQUEST_READ_DELAY) {
        ++ind;
      } 
    case (2):
      read_press();
      update_temp();
      dt = 0;
      ++ind;
    case (3):
      if (dt >= REQUEST_READ_DELAY) {
        ++ind;
      }
    case (4):
      read_temp();
      ind = 0;
      data_ready = true;
    default:
      break;
  }
}


void MS58xx::calc_press_temp() {
  uint32_t D1 = (uint32_t) ((((uint32_t) Pbuff[0] << 16) | ((uint32_t) Pbuff[1] << 8) | ((uint32_t) Pbuff[2])));
  uint32_t D2 = (uint32_t) ((((uint32_t) Tbuff[0] << 16) | ((uint32_t) Tbuff[1] << 8) | ((uint32_t) Tbuff[2])));

  float dT = (float) D2 - ((float) TREF * POW2_8);
  float T16 = 2000.0 + (dT * (float) TEMPSENS / (float) POW2_23);
  temperature = T16 / 100.0;

  float OFF, SENS, MS58xx_const;
  #ifdef MS58xx_02
    OFF = ((float) POFF * POW2_17) + (((float) TCOFF * dT) / POW2_6);
    SENS = ((float) PSENS * POW2_16) + ((dT * (float) TCSENS) / POW2_7);
    MS58xx_const = (float) POW2_15;
  #endif
  #ifdef MS58xx_30
    OFF = ((float) POFF * POW2_16) + (((float) TCOFF * dT) / POW2_7);
    SENS = ((float) PSENS * POW2_15) + ((dT * (float) TCSENS) / POW2_8);
    MS58xx_const = (float) POW2_13;
  #endif
  pressure_mbar = (((float) D1 * SENS / POW2_21) - OFF) / MS58xx_const;
}

// System Commands
void MS58xx::reset() {
  spi_start(cs, MS58xx_SPI_settings); // Start communication
  SPI.transfer(_RESET);
  delay(10);
  spi_end(cs);                        // end communication
  delay(5);
}

void MS58xx::get_PSENS() {
  PSENS = 0;

  spi_start(cs, MS58xx_SPI_settings); // Start communication
  SPI.transfer(_PSENS);
  PSENS = SPI.transfer(0x00);
  PSENS = PSENS << 8;
  PSENS |= SPI.transfer(0x00);
  spi_end(cs);                        // end communication
}

void MS58xx::get_POFF() {
  POFF = 0;

  spi_start(cs, MS58xx_SPI_settings); // Start communication
  SPI.transfer(_POFF);
  POFF = SPI.transfer(0x00);
  POFF = POFF << 8;
  POFF |= SPI.transfer(0x00);
  spi_end(cs);                        // end communication
}

void MS58xx::get_TCSENS() {
  TCSENS = 0;

  spi_start(cs, MS58xx_SPI_settings); // Start communication
  SPI.transfer(_TCSENS);
  TCSENS = SPI.transfer(0x00);
  TCSENS = TCSENS << 8;
  TCSENS |= SPI.transfer(0x00);
  spi_end(cs);                        // end communication
}

void MS58xx::get_TCOFF() {
  TCOFF = 0;

  spi_start(cs, MS58xx_SPI_settings); // Start communication
  SPI.transfer(_TCOFF);
  TCOFF = SPI.transfer(0x00);
  TCOFF = TCOFF << 8;
  TCOFF |= SPI.transfer(0x00);
  spi_end(cs);                        // end communication
}

void MS58xx::get_TREF() {
  TREF = 0;

  spi_start(cs, MS58xx_SPI_settings); // Start communication
  SPI.transfer(_TREF);
  TREF = SPI.transfer(0x00);
  TREF = TREF << 8;
  TREF |= SPI.transfer(0x00);
  spi_end(cs);                        // end communication
}

void MS58xx::get_TEMPSENS() {
  TEMPSENS = 0;

  spi_start(cs, MS58xx_SPI_settings); // Start communication
  SPI.transfer(_TEMPSENS);
  TEMPSENS = SPI.transfer(0x00);
  TEMPSENS = TEMPSENS << 8;
  TEMPSENS |= SPI.transfer(0x00);
  spi_end(cs);                        // end communication
}

void MS58xx::update_press() {
  spi_start(cs, MS58xx_SPI_settings); // Start communication
  SPI.transfer(_D1);                  // initiate pressure conversion OSR-4096
  spi_end(cs);                        // end communication
}

void MS58xx::update_temp() {
  spi_start(cs, MS58xx_SPI_settings); // Start communication
  SPI.transfer(_D2);                  // initiate temperature conversion OSR-4096
  spi_end(cs);                        // end communication
}

void MS58xx::read_press() {
  uint32_t i;

  spi_start(cs, MS58xx_SPI_settings); // Start communication
  SPI.transfer(_ADC);
  for (i = 0; i < 3; i++) {
    Pbuff[i] = SPI.transfer(0x00);
  }
  spi_end(cs);                        // end communication
}

void MS58xx::read_temp() {
  uint32_t i;

  spi_start(cs, MS58xx_SPI_settings); // Start communication
  SPI.transfer(_ADC);
  for (i = 0; i < 3; i++) {
    Tbuff[i] = SPI.transfer(0x00);
  }
  spi_end(cs);                        // end communication
}

MS58xx press_sens;
