#pragma once

#include <Arduino.h>

/**
 * @brief Abstract structure containing the important descriptors for each buffer
 * 
 * @tparam T Any valid type (e.g. uint32_t, int16_t, unsigned char, etc.)
 * 
 * 
 * Each buffer is of the form:\n
 * `[    ID   |   Count  | --- Time --- | --------------Data-------------------]`\n
 * `[  1 byte |  1 Byte  |    4 bytes   | ---------Remaining Bytes-------------]`\n 
 */
template <class T>
struct BufferHelper {
    volatile T* head_ptr; /**< Pointer to the start of the buffer, as type T */
    volatile uint8_t* id_ptr; /**< Pointer to the id pointer, the first byte of the buffer, which identifies the sampling device this data comes from */
    volatile uint8_t* count_ptr; /**< Pointer to the count, which counts the total number of times a buffer for this specific device was pushed to the SD card (will overflow) */
    volatile uint32_t* time_ptr; /**< Pointer to the time, which is the time at which the buffer was filled */
    volatile T* data_ptr; /**< Pointer to the start of the data section, as type T */
    size_t length; /**< Metadata for the total length of the buffer, using the size of type T to measure (an 8192 byte buffer of type uint16_t would read 4096) */
    size_t data_length; /**< Metadata for the total length of the data_buffer, using the size of type T to measure (an 8192 buffer minus the 6 byte header gives a data length of 4093)*/ 
};

/**
 *  @brief Abstract class to manage the data buffers
 *  
 *  As this is an abstract class, it cannot be instantiated directly, and is instead used as a base from which
 *  the other buffers are derived. Every sampling instance requires two data buffers. As one fills up, the next buffer
 *  saves data while the filled one awaits to be written to the SD card. Additionally, each data buffer needs to have
 *  a short header containing the ID of the data, filled buffer count, and time. This class essentially builds a \ref BufferHelper
 *  for each buffer passed into it.
 */
template <class T>
class AbstractBuffer {
    public:
        /**
         * @brief Construct a new Abstract Buffer object
         * 
         * @param buff_vec A pointer to an array containing pointers to buffers of type T
         * @param num_buffers The number of buffers (length of the \ref buff_vec array)
         * @param length_buffers The length of each array passed in \ref buff_vec
         * @param id Sets the identification bytes for each buffer
         */
        AbstractBuffer(volatile T** buff_vec, size_t num_buffers, size_t length_buffers, uint8_t id);

        /**
         * @brief Destroy the Abstract Buffer object
         * 
         * Since AbstractBuffer() uses new[], the destructor object must call delete[].
         * This should never be called by the Teensy, as the classes are initialized globally,
         * and never unallocated
         */
        ~AbstractBuffer();

        /**
         * @brief Pure virtual function to reset the buffers to their initial state
         */
        virtual void reset() = 0;

        /**
         * @brief Get the number of buffers
         * 
         * @return size_t The number of buffers
         */
        size_t get_num_buffers() { return num_buffers; };

        /**
         * @brief Get the buffers object
         * 
         * @return volatile* of type T, the pointer to the array of BufferHelper objects, most useful when logging
         */
        volatile BufferHelper<T>* get_buffers() { return buffers; };

    protected:
        volatile BufferHelper<T>* buffers; /**< The pointer to the head of the array of BufferHelper objects */
        uint8_t identifier; /**< The identifier byte for the buffers */

        size_t num_buffers; /**< The number of buffers */
        volatile uint8_t buffer_selector; /**< An index selecting the current active buffer */
        volatile uint8_t buffer_trigger_count; /**< The number of times any buffer was sent to the SD card */
        volatile size_t current_index; /**< The current location in a buffer */
};
