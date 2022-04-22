#ifndef GUARD_TAG_DMA
#define GUARD_TAG_DMA

#include <ADC.h>
#include <DMAChannel.h>
#include "AbstractDevice.h"
#include "ADCBuffer.h"

#include "Logger.h"
extern Logger logger;

/**
 * @brief Class to implement an Abstract device for the ADC control
 * 
 * This class controls the speed of sampling, which pin to sample, the resolution, and to where write the data
 * 
 */
class Tag_ADC : public AbstractDevice {
    public:
        /**
         * @brief Construct a new Tag_ADC object
         * 
         * Note that averaging can only take on the values of 0, 2, 4, 8, 16, or 32.
         * The frequency of sampling activates a timer at the set frequency.
         * 
         * @param pin Which pin to sample
         * @param frequency The frequency of sampling
         * @param avg The number of samples to average
         * @param resolution Resolution of sampling
         * @param adc_buffer ADCBuffer pointer in which to save sampled data
         */
        Tag_ADC(uint8_t pin, int frequency, uint8_t avg, int resolution, ADCBuffer* adc_buffer);
        ~Tag_ADC();

        /**
         * @brief Begins sampling
         * 
         * Starts the timer and attaches hardware interrupt to the ADCBuffer
         */
        void begin();

        /**
         * @brief Ends sampling
         * 
         * Stops the timer and detaches interrupt
         */
        void end();

        /**
         * @brief Initializes the ADC settings and sets up the ADCBuffer
         */
        bool init();

    private:
        ADC* _adc; /**< Pointer to an ADC object */
        ADCBuffer* adc_dma; /**< Pointer to ADCBuffer in which data will be stored */
        uint8_t _avg; /**< Number of points to average over */
        uint8_t _resolution; /**< Resolution of sampling */
        uint8_t _pin; /**< Pin number to sample */
        int _frequency; /** Frequency with which to sample */
};

#endif