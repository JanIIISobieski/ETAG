#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Tag_Info.h>
#include <Logger.h>

class Writer {
    protected:
        StaticJsonDocument<512> header_info;
        StaticJsonDocument<256> json_imu_calibration;

        size_t create_data_header(RunData* run_data);

        template <typename T>
        void write_json_array(JsonArray* array, T* src, size_t len);

    public:
        virtual size_t write_header() = 0;
        virtual size_t write_data() = 0;
};

template <typename T>
void Writer::write_json_array(JsonArray* array, T* src, size_t len) {
    for (size_t i = 0; i < len; i++) {
        array->add(*(src + i));
    }   
}

size_t Writer::create_data_header(RunData* run_data) {
    json_imu_calibration["aRes"] = (run_data->get_imu())->accel_resolution;
    json_imu_calibration["gRes"] = (run_data->get_imu())->gyro_resolution;
    json_imu_calibration["mRes"] = (run_data->get_imu())->magnetometer_resolution;

    JsonArray aBias = json_imu_calibration.createNestedArray("aBias");
    JsonArray mBias = json_imu_calibration.createNestedArray("mBias");
    JsonArray mCal  = json_imu_calibration.createNestedArray("mCal");

    write_json_array(&aBias, (run_data->get_imu())->accel_biases, 3);
    write_json_array(&mBias, (run_data->get_imu())->magnetometer_biases, 3);
    write_json_array(&mCal,  (run_data->get_imu())->magnetometer_calibration, 3);

    header_info["imu_calibration"] = json_imu_calibration;
    header_info["name"] = (run_data->get_animal())->name;
    header_info["species"] = (run_data->get_animal())->species;
    header_info["description"] = (run_data->get_description());

    return measureJson(header_info);
}