#pragma once

#include "BufferBase.h"
#include "Logger.h"
extern Logger logger;

class BufferManager {
    private:
        BufferBase** buffers;
        size_t num_buffers;
    public:
        BufferManager(BufferBase** buffers, size_t num_buffers) : buffers(buffers), num_buffers(num_buffers) {};

        void reset();
};