#include "Tag_Info.h"

RunData::RunData() {
    animal.name = "";
    animal.species = "";
    imu_calibration = {};
    datetime = "";
}

void RunData::update_datetime() {
    time_t current_time = now();
    String datestring = String(year(current_time)) + "-" + String(month(current_time)) + "-" + String(day(current_time));
    String timestring = "_";
    int time[3] = {hour(current_time), minute(current_time), second(current_time)};
    for (size_t i = 0; i < 3; i++)
    {
        if (i > 0) {
            timestring = timestring + "-";  //seperate by hyphens
        }
        if (time[i] < 10) {
            timestring = timestring + "0";  //if single digit time, prepend a zero
        }
        timestring = timestring + time[i];
    }
    this->datetime = datestring + timestring;

    logger.print_variable("Datetime", datetime);
};

void RunData::update_imu_calibration(MPU9250& imu) {
    imu_calibration.accel_resolution = imu.aRes;
    memcpy(imu_calibration.accel_biases, imu.accelBias, 12);  // copy 3 floats, which are 4 bytes each, and so 12 bytes

    imu_calibration.gyro_resolution = imu.gRes;
    
    imu_calibration.magnetometer_resolution = imu.mRes;
    memcpy(imu_calibration.magnetometer_calibration, imu.factoryMagCalibration, 12);
    memcpy(imu_calibration.magnetometer_biases, imu.factoryMagBias, 12);

    logger.print_message("-----IMU Calibration--------------------");
    logger.print_variable("Accel Res", imu_calibration.accel_resolution);
    logger.print_variable("Gyro  Res", imu_calibration.gyro_resolution);
    logger.print_variable("Mag   Res", imu_calibration.magnetometer_resolution);

    logger.print_array("Accel Bias", imu_calibration.accel_biases, 3);
    logger.print_array("Mag    Cal", imu_calibration.magnetometer_calibration, 3);
    logger.print_array("Mag   Bias", imu_calibration.magnetometer_biases, 3);
};