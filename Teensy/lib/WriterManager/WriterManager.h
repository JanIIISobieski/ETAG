#pragma once

#include "Writer.h"

#include "Logger.h"
extern Logger logger;

class WriterManager {
    private:
        Writer** writer_ptrs;
        size_t len;
        size_t writer_ind;
    
    public:
        WriterManager(Writer** writers, size_t num_writers) : writer_ptrs(writers), len(num_writers) { writer_ind = 2; }
        ~WriterManager() {};

        void select_writer(size_t index) {
            writer_ind = index;
            logger.print_variable("Writer Ind", index);
        }

        size_t write_data(void* buff_ptr, size_t num_bytes) {
            logger.print_message("Writing Data");
            return writer_ptrs[this->writer_ind]->write_data(buff_ptr, num_bytes);
        }

        size_t write_data(size_t ind, void* buff_ptr, size_t num_bytes) {
            return writer_ptrs[ind]->write_data(buff_ptr, num_bytes);
        }

        size_t write_header() {
            return writer_ptrs[this->writer_ind]->write_header();
        }

        size_t write_header(size_t ind) {
            logger.print_message("Writing Header");
            return writer_ptrs[ind]->write_header();
        }

        void pre_sampling_setup() {
            logger.print_message("Pre sampling setup");
            writer_ptrs[this->writer_ind]->pre_sampling_setup();
        }

        void pre_sampling_setup(size_t ind) {
            writer_ptrs[ind]->pre_sampling_setup(); 
        }

        void post_sampling_conclude() {
            logger.print_message("Post sampling conclusion");
            writer_ptrs[this->writer_ind]->post_sampling_conclude();
        }

        void post_sampling_conclude(size_t ind) {
            writer_ptrs[ind]->post_sampling_conclude();
        }
};