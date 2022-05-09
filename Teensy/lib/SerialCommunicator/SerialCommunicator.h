#pragma once

#include <Arduino.h>
#include <Tag_SD.h>
#include "Tag_Info.h"
#include "DiskManager.h"

#include "Logger.h"
extern Logger logger;

/**
 *  @brief Byte array union to access data by byte or as the data type.
 *  
 *  This enables writing to the union byte by byte, and then reading as the wanted variable type. 
 */
template <typename T> union byte_array {
    byte as_bytes[sizeof(T)]; /** Byte array for storing read data */
    T as_type;                /** Read the full byte array as a particular type */
};

template <typename T> using ByteArray = byte_array<T>;  /** Alias template for the #byte_array union to enable its use as a data type */


/**
 * @brief This abstract class establishes the interface for serial communication
 * 
 * Note that this class cannot inherit from Stream, as Stream does not itself implement
 * the code required for sending data over USB or Serial. Thus, this function instead
 * delegates the write and print functions to the write and print functions that are
 * written in the implementation of Stream class (like usb_serial_class or HardwareSerial).
 * 
 * Tag_USB and Tag_Bluetooth inherit from TagCommunicator for their functions. \ref file_send
 * is the function that has to be implemented in any dervied class as it is a pure virtual function.
 * This interface thus allows for there to be one communications manager that can query all the
 * available communication devices.
 * 
 * \todo This and VarPrinter are redundant, should just combine both of these into one file, or perahps
 * two files, one that handles printing of variables, and the TagCommunicators additionally need methods
 * for sending files, awaiting acknowledgment, 
 */
class SerialCommunicator {
    public:
        /**
         * @brief Construct a new Serial Communicator object
         * 
         * @param serial pointer to a Stream
         */
        SerialCommunicator(Stream* serial, DiskManager* diskManagerPtr) : _serial(serial), _diskManagerPtr(diskManagerPtr) {};

        /**
         * @brief Destroy the Serial Communicator object
         * 
         */
        ~SerialCommunicator();

        /**
         * @brief reads a byte from the stream
         * 
         * @return int -1 if no byte read, otherwise the byte read will be int & 0xFF
         */
        int read();

        /**
         * @brief Write a byte to the stream
         * 
         * @param val value to write
         * @return size_t number of bytes written
         */
        size_t write(uint8_t val);

        /**
         * @brief Write an array to the stream
         * 
         * @param buffer head of the array
         * @param size number of bytes to write
         * @return size_t number of bytes written
         */
        size_t write(uint8_t* buffer, size_t size);

        /**
         * @brief Write an array to the stream
         * 
         * Note, this is the SIZE of the array, not length.
         * An array of length 6 of uint16_t has a size of 12!
         * 
         * @param buffer head of the array
         * @param size number of bytes to write
         * @return size_t number of bytes written
         */
        size_t write(uint16_t* buffer, size_t size);

        /**
         * @brief Send constant buffers to be written
         * 
         * @param buffer head of the array
         * @param size number of bytes to write
         * @return size_t number of bytes written
         */
        size_t write(const uint8_t buffer[], size_t size);

        /**
         * @brief Read an array from the stream
         * 
         * @param buffer Head of the array where the data should be written to
         * @param size Number of bytes to read from the stream
         * @return size_t Number of bytes read
         */
        size_t read(uint8_t* buffer, size_t size);

        /**
         * @brief Print string to the stream, followed by a newline
         * 
         * @param string String to print
         * @return size_t Number of bytes written
         */
        size_t println(String string);

        /**
         * @brief Print a character using the given base to the stream
         * 
         * @param n character to write
         * @param base base of the character to print
         * @return size_t Number of byes written
         */
        size_t println(unsigned char n, int base);

        /**
         * @brief Print string to the stream, with no newline
         * 
         * @param string String to print
         * @return size_t Number of bytes written
         */
        size_t print(String string);

        /**
         * @brief Read from the stream until a character is reached
         * 
         * For example, the stream `abcdef|ghijk>` with readStringUntil('|', 12) will yield `abcdef`.
         * 
         * @param terminator The character until which the stream will be read
         * @param max The maximum number of bytes to read before reaching the terminator
         * @return String The string of characters read from the stream up to (but not including) the terminator
         */
        String readStringUntil(char terminator, size_t max);

        /**
         * @brief Send a file over the stream
         * 
         * @param file_name Name of file to send
         * @return true File was successfully sent
         * @return false File was not successfully sent
         */
        virtual bool file_send(String file_name) = 0;

        /**
         * @brief Wait for acknowledgement from the Stream, blocking read
         * 
         * Typically waiting for the ASCII ACK (value of 6)
         * 
         * @return true Acknowledgment recieved
         * @return false Acknowledgement not recieved
         */
        bool await_acknowledgment();

        /**
         * @brief Initalize the Stream with a baud rate
         * 
         * Note that for usb_serial_class, the baud rate makes no difference, full USB speed will be used
         * 
         * @param baud_rate Set the baud rate
         */
        virtual void init(uint32_t baud_rate) = 0;

        /**
         *  @brief Reads bytes from the last read SerialCommunicator as a standard data type.
         * 
         *  This function enforces waiting for the bytes to go through. Will not return unless the bytes are read.
         *  As this is a template function, it can take any standard data type.
         * 
         *  @param[out] array ByteArray of any standard type.
         *  @returns size_t Number of bytes read
         */
        template <typename T>
        size_t read_type(ByteArray<T>& array);
        
    protected:
        Stream* _serial;    /**< pointer to a Stream to which characters will be sent from, read */
        int read_byte;      /**< pre-allocate the storage of the read-byte */
        DiskManager* _diskManagerPtr;
};

template <typename T>
size_t SerialCommunicator::read_type(ByteArray<T>& array) {
    return read(reinterpret_cast<uint8_t*>(&array), sizeof(array));
}
