#include "TagCommunicator.h"

TagCommunicator::TagCommunicator(SerialCommunicator** ptr, size_t len) {
    ID = 0;
    length = len;
    communicator_ptr = ptr;
}

int TagCommunicator::check_for_commands() {
    int return_int = -1;    // -1 by default (i.e. no read)
    for (size_t i = 0; i < length; i++) {
        int read_val = (*(communicator_ptr + i))->read();
        if (read_val == -1) {
            continue;
        }
        else {
            this->ID = i;  //if something was read, get the ID (index) and the value. Note, this-> is redundant, using it for readability and underline that ID is a class member
            return_int = read_val;
        }
    }
    return return_int;  //return the read value, -1 if nothing read
}

bool TagCommunicator::send_file(String filename) {
    return (*(communicator_ptr + ID))->file_send(filename.c_str());
}

size_t TagCommunicator::write(uint8_t port, uint8_t byte) {
    return (*(communicator_ptr + port))->write(byte);
}

size_t TagCommunicator::write(uint8_t port, uint8_t* buffer, size_t len) {
    return (*(communicator_ptr + port))->write(buffer, len);
}

size_t TagCommunicator::write(uint8_t byte) {
    return write(ID, byte);  // send the byte over the line from which the command came
}

size_t TagCommunicator::write(uint8_t* buffer, size_t len) {
    return write(ID, buffer, len);  // send the byte over the line from which the command came
}

bool TagCommunicator::await_acknowledgment() {
    return (*(communicator_ptr + ID))->await_acknowledgment();  // run the function for the corresponding SerialCommunicator
}

size_t TagCommunicator::print(String string) {
    return (*(communicator_ptr + ID))->print(string);  // run the function for the corresponding SerialCommunicator
}

size_t TagCommunicator::println(String string) {
    return (*(communicator_ptr + ID))->println(string);  // run the function for the corresponding SerialCommunicator
}

size_t TagCommunicator::println(unsigned char n, int base) {
    return (*(communicator_ptr + ID))->println(n, base);  // run the function for the corresponding SerialCommunicator
}

size_t TagCommunicator::read_settings(SettingsPacket& settings) {
    return (*(communicator_ptr + ID))->read(reinterpret_cast<uint8_t*>(&settings), sizeof(settings));
}

String TagCommunicator::readStringUntil(char terminator, size_t max) {
    return (*(communicator_ptr + ID))->readStringUntil(terminator, max);
}