#include "BufferBase.h"

BufferBase::BufferBase(size_t num_buffers, size_t length_buffers, uint8_t id) {
    this->buffer_selector = 0;
    this->buffer_trigger_count = 0;
    this->current_index = 0;
    this->num_buffers = num_buffers;

    identifier = id;
}