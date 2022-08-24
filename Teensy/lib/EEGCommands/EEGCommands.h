#pragma once

#include <Arduino.h>
#include "EEGSettings.h"

class EEGCommands {
    public:
        EEGCommands() : cs(0) {};
        EEGCommands(uint8_t CS) : cs(CS) {};

        void write_all_registers();
        void write_register(uint8_t address, uint8_t value);
        void read_register(uint8_t address);

        /**** SPI Command Defintions (Datasheet, pg 40) ****/
        void wakeup();      //**< Wake-up from standby mode */
        void standby();     //**< Enter standby mode */
        void reset();       //**< Reset the device */
        void start();       //**< Start and restart (synchronize) conversions */
        void stop();        //**< Stop conversion */
        void rdata();       //**< Read data by command */   
        void rdatac();      //**< Enable Read Data Continuous mode (default at powerup)  */
        void sdatac();      //**< Stop Read Data Continuously mode */
        void get_id();      //**< Get the ID byte */

    private:
        SPISettings spi_settings;
        EEGSettings registers;
        void spi_start();
        void spi_end();
        void send_command(uint8_t command);

        uint8_t cs;
};