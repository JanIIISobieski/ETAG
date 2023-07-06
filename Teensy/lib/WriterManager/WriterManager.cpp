#include "WriterManager.h"

ByteArray<uint8_t> WriterManager::writer_ind = { .as_type = 2 };  // initializer declaration of the WriterManager default write index.
                                                                  // this now makes the writer_ind a part of Writer Manager, while allowing
                                                                  // for its use outside of the class as well, namely to put data into during
                                                                  // update_params by \TagComms

void WriterManager::select_writer(size_t index) {
    writer_ind.as_type = index;
    logger.print_variable("Writer Ind", index);
}

size_t WriterManager::get_writer_ind() { 
    return writer_ind.as_type;
};

size_t WriterManager::write_data(void* buff_ptr, size_t num_bytes) {
    logger.print_variable("Writing Data", writer_ind.as_type);
    return writer_ptrs[this->writer_ind.as_type]->write_data(buff_ptr, num_bytes);
}

size_t WriterManager::write_header() {
    logger.print_variable("Writing Header", writer_ind.as_type);
    return writer_ptrs[this->writer_ind.as_type]->write_header();
}

void WriterManager::pre_sampling_setup() {
    logger.print_variable("Pre Sampling Setup", writer_ind.as_type);
    writer_ptrs[this->writer_ind.as_type]->pre_sampling_setup();
}

void WriterManager::post_sampling_conclude() {
    logger.print_variable("Post Sampling Conclude", writer_ind.as_type);
    writer_ptrs[this->writer_ind.as_type]->post_sampling_conclude();
}