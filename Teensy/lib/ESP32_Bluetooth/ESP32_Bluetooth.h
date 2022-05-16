#pragma once

#include <Arduino.h>
#include <SerialCommunicator.h>

#define FILE_READ_SIZE 2048
#define BLUETOOTH_BUFFER_SIZE 64
#define BLUETOOTH_WRITE_BUFFER 64

#define MIN_SPEED_DELAY 10
#define MIN_PRESSURE_DELAY 10

#define PHRASE_LEN 7

union TimePressureTemperature {
    struct DataStruct {
        uint32_t time;
        ByteArray<float> pressure;
        ByteArray<float> temperature;
    } tpt_struct;
    uint8_t buffer[12];
};

union TimeSpeed {
    struct DataStruct {
        uint32_t time;
        ByteArray<int32_t> speed;
    } st_struct;
    uint8_t buffer[8];
};

class ESP32_Bluetooth : public SerialCommunicator {
    public:
        ESP32_Bluetooth(HardwareSerial* bt_serial, DiskManager* diskManagerPtr);
        ~ESP32_Bluetooth() {};

        void init(uint32_t baud_rate);
        bool file_send(String file_name);

        bool sample_pressure_temperature();
        bool sample_speed();
        int32_t sample_saltwater_sensor();

        uint8_t* get_pressure_temperature() { return pressure_temp_vals.buffer; };
        uint8_t* get_speed() { return time_speed.buffer; };

        void enable_saltwater_sensor();
        void disable_saltwater_sensor();

        void enable_release();
        void disable_release();
        
        void reset_pressure();

        void set_WiFi_mode();
        void set_Bluetooth_mode();
        void turn_off_comms();

        void begin_sampling() { is_sampling = true; }
        void end_sampling() { is_sampling = false; }

    private:
        HardwareSerial* hard_serial;

        TimePressureTemperature pressure_temp_vals;
        TimeSpeed time_speed;
        ByteArray<int32_t> saltwater_val;

        elapsedMillis pressure_timer;
        elapsedMillis speed_timer;

        bool is_sampling;
};