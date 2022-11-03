#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Tag_Info.h>
#include <Logger.h>

/**
 * @brief 
 * 
 */
class Writer {
    protected:
        static StaticJsonDocument<512> header_info; /**< JSON containing run metadata like animal species, animal name, and a description */
        static StaticJsonDocument<256> json_imu_calibration; /**< JSON containing the IMU calibration values (acceleration bias, gyro bias, scaling factors, etc.) */

        /**
         * @brief This results in actually deleting the information from the JSONs
         * 
         * Note that due to the structure of these JSONs and how they are implemented in <a href="https://github.com/bblanchon/ArduinoJson">ArduinoJSON</a>, 
         * the JSONs must be cleared in between writing new values to the keys. Otherwise, there will be a memory leak.
         * Since the JSONs are only really created before the header has to be written, they need to be cleared after writing.
         * Thus, for the most part, clera_jsons() is run in the \ref Writer::post_sampling_conclude(), in addition
         * to any other tasks that might have to be run.
         */
        void clear_jsons() { json_imu_calibration.clear(); header_info.clear(); }

        /**
         * @brief Create a data header JSON
         * 
         * The \ref header_info and \ref json_imu_calibration headers are created from scratch
         * with the appropriate settings. Note that these JSONs are both static, and are thus
         * shared between all classes that inherit from \ref Writer. This results in saving some memory
         * to not have several JSONs taking up space in memory.
         * 
         * @return size_t The size of the created JSONs
         */
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
        virtual size_t write_header() = 0;  /**< Pure virtual function to write the header */
        virtual size_t write_data(void* buff_ptr, size_t num_bytes) = 0; /**< Pure virtual function to write a data buffer */
        virtual void   pre_sampling_setup() = 0; /**< Pure virtual function to run any setup before sampling can be started */
        virtual void   post_sampling_conclude() = 0; /**< Pure virtual function to run after sampling to properly conclude sampling */

        size_t  get_json_size() { return measureJson(header_info); };

        /**
         * @brief Construct a new Writer object
         * 
         * @param run_data A \ref RunData structure to refer to for the data that will be written in the data header.
         */
        Writer(RunData* run_data) : run_data(run_data) {};
};

template <typename T>
void Writer::write_json_array(JsonArray* array, T* src, size_t len) {
    for (size_t i = 0; i < len; i++) {
        array->add(*(src + i));
    }   
}