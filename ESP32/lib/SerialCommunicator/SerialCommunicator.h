#pragma once

#include <Arduino.h>

#define COMMAND_BUFFER 7

class SerialCommunicator {
    public:
        SerialCommunicator();
        ~SerialCommunicator();

        size_t buffered_read();
        size_t buffered_write();

    private:
        char buffer[COMMAND_BUFFER];
        char phrase[COMMAND_BUFFER];
        uint8_t ind = 0;

        Stream** comms;
};