#pragma once

#include <Arduino.h>
#include "CompQueue.h"

/**
 * @brief Class to act as an interface for the Serial communication between the boards
 * 
 * This class is to simplify the communication scheme and abstract away the inherent complexity in
 * either passing the data from the Bluetooth to the Teensy or having the ESP32 chip react to incoming bytes.
 * 
 * This class essentially sets up two different modes of communication:
 * 1. PASSTHROUGH
 *      In this mode, any incoming byte through the Bluetooth is passed directly to the Teensy. The ESP32 does not react to the bytes at all,
 *      other than simplify forwarding them. The one exception is that $ESP32$ followed by a byte will cause the ESP32 to perform the command that follows after the
 *      data header. That is, $ESP32$[command], where [command] is a one-byte character, will cause the ESP32 to run that particular
 *      command.
 * 
 *      These command bytes can be as follows:
 *        * 'o': disables all comunication. Note then that the Teensy is the only one that can turn on wireless communication. This enables command mode as well.
 *        * 'w': enables WiFi communication (to be set before uploading new firmware).
 *        * 'y': enable bluetooth communication. This is off by defualt. Teensy has to set this command.
 *        * '-': disable passthrough mode and enable command mode  
 * 
 * 2. COMMAND
 *      In this mode, set by $ESP32$-, the ESP32 chip will only respond to valid, one-byte commands from the Teensy over the Serial port.
 *      This is the default mode at startup.
 *      
 *      The commands that can be sent through this mode are as follows:
 *        * '+': enable passthrough mode
 *        * 'a': enable the release
 *        * 'e': disable the release
 *        * 'k': enable the saltwater sensor and read the value
 *        * 'l': disable the saltwater sensor
 *        * 'p': get the pressure and temperature readings
 *        * 'r': reset the pressure sensor
 *        * 'v': read the speed sensor
 *        * 'o': disable all communication
 *        * 'w': set WiFi communication
 *        * 'y': set BlueTooth communication mode
 * 
 * The header $ESP32$ was chosen as it is a 7-byte command. The probability that 7 random bytes in a row will form $EPS32$ is 1/(256^7).
 * Put another way, writing randomly a 4 GB (max file size in FAT32) file, there is about a 1 in 16 million chance that it will contain
 * $ESP32$.
 * 
 * The way this class achieves this behavior is by having an internal 7-byte buffer. If an incoming byte matches $, it is saved in the buffer.
 * As each additional byte matches the next character of the $ESP32$ sequence, it is added to the buffer. If a byte comes in before the length of the buffer
 * 
 * This function works right now only if len = 2. More than 2 and the passthrough becomes more complicated.
 */
class SerialCommunicator {
    public:
        SerialCommunicator(Stream** comms, size_t len);
        ~SerialCommunicator();

        int read_and_passthrough_until_command();

        int read();

        template <typename T>
        size_t write(T val);

        size_t write(uint8_t* array, size_t len);

    private:
        Stream** comms;  /**<Pointer to a vector of Stream* */
        size_t len;

        int special_command = -1;
        uint8_t ID;

        CompQueue queue[2];
        ReturnData data[2];
};

template <typename T>
size_t SerialCommunicator::write(T val) {
    return comms[ID]->write(val);
}