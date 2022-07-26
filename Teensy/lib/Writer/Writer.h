#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Tag_Info.h>
#include <Logger.h>

class Writer {
    protected:
        static StaticJsonDocument<512> header_info;
        static StaticJsonDocument<256> json_imu_calibration;

        void clear_jsons() { json_imu_calibration.clear(); header_info.clear(); }
        size_t create_data_header();

        /**
         * @brief Helping function to write a JSON array
         * 
         * Mostly just eliminates the need for many uses of array.add()
         * 
         * @tparam T Arbitrary data type (e.g. uint8_t, int16_t, unsigned char, float, etc.)
         * @param array A JsonArray object (from ArduinoJSON package)
         * @param src The array to write to the JsonArray
         * @param len The length of /ref src to write
         */
        template <typename T>
        void write_json_array(JsonArray* array, T* src, size_t len);

        RunData* run_data;

    public:
        virtual size_t write_header() = 0;
        virtual size_t write_data(void* buff_ptr, size_t num_bytes) = 0;
        virtual void   pre_sampling_setup() = 0;
        virtual void   post_sampling_conclude() = 0;

        Writer(RunData* run_data) : run_data(run_data) {};
};

template <typename T>
void Writer::write_json_array(JsonArray* array, T* src, size_t len) {
    for (size_t i = 0; i < len; i++) {
        array->add(*(src + i));
    }   
}