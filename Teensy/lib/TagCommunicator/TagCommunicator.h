#ifndef GUARD_TAG_COMMUNICATOR
#define GUARD_TAG_COMMUNICATOR

#include "SerialCommunicator.h"
#include "TagSettings.h"
#include "Tag_Info.h"

/**
 *  @brief Class for managing communication over USB and Bluetooth
 * 
 *  Rather than writing seperate code in main(), this class manages reading data
 *  from the available sources, and writing data using the appropriate method.
 *  This method can handle arbitrary number of communicators, as long as each
 *  inherits from the #SerialCommunicator class. Important assumption: when calling
 *  a function that does not sample (i.e. syncing time or dowloading files), this
 *  class assumes that the data should be sent back over the same communicator that
 *  sent the command (i.e. if Bluetooth sent the command 'c' for time syncing,
 *  Bluetooth will be used for acknowledgment of the command and for recieving more
 *  data from the computer). Sampling can be stopped using either method, no matter 
 *  which method was used to start sampling. File download is recommended to only
 *  use with the USB. Ports can also be passed to the write functions to send data
 *  over a specified line.
 */
class TagCommunicator {
	private:
		size_t ID;
        size_t length;
		SerialCommunicator** communicator_ptr;

	public:
        /**
         * @brief Class Initializer
         * 
         * @param[in] ptr a pointer to an array of SerialCommunicator*
         * @param[in] baud_rates a pointer to an array of baud_rates, with indices corresponding to that of #ptr
         * @param[in] len the length of both arrays (must be equal)
         * @param[in] run_data* pointer to the RunData structure
         */
		TagCommunicator(SerialCommunicator** ptr, size_t len);
        
        /**
         * @brief Checks for available commands from each managed SerialCommunicator.
         * 
         * This function specifies which SerialCommunicator to use for any further communication (until function returns)
         * 
         * @returns int Integer read from the SerialCommunicators. If nothing read, returns -1. Else, the byte read is the last byte of the int: i.e. the result of int & 0xFF 
         */
        int check_for_commands();
        
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
		
        /**
         * @brief Read a certain number of bytes into a buffer
         * 
         * @param buffer The pointer to the start of the buffer
         * @param len The number of bytes to read
         * @return size_t The number of bytes read
         */
        size_t read(uint8_t* buffer, size_t len);

        /** @brief Read data for the device settings over a SerialCommunicator
         * 
         * This function, like #read_type(), enforces waiting for the bytes. Will not return unless the bytes are read.
         * @param[out] settings SettingsPacket to which the settings will be written
         * 
         * @returns size_t the number of bytes counted
         */
        size_t read_settings(SettingsPacket& settings);

        /**
         *  @brief Function to send a specfic file over one of the Serial Communicators
         *  
         *  @param[in] filename The name of the file to send
         *  @returns bool If files written successfully, returns true, otherwise returns false
         */
        bool send_file(String filename);

        /**
         * @brief Write a byte to the specified port
         * 
         * @param port 0 for Bluetooth, 1 for USB, see \ref Ports in \ref initalize_variables.h
         * @param byte Byte to write
         * @return size_t The number of bytes written
         */
        size_t write(uint8_t port, uint8_t byte);

        /**
         * @brief Write a buffer to the specified port
         * 
         * @param port 0 for Bluetooth, 1 for USB, see \ref Ports in \ref initalize_variables.h
         * @param buffer Pointer to the head of the buffer to write
         * @param len Number of bytes to write
         * @return size_t 
         */
        size_t write(uint8_t port, uint8_t* buffer, size_t len);

        /**
         *  @brief Write a byte to the last read SerialCommunicator
         * 
         *  @param[in] byte byte to write
         *  @returns size_t number of bytes written
         */
		size_t write(uint8_t byte);

        /**
         *  @brief Write a buffer to the last read SerialCommunicator
         * 
         *  @param[in] buffer pointer to an array buffer
         *  @param[in] len The number of bytes to write from array
         *  @returns size_t The number of bytes written
         */
		size_t write(uint8_t* buffer, size_t len);

        /**
         *  @brief Print to the last read SerialCommunicator
         * 
         *  @param[in] string String to write to the SerialCommunicator 
         *  @returns size_t Number of bytes written
         */
        size_t print(String string);

        /**
         *  @brief Print with a newline to the last read SerialCommunicator
         * 
         *  @param[in] string String to write to the SerialCommunicator
         *  @returns size_t Number of bytes written
         */
        size_t println(String string);

        /**
         *  @brief Print an unsigned char in a specific base
         * 
         *  @param[in] n the value to print
         *  @param[in] base the base in which to print the value
         *  @returns size_t Number of bytes written
         */
        size_t println(unsigned char n, int base);

        /** 
         *  @brief Wait for data over the last read SerialCommunicator
         * 
         *  Blocking. Will wait for new data to be sent over the line.
         *
         *  @returns bool returns True once finished. 
         */
        bool await_acknowledgment();

        /**
         *  @brief Returns a String up to the specificied terminator
         * 
         *  @param[in] terminator the character at which reading will stop
         *  @param[in] max the maximum chars to read before timeout. Default is 120.
         * 
         *  @returns String the character string until the terminator character
         */
        String readStringUntil(char terminator, size_t max = 120U);

        /**
         * @brief Read byte from active port
         * 
         * @returns The byte read from the port
         */
        uint8_t read_byte_from_active_port();
};

template <typename T>
size_t TagCommunicator::read_type(ByteArray<T>& array) {
    return (*(communicator_ptr + ID))->read_type(array);
}



#endif // GUARD TAG_COMMUNICATOR