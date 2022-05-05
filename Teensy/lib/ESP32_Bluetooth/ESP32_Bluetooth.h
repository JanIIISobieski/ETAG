#pragma once

#include <Arduino.h>
#include <SerialCommunicator.h>

#define FILE_READ_SIZE 2048
#define BLUETOOTH_BUFFER_SIZE 64
#define BLUETOOTH_WRITE_BUFFER 64

class ESP32_Bluetooth : public SerialCommunicator {
    public:
        ESP32_Bluetooth(HardwareSerial* bt_serial, DiskManager* diskManagerPtr);
        ~ESP32_Bluetooth() {};

        void init(uint32_t baud_rate);
        bool file_send(String file_name);

        void enable_passthrough();
        void disable_passthrough();

    private:
        HardwareSerial* hard_serial;
};