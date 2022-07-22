#pragma once

#include <Arduino.h>

class BufferBase {
    protected:
        uint8_t identifier; /**< The identifier byte for the buffers */

        size_t num_buffers; /**< The number of buffers */
        volatile uint8_t buffer_selector; /**< An index selecting the current active buffer */
        volatile uint8_t buffer_trigger_count; /**< The number of times any buffer was sent to the SD card */
        volatile size_t current_index; /**< The current location in a buffer */

    public:
        /**
         * @brief Construct a new Abstract Buffer object
         * 
         * @param buff_vec A pointer to an array containing pointers to buffers of type T
         * @param num_buffers The number of buffers (length of the \ref buff_vec array)
         * @param length_buffers The length of each array passed in \ref buff_vec
         * @param id Sets the identification bytes for each buffer
         */
        BufferBase(size_t num_buffers, size_t length_buffers, uint8_t id);


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
         * @brief Get the current index object
         * 
         * @return size_t The index of the buffer currently being filled
         */
        size_t get_current_index() { return current_index; };
};