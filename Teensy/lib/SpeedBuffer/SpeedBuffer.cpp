#include "SpeedBuffer.h"

void SpeedBuffer::write(volatile uint8_t* source, size_t length) {
    for (size_t i = 0; i < length; i++) {
        *(buffers[buffer_selector].data_ptr + current_index) = *(source + i);
        increment_and_check_push();
    }
}

void SpeedBuffer::increment_and_check_push() {
    if (++current_index == (buffers[buffer_selector].data_length)) {
        *(buffers[buffer_selector].time_ptr) = micros();
        *(buffers[buffer_selector].count_ptr) = buffer_trigger_count++;
        _queue->enqueue(buffers[buffer_selector].id_ptr);  // this operation is nested within __disable_irq(), enable_irq()
        current_index = 0;
        buffer_selector = (++buffer_selector == num_buffers) ? 0 : buffer_selector;  //change the buffer_selector index, incrementing by 1 or resetting it to 0 once the max length is reached
#ifdef ETAG_DEBUG
    logger.update_timing_data(speed_buffer_push);
#endif
    }
}

void SpeedBuffer::reset_buffer(volatile BufferHelper<uint8_t>* buff) {
    *(buff->count_ptr) = 0;
    *(buff->time_ptr) = 0;
    for (size_t i = 0; i < buff->data_length; i++) {
        *(buff->data_ptr + i) = 0;
    } 
}

void SpeedBuffer::reset() {
    for (size_t i = 0; i < num_buffers; i++) {
        reset_buffer(&buffers[i]);
    }

    buffer_selector = 0;
    current_index = 0;
    buffer_trigger_count = 0;
}