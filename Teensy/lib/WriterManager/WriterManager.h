#pragma once

#include "Writer.h"

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
        size_t len /**< The number of Writers */;
        size_t writer_ind /**< The index of the writing array that is currently being used */;
    
    public:
        /**
         * @brief Construct a new Writer Manager object
         * 
         * By default, writer_ind will be initialized to 2 (corresponding to the SD card index)
         * 
         * @param writers An array of pointers to \ref Writer objects
         * @param num_writers The length of the writers array
         */
        WriterManager(Writer** writers, size_t num_writers) : writer_ptrs(writers), len(num_writers) { writer_ind = 2; }
        ~WriterManager() {};

        /**
         * @brief Updates the writer_ind to use
         * 
         * @param index The new index to use for calling the functions
         */
        void select_writer(size_t index) {
            writer_ind = index;
            logger.print_variable("Writer Ind", index);
        }

        /**
         * @brief Writes the data buffer using the Writer corresponding to the writer_ind
         * 
         * @param buff_ptr The address of the buffer to be written
         * @param num_bytes The number of bytes to write from the buffer
         * @return size_t The number of bytes actually written
         */
        size_t write_data(void* buff_ptr, size_t num_bytes) {
            logger.print_variable("Writing Data", writer_ind);
            return writer_ptrs[this->writer_ind]->write_data(buff_ptr, num_bytes);
        }

        /**
         * @brief Writes the data header using the Writer corresponding to the writer_ind
         * 
         * @return size_t The number of bytes written
         */
        size_t write_header() {
            logger.print_variable("Writing Header", writer_ind);
            return writer_ptrs[this->writer_ind]->write_header();
        }

        /**
         * @brief This routine is run just before sampling, running the pre_sampling_setup of the Writer corresponding to the writer_ind
         */
        void pre_sampling_setup() {
            logger.print_variable("Pre Sampling Setup", writer_ind);
            writer_ptrs[this->writer_ind]->pre_sampling_setup();
        }

        /**
         * @brief This routine is run after sampling is stopped, running the post_sampling_conclude of the Writer corresponding to the writer_ind
         */
        void post_sampling_conclude() {
            logger.print_variable("Post Sampling Conclude", writer_ind);
            writer_ptrs[this->writer_ind]->post_sampling_conclude();
        }
};