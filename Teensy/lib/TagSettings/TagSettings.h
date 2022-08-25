#pragma once

#include <Arduino.h>

#pragma pack(push, 1)
union ADS1299Settings {
    struct {
        uint8_t config1;
        uint8_t config2;
        uint8_t config3;
        uint8_t loff;
        uint8_t chnset[8];
        uint8_t bias_sensp;
        uint8_t bias_sensn;
        uint8_t loff_sensp;
        uint8_t loff_sensn;
        uint8_t loff_flip;
        uint8_t loff_statp;
        uint8_t loff_statn;
        uint8_t gpio;
    } fields;
    uint8_t raw_bytes[sizeof(fields)];
};

union ADCSettings {
    struct {
        int frequency;
        uint8_t avg;
        uint8_t resolution;
    } fields;
    uint8_t raw_bytes[sizeof(fields)];
};

union DeviceEnable {
    struct {
        boolean nirs_enable {false};
        boolean imu_enable {true};
        boolean eeg_enable {false};
        boolean hydrophone_enable {false};
        boolean bluetooth_sampling_enable {false};
    } fields;
    boolean raw_bytes[sizeof(fields)];
};

union SettingsPacket {
    struct {
        ADS1299Settings ADS;
        ADCSettings ADC;
        DeviceEnable DeviceStart;
    } settings;
    uint8_t raw_bytes[sizeof(ADS1299Settings) + sizeof(ADCSettings) + sizeof(DeviceEnable)];
};

#pragma pack(pop)