#include "BufferManager.h"

void BufferManager::reset() {
    logger.print_message("Resetting Buffers");
    for (size_t i = 0; i < num_buffers; i++) {
        logger.print_variable("Buffer Ind", i);
        buffers[i]->reset();
    }
}