#ifndef GUARD_IMU_BUFFER
#define GUARD_IMU_BUFFER

#include "Logger.h"
extern TimingData imu_buffer_push;
extern Logger logger;

#include <Arduino.h>
#include "Tag_Queue.h"
#include "AbstractBuffer.h"

/**
 * @brief Class that manages the buffers for the IMU
 * 
 * This class is responsible for managing the writing, filling, and interrupts that allow the device to sample
 */
class IMUBuffer : public AbstractBuffer<int16_t> {
    public:
        /**
         * @brief Construct a new EEGBuffer object
         * 
         * This class implements AbstractBuffer and uses it to set up the buffers.
         * 
         * @param buffers A pointer to an array containing pointers to buffers
         * @param num_buffers The number of buffers (length of the \ref buffers array)
         * @param buffer_length The lengths of each array in \ref buffers
         * @param id Sets the idnetification bytes for each buffer
         * @param queue Defines the queue object into which the pointers of the filled buffers will be sent
         */
        IMUBuffer(volatile int16_t** buffers, size_t num_buffers, size_t buffer_length, uint8_t id, Tag_Queue* queue) :
            AbstractBuffer(buffers, num_buffers, buffer_length, id) {
                _queue = queue;
            };

        /**
         * @brief Writes data from the source to the IMU Buffer
         * 
         * @param source Pointer from where the data is to be written
         * @param length Length of the data to read
         */
        void write(volatile int16_t* source, size_t length);

        /**
         * @brief Resets the IMU Buffers and associated indices
         * 
         * The time and count in the buffers is set to 0, data is zeroed out in the buffers,
         * and the buffer selector, current index, and buffer trigger count is reset to 0.
         */
        void reset();

        /**
         * @brief Get the index of the buffer currently being written to
         * 
         * @return uint8_t The index of the buffer
         */
        uint8_t get_buffer_selector() { return buffer_selector; }

        /**
         * @brief Get the index in the active buffer where data will be written to next
         * 
         * Note that a uint16_t index will maximally be 65535.
         * 
         * @return uint16_t The index
         */
        uint16_t get_index() { return current_index; }

        /**
         * @brief Get the trigger count object
         * 
         * @return uint8_t The total number of times a buffer was sent to the queue
         */
        uint8_t get_trigger_count() { return buffer_trigger_count; }

    private:
        Tag_Queue* _queue; /**< Pointer to the queue object to which buffers will be sent */
        void reset_buffer(volatile BufferHelper<int16_t>* buff); /**< Helper function to reset an individual buffer */
        void increment_and_check_push(); /**< Checks whether the buffer if filled and thus then has to be sent to the queue */
};

#endif //GUARD_TAG_IMU_BUFFER