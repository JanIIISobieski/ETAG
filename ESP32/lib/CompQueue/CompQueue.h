#pragma once

#include <Arduino.h>

#define PHRASE_LENGTH 7

struct ReturnData {
    uint8_t* data_ptr;
    size_t len;
    bool ready_to_write;
};


class CompQueue {
    public:
        CompQueue();
        ~CompQueue();

        const ReturnData enqueue_and_check(uint8_t val);
        void reset_buffer() { memset(buffer, 0, ind); ind = 0; };

    private:
        uint8_t phrase[PHRASE_LENGTH] = {'$', 'E', 'S', 'P', '3', '2', '$'};
        uint8_t buffer[PHRASE_LENGTH] = {0};

        size_t ind;
        ReturnData data;
};