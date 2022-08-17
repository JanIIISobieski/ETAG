#pragma once

#include <DMAChannel.h>
#include <ADC.h>
#include "Tag_Queue.h"
#include "AbstractBuffer.h"

#include "Logger.h"
extern TimingData hydrophone_buffer_push;
extern Logger logger;

#if defined(__IMXRT1062__)  // Teensy 4.0
#define SOURCE_ADC_0    ADC1_R0
#define DMAMUX_ADC_0    DMAMUX_SOURCE_ADC1
#define SOURCE_ADC_1    ADC2_R0
#define DMAMUX_ADC_1    DMAMUX_SOURCE_ADC2
#endif

class ADCBuffer2 : public AbstractBuffer<uint16_t> {
    public:
        ADCBuffer2(volatile uint16_t** buffers, size_t num_buffers, size_t length_buffers, uint8_t id, Tag_Queue* queue) :
        AbstractBuffer(buffers, num_buffers, length_buffers, id) {
            _queue = queue;
            _dmasettings_adc = new DMASetting [num_buffers];
        }

        ~ADCBuffer2() {
            delete[] _dmasettings_adc;
        };

        void init(ADC *adc, int8_t adc_num = -1);
        void begin();
        void end();

        void stopOnCompletion(bool stop_on_complete);
        inline bool stopOnCompletion() {return _stop_on_completion; };
        bool clearCompletion();
        void reset();
        uint8_t get_trigger_count() { return buffer_trigger_count; };

        DMASetting* _dmasettings_adc;
        DMAChannel _dmachannel_adc;

        static ADCBuffer2* _activeObjectPerADC[2];

        static void adc_0_dmaISR();
        static void adc_1_dmaISR();
        void processADC_DMAISR();

    private:
        Tag_Queue* _queue;        
        bool _stop_on_completion = false;
        void reset_buffer(volatile BufferHelper<uint16_t>* buff);
        int8_t adc_num;
};