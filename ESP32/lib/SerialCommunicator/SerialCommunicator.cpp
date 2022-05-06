#include "SerialCommunicator.h"

SerialCommunicator::SerialCommunicator(Stream** comms, size_t len) {
    this->comms = comms;
    this->len = len;
}

size_t SerialCommunicator::buffered_read() {
    size_t bytes_read;
    size_t phrase_bytes_read;
    size_t bytes_left;

    for (size_t i = 0; i < len; i++) {
        if (comms[i]->available()) {
            bytes_read = comms[i]->readBytesUntil('$', buffer, MAX_BLUETOOTH_PACKET_SIZE);

            if (bytes_read < MAX_BLUETOOTH_PACKET_SIZE) {
                bytes_left = comms[i]->available();

                if (bytes_left > PHRASE_SIZE) {
                    size_t phrase_bytes_read = comms[i]->readBytesUntil('$', additional_buffer + buffer_offset, PHRASE_SIZE + 1);

                    if ((additional_buffer[PHRASE_SIZE + 1] == 0) & array_comparison(phrase, additional_buffer, PHRASE_SIZE)) {
                        special_command = comms[i]->read();
                    }
                }
            } 
        }
    }  
}

bool SerialCommunicator::array_comparison(uint8_t* array1, uint8_t* array2, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (array1[i] != array2[i]) {
            return false;
        }
    }
    return true;
}