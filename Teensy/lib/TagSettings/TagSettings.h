#pragma once

#include <Arduino.h>

#pragma pack(push, 1)

union ADS1299Settings {
    struct ADSRegisters {
        uint8_t config1 { 0x96 };
        uint8_t config2 { 0xC0 };
        uint8_t config3 { 0x60 };
        uint8_t loff { 0x00 };
        uint8_t chnset[8] { 0x61, 0x61, 0x61, 0x61, 0x00, 0x00, 0x00, 0x00 };
        uint8_t bias_sensp { 0x00 };
        uint8_t bias_sensn { 0x00 };
        uint8_t loff_sensp { 0x00 };
        uint8_t loff_sensn { 0x00};
        uint8_t loff_flip { 0x00 };
        uint8_t misc1 { 0x00 };
    } fields {};  // the {} after fields is for the default initalization of fields
    uint8_t raw_bytes[sizeof(fields)];
};


#define HYDROPHONE_AVG_DEFAULT 0
#define HYDROPHONE_RESOLUTION_DEFAULT 12
#define HYDROPHONE_SAMPLING_FREQUENCY_DEFAULT 40000
union ADCSettings {
    struct ADCVals {
        int frequency { HYDROPHONE_SAMPLING_FREQUENCY_DEFAULT };
        uint8_t avg { HYDROPHONE_AVG_DEFAULT };
        uint8_t resolution { HYDROPHONE_RESOLUTION_DEFAULT };
    } fields;
    uint8_t raw_bytes[sizeof(fields)];
};

union DeviceEnable {
    struct {
        boolean nirs_enable {false};
        boolean imu_enable {false};
        boolean eeg_enable {false};
        boolean hydrophone_enable {true};
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