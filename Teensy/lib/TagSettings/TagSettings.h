#pragma once

#include <Arduino.h>

#pragma pack(push, 1)
/**
 * @brief Settings for the ADS1299 EEG Chip
 * 
 * This is a union, to make more explicit that is contains both a structure containing the registers,
 * as well as simply being an array of bytes that can be read from a \ref TagCommunicator sending a vector of bytes.
 */
union ADS1299Settings {
    /**
     * @brief The structure that stores the settable parameters of the ADS1299
     * 
     * For full details, see the ADS1299 data sheet
     */
    struct ADSRegisters {
        uint8_t config1 { 0x96 }; /**< Config 1 - Res | Daisy Chain (0) or Multiple Read Back (1) | Oscillator Clock Disabled (0) or Enabled (1) | 1 | 0 | Sampling Rate (000 for 16 kSPS, 001 for 8 kSPS, ... 110 for 250 SPS) */
        uint8_t config2 { 0xC0 }; /**< Config 2 - */
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
#pragma pack(pop)


#define HYDROPHONE_AVG_DEFAULT 0
#define HYDROPHONE_RESOLUTION_DEFAULT 12
#define HYDROPHONE_SAMPLING_FREQUENCY_DEFAULT 40000

#pragma pack(push, 1)
union ADCSettings {
    struct ADCVals {
        int frequency { HYDROPHONE_SAMPLING_FREQUENCY_DEFAULT };
        uint8_t avg { HYDROPHONE_AVG_DEFAULT };
        uint8_t resolution { HYDROPHONE_RESOLUTION_DEFAULT };
    } fields {};
    uint8_t raw_bytes[sizeof(fields)];
};
#pragma pack(pop)

#pragma pack(push, 1)
union DeviceEnable {
    struct {
        boolean nirs_enable {false};
        boolean imu_enable {false};
        boolean eeg_enable {false};
        boolean hydrophone_enable {false};
        boolean bluetooth_sampling_enable {false};
    } fields {};
    boolean raw_bytes[sizeof(fields)];
};
#pragma pack(pop)

#pragma pack(push, 1)
union SettingsPacket {
    struct {
        ADS1299Settings ADS {};
        ADCSettings ADC {};
        DeviceEnable DeviceStart {};
    } settings;
    uint8_t raw_bytes[sizeof(ADS1299Settings) + sizeof(ADCSettings) + sizeof(DeviceEnable)];
};
#pragma pack(pop)
