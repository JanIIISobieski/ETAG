#include "SerialCommunicator.h"

SerialCommunicator::SerialCommunicator(Stream** comms, size_t len) {
    this->comms = comms;
    this->len = 2;
}

int SerialCommunicator::read_and_passthrough_until_command() {
    size_t bytes_read;
    size_t phrase_bytes_read;
    size_t bytes_left;

    for (size_t i = 0; i < len; i++) {
        special_command = -1; //zero out the special command for the channel
        while (comms[i]->available()) {
            bytes_read = comms[i]->readBytesUntil('$', buffer, MAX_BLUETOOTH_PACKET_SIZE); // read until we might have hit the first $ in "$ESP32$"
             // forward this data to the next serial monitor, assume len = 2 for now. if i=0, 1-i=1, and if i=1, then 1-1=0. This way we can toggle between the two SerialCommunicators
            comms[1-i]->write(buffer, bytes_read);
            //now check if the next bytes after what we just read match "ESP32$"
            if (bytes_read < MAX_BLUETOOTH_PACKET_SIZE) { // we still have bytes in the serial, read them
                bytes_left = comms[i]->available();
                if (bytes_left > PHRASE_SIZE) {
                    size_t phrase_bytes_read = comms[i]->readBytesUntil('$', additional_buffer, PHRASE_SIZE + 1);
                    if ((additional_buffer[PHRASE_SIZE + 1] == 0) & array_comparison(phrase, additional_buffer, PHRASE_SIZE)) { //does this match the phrase and the last element is 0
                        special_command = comms[i]->read(); //we have the command
                        reset_buffer(additional_buffer, PHRASE_SIZE+1);
                        return special_command;  //we got a command, return it
                    } else { // this wasn't a command, we need to forward the data, including the '$' and what followed after
                        comms[1-i]->write('$');
                        comms[1-i]->write(additional_buffer, PHRASE_SIZE+1);
                    }
                }       
            }
        } 
    }
    return -1;  //none of the communicators found a $ESP32$, so we return -1 and return to the main program
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

bool SerialCommunicator::array_comparison(uint8_t* array1, uint8_t* array2, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (array1[i] != array2[i]) {
            return false;
        }
    }
    return true;
}

inline void SerialCommunicator::reset_buffer(uint8_t* array, size_t len) {
    memset(array, 0, len);
}