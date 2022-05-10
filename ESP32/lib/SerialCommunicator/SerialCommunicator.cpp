#include "SerialCommunicator.h"

SerialCommunicator::SerialCommunicator(Stream** comms, size_t len) {
    this->comms = comms;
    this->len = 2;
}

SerialCommunicator::~SerialCommunicator() {
    
}

int SerialCommunicator::read_and_passthrough_until_command() {
    for (size_t i = 0; i < len; i++) {
        if (comms[i]->available()) {
            timeouts[i] = 0;
            data[i] = queue[i].enqueue_and_check(comms[i]->read());

            if (data[i].ready_to_write) {
                comms[1-i]->write(data[i].data_ptr, data[i].len);
                reset(i);
                return -1;
            }
            
            if (data[i].len == PHRASE_LENGTH) {
                while (!comms[i]->available()) { }
                reset(i);
                return comms[i]->read();
            }
        } else { //no comms available
            //we might have a partially filled buffer with data, and no new data seems to be coming in
            //we should flush the buffer if more than timeout has been reached
            if ((timeouts[i] > READ_TIMEOUT) & (data[i].len > 0)) {
                comms[1-i]->write(data[i].data_ptr, data[i].len);
                reset(i);
                timeouts[i] = 0;
            }
        }
    }
    return -1;
}

int SerialCommunicator::read() {
    for (size_t i = 0; i < len; i++) {
        if (comms[i]->available()) {
            ID = i;
            return comms[i]->read();
        }
    }
    return -1;
}

size_t SerialCommunicator::write(uint8_t* array, size_t len) {
    return comms[ID]->write(array, len);
}