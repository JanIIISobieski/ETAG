#pragma once

#include "Writer.h"
#include "SerialCommunicator.h"

#include "Logger.h"
extern Logger logger;

/**
 * @brief Class to manage the classes that inherit from \ref Writer.
 * 
 * This class forwards the calls for writing data, writing headers, pre and post sampling routines,
 * to the appropriate \ref Writer.
 * 
 * An index of 0 corresponds to the bluetooth, and index of 1 corresponds to the USB, while an index of 2 corresponds to the SD card
 * (through \ref DiskManager).
 * 
 */
class WriterManager {
    private:
        Writer** writer_ptrs; /**< An array of pointers to Writers */
        size_t len; /**< The number of Writers */

    public:
        static ByteArray<uint8_t> writer_ind; /**< The index of the writing array that is currently being used */;      

        /**
         * @brief Construct a new Writer Manager object
         * 
         * By default, writer_ind will be initialized to 2 (corresponding to the SD card index)
         * 
         * @param writers An array of pointers to \ref Writer objects
         * @param num_writers The length of the writers array
         */
        WriterManager(Writer** writers, size_t num_writers) : writer_ptrs(writers), len(num_writers) { }
        ~WriterManager() {};

        /**
         * @brief Updates the writer_ind to use
         * 
         * @param index The new index to use for calling the functions
         */
        void select_writer(size_t index);

        /**
         * @brief Get the writer ind object
         * 
         * 0 is Bluetooth, 1 is USB, 2 is SD card
         * 
         * @return size_t The current writer in use
         */
        size_t get_writer_ind();

        /**
         * @brief Writes the data buffer using the Writer corresponding to the writer_ind
         * 
         * @param buff_ptr The address of the buffer to be written
         * @param num_bytes The number of bytes to write from the buffer
         * @return size_t The number of bytes actually written
         */
        size_t write_data(void* buff_ptr, size_t num_bytes);

        /**
         * @brief Writes the data header using the Writer corresponding to the writer_ind
         * 
         * @return size_t The number of bytes written
         */
        size_t write_header();

        /**
         * @brief This routine is run just before sampling, running the pre_sampling_setup of the Writer corresponding to the writer_ind
         */
        void pre_sampling_setup();

        /**
         * @brief This routine is run after sampling is stopped, running the post_sampling_conclude of the Writer corresponding to the writer_ind
         */
        void post_sampling_conclude();
};
