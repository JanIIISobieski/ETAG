#ifndef GUARD_TAG_QUEUE
#define GUARD_TAG_QUEUE

#include <Arduino.h>

#define QUEUE_MAX_LENGTH 16

/**
 *  Class that mananges the buffer pointers that are ready for writing to the SD card.
 *  
 *  This class effectively emulates a First-In/First-Out (FIFO) circular buffer to which pointers are written (enqueued) consecutively, starting from the 0th index.
 *  Pointers are dequeued consecutively as well, starting from the 0th index. Pointers not yet written will be overwritten by a push.
 *  Should that occur, the #num_to_write() function will return a value greater than its size (which is returned from #get_max_size()). 
 *  As there is a data race between filling the buffers and in writing the buffers, there is no guarantee that 
 */
class Tag_Queue {
private:
    volatile uint8_t* queue[QUEUE_MAX_LENGTH]; /**< The start of the queue */

    uint8_t to_pop;                            /**< The index where the next buffer can be enqueued (write buffer to the queue) */
    volatile uint8_t to_write;                 /**< The index of the next buffer to dequeue (write buffer data to SD) */

    uint8_t size;                              /**< The length of the buffer */
    volatile uint8_t run;                      /**< The number of buffers left to write */

    void next(uint8_t &ptr);                   /**< Increments index (handling over-flow) */
    void next(volatile uint8_t &ind);          /**< Increments index (handling over-flow) */

    volatile uint8_t* popped = nullptr;        /**< Address of popped buffer */

public:
    /**
     *  Class constructor.
     * 
     *  The total size of the queue is given by QUEUE_MAX_LENGTH in the top of this file.
     */
    Tag_Queue() : size(QUEUE_MAX_LENGTH) {};

    /**
     *  Class destructor.
     */
    ~Tag_Queue() {};

    /**
     *  Method to enqueue volatile pointers to the queue, returns nothing. 
     * 
     *  Write at the current location of the to_write index the pointer to the buffer.
     *  Also increments the counter for the number of buffers that are to be written.
     *  This function is used in the interrupt routines of the myAnalogBufferClass().
     */
    void enqueue(volatile uint8_t* ptr);

    /**
     *  Method to dequeue the volatile pointers from the queue, returning a uint16_t pointer. 
     * 
     *  This pointer is written to #popped, while the previous #popped is written to #prev_popped.
     *  This method also decrements the buffer counter of buffers that are to be written.
     *  The pointer is returned while the current buffer location in the queue becomes replaced with a nullptr.
     */
    volatile uint8_t* dequeue();
    
    /**
     *  Getter method for the number of buffers left to write in the queue 
     */
    int num_to_write() { return run; };

    /**
     *  Getter method for the maximum size of the queue. 
     */
    int get_max_size() { return size; };

    /**
     *  Getter method for the current index of the buffer whose pointer will be returned by pop().
     */
    size_t get_pop_index() { return to_pop; };

    /**
     *  Getter method for the current index of the buffer that will be written to by push().
     */
    size_t get_write_index() { return to_write; };

    /**
     *  Getter method for the pointer that was just popped.
     */

    volatile uint8_t* get_popped() { return popped; };
    
    /**
     *  Resets the Queue buffer
     * 
     *  All elements inside are replaced with a `nullptr`, the pushing and popping pointers are reset to the 0th element,
     *  and the value returned by num_to_write() is set to 0. Size of queue remains unchanged.
     */
    void reset();
};

#endif //GUARD_TAG_QUEUE