#pragma once

#include "Logger.h"
extern TimingData speed_buffer_push;
extern Logger logger;

#include <Arduino.h>
#include "Tag_Queue.h"
#include "AbstractBuffer.h"

class SpeedBuffer : public AbstractBuffer<uint8_t> {
    public:
        SpeedBuffer(volatile uint8_t** buffers, size_t num_buffers, size_t buffer_length, uint8_t id, Tag_Queue* queue) :
            AbstractBuffer(buffers, num_buffers, buffer_length, id) {
                _queue = queue;
            };

        void write(volatile uint8_t* source, size_t length);
        void reset();
        uint8_t get_buffer_selector() { return buffer_selector; }
        uint16_t get_index() { return current_index; }
        uint8_t get_trigger_count() { return buffer_trigger_count; }

    private:
        Tag_Queue* _queue;
        void reset_buffer(volatile BufferHelper<uint8_t>* buff);
        void increment_and_check_push();
};