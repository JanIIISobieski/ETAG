#include "EEGBuffer.h"

extern EEGBuffer eeg_buffer;

//This function is called when the DMA SPI transfer is finished -> the same as spi_end
void dma_spi_finished(EventResponderRef event) {
    delayMicroseconds(1);    // This delay is necessary on the Teensy 4.0 in order to assure communication finished
    digitalWriteFast(8, HIGH); // End communication (EEG_CS, HIGH)
    SPI.endTransaction();
    eeg_buffer.write();
}

void dma_spi_finished_streaming(EventResponderRef event) {
    delayMicroseconds(1);
    eeg_buffer.set_time();
    digitalWriteFast(8, HIGH);
    SPI.endTransaction();
    eeg_buffer.set_new_data(true);
}

// ADS1299 data ready interrupt service routine
void ads_ISR() {
    SPI.beginTransaction(SPISettings(EEG_DMA_SPI_SPEED, MSBFIRST, SPI_MODE1));
    digitalWriteFast(8, LOW);  // Start communication (EEG_CS pin, LOW)
    SPI.transfer(nullptr, (void *)eeg_buffer.get_eeg_pointer(), 27, eeg_buffer.dma_spi_transfer);
}


void EEGBuffer::init() {
    dma_spi_transfer.attach(dma_spi_finished);
    SPI.usingInterrupt(digitalPinToInterrupt(14));  //EEG_DRDY pin: 14
    attachInterrupt(digitalPinToInterrupt(14), ads_ISR, FALLING);
    new_data = false;
    logger.print_message("Initalized EEG Buffer");
}

void EEGBuffer::write() {
    for (size_t i = 0; i < 27; i++) {
        *(buffers[buffer_selector].data_ptr + current_index) = *(get_eeg_pointer() + i);
        increment_and_check_push();
    }
}

void EEGBuffer::write(volatile uint8_t* source, size_t length) {
    for (size_t i = 0; i < length; i++) {
        logger.print_variable("CI", current_index);
        *(buffers[buffer_selector].data_ptr + current_index) = *(source + i);
        increment_and_check_push();
    }
}

inline void EEGBuffer::increment_and_check_push() {
    if (++current_index == (buffers[buffer_selector].data_length)) {
        *(buffers[buffer_selector].time_ptr) = micros();
        *(buffers[buffer_selector].count_ptr) = buffer_trigger_count++;
        _queue->enqueue(buffers[buffer_selector].id_ptr);  // this operation is nested within __disable_irq(), enable_irq()
        current_index = 0;
        buffer_selector = (++buffer_selector == num_buffers) ? 0 : buffer_selector;  //change the buffer_selector index, incrementing by 1 or resetting it to 0 once the max length is reached

#ifdef ETAG_DEBUG
    logger.update_timing_data(eeg_buffer_push);
#endif
    }
}

void EEGBuffer::reset_buffer(volatile BufferHelper<uint8_t>* buff) {
    *(buff->count_ptr) = 0;
    *(buff->time_ptr) = 0;
    for (size_t i = 0; i < (buff->data_length); i++) {
        *(buff->data_ptr + i) = 0;
    }
}

void EEGBuffer::reset() {
    dma_spi_transfer.detach();
    for (size_t i = 0; i < num_buffers; i++) {
        reset_buffer(&buffers[i]);
    }
    
    buffer_selector = 0;
    buffer_trigger_count = 0;
    current_index = 0;
}