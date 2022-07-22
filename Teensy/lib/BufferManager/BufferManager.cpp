#include "BufferManager.h"

void BufferManager::reset() {
    for (size_t i = 0; i < num_buffers; i++) {
        buffers[i]->reset();
    }
}