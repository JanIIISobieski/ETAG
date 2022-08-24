#pragma once

#include <SPI.h>
#include "ADS1299_Definitions.h"
#include "Arduino.h"

#define EEG_POST_SPI_TRANSFER_DELAY_MICROSECONDS 2
#define NUM_EEG_PINS 8

class Register {
    public:
        Register() : value(0), mem_offset(0) {};
        Register(uint8_t val, uint8_t offset) : value(val), mem_offset(offset) {};

        void set(uint8_t val, uint8_t offset) { value = val; mem_offset = offset; };
        void set(uint8_t val) { value = val; };

        uint8_t value;
        uint8_t mem_offset;
};

class EEGSettings {
    public:
        EEGSettings() : config1{0x96, CONFIG1},
                        config2{0xC0, CONFIG2},
                        config3{0x60, CONFIG3},
                        config4{0x00, CONFIG4},
                        loff{0x00, LOFF},
                        bias_sensp{0x00, BIAS_SENSP},
                        bias_sensn{0x00, BIAS_SENSN},
                        loff_sensp{0x00, LOFF_SENSP},
                        loff_sensn{0x00, LOFF_SENSN},
                        misc1{0x00, MISC1},
                        misc2{0x00, MISC2},
                        gpio{0x0F, GPIO}
                        {
                            for (size_t i = 0; i < NUM_EEG_PINS; i++) {
                                ch_n_set[i].set(0x00, 0x05+i);
                            }
                        };

        EEGSettings(size_t num_channels) : config1{0x96, CONFIG1},
                                           config2{0xC0, CONFIG2},
                                           config3{0x60, CONFIG3},
                                           config4{0x00, CONFIG4},
                                           loff{0x00, LOFF},
                                           bias_sensp{0x00, BIAS_SENSP},
                                           bias_sensn{0x00, BIAS_SENSN},
                                           loff_sensp{0x00, LOFF_SENSP},
                                           loff_sensn{0x00, LOFF_SENSN},
                                           misc1{0x00, MISC1},
                                           misc2{0x00, MISC2},
                                           gpio{0x0F, GPIO}
                                           {
                                                for (size_t i = 0; i < num_channels; i++) {
                                                    ch_n_set[i].set(0x61, 0x05+i);
                                                }

                                                for (size_t i = num_channels; i < NUM_EEG_PINS; i++ ) {
                                                    ch_n_set[i].set(0x00, 0x05+i);
                                                }
                                            };

        Register config1;
        Register config2;
        Register config3;
        Register config4;
        Register loff;
        Register bias_sensp;
        Register bias_sensn;
        Register loff_sensp;
        Register loff_sensn;
        Register misc1;
        Register misc2;
        Register gpio;
        Register ch_n_set[NUM_EEG_PINS];
};