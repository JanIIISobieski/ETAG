#pragma once

#include <Arduino.h>

#pragma pack(push, 1)
union ADS1299Settings {
    struct {
        uint8_t config1;
        uint8_t config3;
        uint8_t bias_sensp;
        uint8_t misc1;
        uint8_t chnset[8];
    } fields;
    uint8_t raw_bytes[12];
};

union ADCSettings {
    struct {
        int frequency;
        uint8_t avg;
        uint8_t resolution;
    } fields;
    uint8_t raw_bytes[6];
};

union DeviceEnable {
    struct {
        boolean imu_enable;
        boolean hydrophone_enable;
        boolean eeg_enable;
    } fields;
    uint8_t raw_bytes[3];
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