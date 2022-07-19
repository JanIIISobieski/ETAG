#include "Writer.h"

StaticJsonDocument<512> Writer::header_info;
StaticJsonDocument<256> Writer::json_imu_calibration;

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