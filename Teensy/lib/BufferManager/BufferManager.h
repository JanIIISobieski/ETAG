#pragma once

#include "BufferBase.h"

class BufferManager {
    private:
        BufferBase** buffers;
        size_t num_buffers;
    public:
        BufferManager(BufferBase** buffers, size_t num_buffers) : buffers(buffers), num_buffers(num_buffers) {};

        void reset();
};