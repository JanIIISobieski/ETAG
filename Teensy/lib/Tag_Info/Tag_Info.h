#pragma once

#include <Arduino.h>
#include <TimeLib.h>
#include "MPU9250.h"

#include "Logger.h"
extern Logger logger;

/**
 * @brief Save Animal Data on the tag
 * 
 * This data is sent over Serial when sampling is to begin (see begin_sampling())
 */
struct AnimalData {
    String name;
    String species;
};

/**
 * @brief Structure to store the current state of IMU calibration
 * 
 * \todo Interface this better with MPU9250 or any other IMU class, right now it's all a bit redundant (functional, but not elegant)
 * 
 */
struct IMUCalibration {
    float accel_resolution = {0};
    float accel_biases[3] = {0};
    float gyro_resolution = {0};
    float magnetometer_resolution = {0};  
    float magnetometer_calibration[3] = {0};
    float magnetometer_biases[3] = {0};
};

/**
 * @brief Class to manage the run metadata
 * 
 * Contains update methods to IMUCalibration, AnimalData, and other data
 * 
 */
class RunData {
    public:
        /**
         * @brief Construct a new Run Data object
         * 
         */
        RunData();

        /**
         * @brief Create a datetime String
         * 
         * This String is used as a filename and is in the form:
         * `YYYY-MM_DD_HH-mm-SS.txt`. This function creates that string based on the time given by now().
         * 
         */
        void update_datetime();

        /**
         * @brief Updates the IMUCalibration object with the most recent data from MPU9250
         * 
         * @param imu The IMU to be passed by reference
         */
        void update_imu_calibration(MPU9250& imu);

        /**
         * @brief Update the name of the animal
         * 
         * @param name The name of the animal
         */
        void update_animal_name(String name) { animal.name = name; };

        /**
         * @brief Update the species of the animal
         * 
         * @param species The species of the animal
         */
        void update_animal_species(String species) { animal.species = species; };

        /**
         * @brief Update the description of the run
         * 
         * This should be fewer than 120 characters.
         * 
         * @param desc A short description that is saved as a header for the file with some information
         */
        void update_description(String desc) { description = desc; };

        /**
         * @brief Get the datetime object
         * 
         * @return String Get the datetime string
         */
        String get_datetime() { return datetime; };

        /**
         * @brief Get the animal object
         * 
         * @return const AnimalData* pointer to the animal data structure (read-only)
         */
        const AnimalData* get_animal() { return &animal; };

        /**
         * @brief Get the imu calibration object
         * 
         * @return const IMUCalibration* pointer to the IMU calibration structure (read-only)
         */
        const IMUCalibration* get_imu() { return &imu_calibration; };

        /**
         * @brief Get the description
         * 
         * @return const String the header description for the file
         */
        const String get_description() { return description; };

    private:
        String datetime;                    /**< The datetime String in YYYY-MM-DD_HH-mm_SS */
        String description;                 /**< Description of the file */
        AnimalData animal;                  /**< Animal Data structure (name and species) */
        IMUCalibration imu_calibration;     /**< IMU calibration structure*/
};
