#include "CompQueue.h"

CompQueue::CompQueue() {
    data.data_ptr = buffer;
    data.len = 0;

    ind = 0;
}

CompQueue::~CompQueue() {

}

const ReturnData CompQueue::enqueue_and_check(uint8_t val) {
    buffer[ind] = val;
    data.ready_to_write = (buffer[ind] != phrase[ind]);
    data.len = ++ind;

    return data;
}