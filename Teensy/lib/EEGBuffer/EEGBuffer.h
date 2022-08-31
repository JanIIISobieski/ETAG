#ifndef GUARD_EEG_BUFFER
#define GUARD_EEG_BUFFER

#include "AbstractBuffer.h"
#include "Tag_Queue.h"
#include <SPI.h>

#include "Logger.h"
extern Logger logger;
extern TimingData eeg_buffer_push;

#define EEG_DMA_SPI_SPEED  2000000

/**
 * @brief Event responder to a completion of a DMA SPI transfer
 * 
 * @param event Event to which this function will respond to (a reference to an EventResponder)
 * 
 * This function will set the chip select to HIGH, end the SPI transation, and write the data
 * to the EEG Buffer. Pass this to dma_spi_transfer.attatch() in this class's init().
 */
void dma_spi_finished(EventResponderRef event);


/**
 * @brief Event responder to a completion of a DMA SPI transfer when wanting to stream the data
 * 
 * @param event Event to which this function will respond to (a reference to an EventResponder)
 * 
 * This function will set the chip select to HIGH, end the SPI transation, and write the data
 * to the EEG Buffer, while also setting a new_data flag to true. Pass this to dma_spi_transfer.attatch()
 * to this class's init().
 */
void dma_spi_finished_streaming(EventResponderRef event);

/**
 * @brief ADS1299 data ready ISR
 * 
 * Whenever the data ready pin goes LOW, the SPI transaction begins, the Chip Select pin is set to low,
 * and the DMA transfer is initalized to save the transferred data to the \ref eeg_buffer in EEGBuffer.
 */
void ads_ISR();

/**
 * @brief Class that manages the buffers for the EEG
 * 
 * This class is responsible for managing the writing, filling, and interrupts that allow the device to sample
 */
class EEGBuffer : public AbstractBuffer<uint8_t> {
public:
    /**
     * @brief Construct a new EEGBuffer object
     * 
     * This class implements AbstractBuffer and uses it to set up the buffers.
     * 
     * @param buffers A pointer to an array containing pointers to buffers
     * @param num_buffers The number of buffers (length of the \ref buffers array)
     * @param length_buffers The lengths of each array in \ref buffers
     * @param id Sets the idnetification bytes for each buffer
     * @param queue Defines the queue object into which the pointers of the filled buffers will be sent
     */
    EEGBuffer(volatile uint8_t** buffers, size_t num_buffers, size_t length_buffers, uint8_t id, Tag_Queue* queue) :
        AbstractBuffer(buffers, num_buffers, length_buffers, id) {
            _queue = queue;
            buffer_selector = 0;
            buffer_trigger_count = 0;
            current_index = 0;
            this->num_buffers = num_buffers;
        }

    /**
     * @brief Initializes the sampling
     * 
     * This method attaches dma_spi_finished to function to the dma_spi_transfer, which is called on each SPI.transfer() in the ads_ISR()
     * Additionally, the SPI uses the EEG_DRDY pin 14 for 
     */
    void init();

    /**
     * @brief Default writer of the EEG data
     * 
     * Writes the 27-bytes from the ADS1299 in \ref eeg_data_vec to the appropriate location in buffers
     */
    void write();

    /**
     * @brief Writer of the EEG Data to the EEGBuffers
     * 
     * @param source Location from which to write data
     * @param length Length of the data to write
     */
    void write(volatile uint8_t* source, size_t length);

    /**
     * @brief Resets the EEGBuffers, as well as the incides, buffer selector, and trigger count
     * 
     * The time and count are changed to 0, and the data overwritten with 0s, while the ID byte remains unchanged
     * 
     */
    void reset();

    /**
     * @brief Get the trigger count
     * 
     * Gets the number of times a buffer was filled and sent to the queue
     * 
     * @return uint8_t 
     */
    uint8_t get_trigger_count() { return buffer_trigger_count; }

    /**
     * @brief Get the index object
     * 
     * @return size_t The current index in the data buffer
     */
    size_t get_index() { return current_index; }

    /**
     * @brief Get the pointer to the EEG data object
     * 
     * @return uint8_t* Pointer to where the eeg_data is stored (points to the first byte of EEG data)
     */
    volatile uint8_t* get_eeg_pointer() { return &(data_buffer.data.data[0]); }

    /**
     * @brief Event responder used as a callback after the DMA SPI transfer
     * 
     * Used an input into SPI.transfer()
     */
    EventResponder dma_spi_transfer;

    /**
     * @brief Sets the state of new_data
     */
    void set_new_data(bool val) { new_data = val; };

    /**
     * @brief Gets the new_data state
     */
    volatile bool get_data_state() { return new_data; };

    /**
     * @brief Set the time object
     * 
     * Sets the time on the data object.
     */
    void set_time() { data_buffer.data.time = micros(); }

    /**
     * @brief Get the buffer object
     * 
     * @return uint8_t* Return the full data buffer
     */
    uint8_t* get_buffer() { return data_buffer.buffer; }

private:
    void reset_buffer(volatile BufferHelper<uint8_t>* buff); /**< Helper function to reset a single buffer */
    inline void increment_and_check_push(); /**< Function to check whether the end of the buffer has been reached, and thus the buffer has to be added to the queue*/
    Tag_Queue *_queue; /**< Pointer to the queue object where filled buffers will be sent */

    volatile bool new_data; /**< Flag for data read from EEG using the interrupt and it has not been read yet */

    /**
     * @brief The data buffer object as a union
     * 
     * Allows for easier sending of the data by using the 32-byte buffer.
     * The structure enforces the format of the data: time (4-bytes) followed by the
     * 27 bytes of the EEG data, followed by a newline character.
     * 
     */
#pragma pack(push, 1)
    union DataBuffer {
        uint8_t buffer[31];
        struct Data {
            volatile uint32_t time;
            volatile uint8_t data[27];
        } data;
    } data_buffer;
#pragma pack(pop)
};

#endif //GUARD_EEG_BUFFER