#ifndef GUARD_ADC_BUFFER
#define GUARD_ADC_BUFFER

#include <DMAChannel.h>
#include <ADC.h>
#include "Tag_Queue.h"
#include "AbstractBuffer.h"

#include "Logger.h"
extern Logger logger;
extern TimingData hydrophone_buffer_push;

#if defined(__IMXRT1062__)  // Teensy 4.0
#define SOURCE_ADC_0    ADC1_R0
#define DMAMUX_ADC_0    DMAMUX_SOURCE_ADC1
#define SOURCE_ADC_1    ADC2_R0
#define DMAMUX_ADC_1    DMAMUX_SOURCE_ADC2
#elif defined(KINETISK)
#define SOURCE_ADC_0    ADC0_RA
#define DMAMUX_ADC_0    DMAMUX_SOURCE_ADC0
#ifdef ADC_DUAL_ADCS
#define SOURCE_ADC_1    ADC1_RA
#define DMAMUX_ADC_1    DMAMUX_SOURCE_ADC1
#endif
#elif defined(KINETISL)
#define SOURCE_ADC_0    ADC0_RA
#define DMAMUX_ADC_0    DMAMUX_SOURCE_ADC0
#endif

/**
 *  @brief ADCBuffer class for managing sampling of pins
 * 
 *  This class is related to AnalogDMABuffer by Pedro Villanueva. It was simplified
 *  and edited to work with the interface needed for this project. Importantly, this
 *  framework for high speed sampling of the ADC allows for using an interrupt to 
 *  trigger a DMA transfer from an ADC register to the buffer, and once one buffer fills up,
 *  send it to the Queue (through an interrupt process that triggers once a buffer is filled),
 *  while also setting up the next buffer to be ready to take in data (to minimize downtime).
 */ 
class ADCBuffer : public AbstractBuffer<uint16_t> {
public:
#ifndef KINETISL
    DMASetting  _dmasettings_adc[2]; /**< Array to store the settings for each buffer */
#endif
    DMAChannel  _dmachannel_adc; /**< Allows for setting the source, destination, and interrupt scheme for a DMA channel */

    static ADCBuffer *_activeObjectPerADC[2]; /**< Allows for global variable to be used for interrupt */

    /**
     * @brief Called for first ADC when DMA has completed filling a buffer
     * 
     * Note that the asm("DSB") is a data synchronization assembly command
     */
    static void adc_0_dmaISR();
    
    /**
     * @brief Used for processors that have a second ADC object (such as the Teensy 4)
     * 
     * Note that the asm("DSB") is a data synchronization assembly command
     */    
    static void adc_1_dmaISR();

    /**
     * @brief Process the DMA completion ISR
     * 
     * Common for both ISRs on those processors who have more than one ADC
     */ 
    void processADC_DMAISR();

public:
    /**
     * @brief Construct a new ADCBuffer object
     * 
     * This class implements AbstractBuffer and uses it to set up the buffers.
     * 
     * @param buffers A pointer to an array containing pointers to buffers
     * @param num_buffers The number of buffers (length of the \ref buffers array)
     * @param length_buffers The lengths of each array in \ref buffers
     * @param id Sets the idnetification bytes for each buffer
     * @param queue Defines the queue object into which the pointers of the filled buffers will be sent
     */
    ADCBuffer(volatile uint16_t** buffers, size_t num_buffers, size_t length_buffers, uint8_t id, Tag_Queue* queue) :
        AbstractBuffer(buffers, num_buffers, length_buffers, id) {
            _queue = queue;
        }
    
    /**
     * @brief Initialize the object including setup of the DMA structures
     * 
     * @param adc A pointer to an ADC object
     * @param adc_num The register of the ADC to use
     */
    void init(ADC *adc, int8_t adc_num = -1);

    /**
     * @brief The method to call beginning sampling
     * 
     * This method attaches the interrupts and triggering of the buffers
     */
    void begin();

    /**
     * @brief The method to call for ending sampling
     * 
     * This method detaches the interrupts and stops the DMA_MUX after a buffer is filled
     * (at 500kHz, this would take at most 8 ms, at 40kHz, this would take about 102 ms).
     */
    void end();

    /**
     * @brief allows for stopping when a DMA buffer has completed filling
     * 
     * Defaults to true when only 1 buffer passed into this class, otherwise to false
     * 
     * @param stop_on_complete if true, then stopped on buffer completion, otherwise false
     */
    void stopOnCompletion(bool stop_on_complete);

    /**
     * @brief Getter method for stop on completion
     * 
     * @return true if the sampling it to stop after filling a buffer
     * @return false if the sampling is to continue after filling a buffer
     */
    inline bool stopOnCompletion(void) {return _stop_on_completion;}

    /**
     * @brief if we have stop on completion, then clear the completion state i.e. reenable the DMA operation.
     * 
     * @return true if DMA channel has been re-enabled
     * @return false if the DMA channel has not been re-enabled (or stop_on_completion was false)
     */
    bool clearCompletion();

    /**
     * @brief Resets the buffers to their default state
     * 
     * The ID byte remains the same, but the count and times get reset to 0,
     * and the data is reset as well
     */
    void reset();

    /**
     * @brief Get the trigger count object
     * 
     * @return uint8_t The number of times a buffer was enqued on the queue from this class
     */
    uint8_t get_trigger_count() { return buffer_trigger_count; }

protected:
    bool _stop_on_completion = false; /**< The flag for stopping on completion */
    Tag_Queue *_queue; /**< A pointer to the queue responsible for sending data to the SD card */
    void reset_buffer(volatile BufferHelper<uint16_t>* buff); /**< An array of BufferHelper objects to refernce the saved buffers */
    int8_t adc_num; /**< The address of the ADC to use */
};

#endif //GUARD_ADC_BUFFER