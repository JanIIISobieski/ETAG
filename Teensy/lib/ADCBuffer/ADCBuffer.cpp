#include "ADCBuffer.h"

// Global objects
ADCBuffer *ADCBuffer::_activeObjectPerADC[2] = {nullptr, nullptr};

void ADCBuffer::init(ADC *adc, int8_t adc_num)
{
    this->adc_num = adc_num;
    // enable DMA and interrupts
#ifndef KINETISL
  // setup a DMA Channel.
  // See if we were created with one or two buffers.  If one assume we stop on completion, else assume continuous.
    if (buffers[1].data_ptr && buffers[1].data_length) {
#ifdef ADC_DUAL_ADCS
        _dmasettings_adc[0].source((volatile uint16_t&)((adc_num == 1) ? SOURCE_ADC_1 : SOURCE_ADC_0));
#else
        _dmasettings_adc[0].source((volatile uint16_t&)(SOURCE_ADC_0));
#endif
        _dmasettings_adc[0].destinationBuffer((uint16_t*)(buffers[0].data_ptr), (buffers[0].data_length) * 2); // 2*b_size is necessary for some reason
        _dmasettings_adc[0].replaceSettingsOnCompletion(_dmasettings_adc[1]);    // go off and use second one...
        _dmasettings_adc[0].interruptAtCompletion(); //interruptAtHalf or interruptAtCompletion
#ifdef ADC_DUAL_ADCS
        _dmasettings_adc[1].source((volatile uint16_t&)((adc_num == 1) ? SOURCE_ADC_1 : SOURCE_ADC_0));
        _dmasettings_adc[1].destinationBuffer((uint16_t*)(buffers[1].data_ptr), (buffers[1].data_length) * 2); // 2*b_size is necessary for some reason
        _dmasettings_adc[1].replaceSettingsOnCompletion(_dmasettings_adc[0]);    // Cycle back to the first one
        _dmasettings_adc[1].interruptAtCompletion(); //interruptAtHalf or interruptAtCompletion
#endif

        _dmachannel_adc = _dmasettings_adc[0];
        _stop_on_completion = false;
    } else {
#ifdef ADC_DUAL_ADCS
        _dmachannel_adc.source((volatile uint16_t&)((adc_num == 1) ? SOURCE_ADC_1 : SOURCE_ADC_0));
#else
        _dmachannel_adc.source((volatile uint16_t&)(SOURCE_ADC_0));
#endif
        _dmachannel_adc.destinationBuffer((uint16_t*)(buffers[0].data_ptr), (buffers[0].data_length) * 2); // 2*b_size is necessary for some reason
        _dmachannel_adc.interruptAtCompletion(); //interruptAtHalf or interruptAtCompletion
        _dmachannel_adc.disableOnCompletion();    // we will disable on completion.
        _stop_on_completion = true;
    }

  //arm_dcache_flush((void*)dmaChannel, sizeof(dmaChannel));
    _dmachannel_adc.enable();

    adc->adc[adc_num]->continuousMode();
    adc->adc[adc_num]->enableDMA();
#else
    // Kinetisl (TLC)
    // setup a DMA Channel.
    // Now lets see the different things that RingbufferDMA setup for us before
    _dmachannel_adc.source((volatile uint16_t&)(SOURCE_ADC_0));;
    _dmachannel_adc.destinationBuffer((uint16_t*)_buffer1, _buffer1_count * 2); // 2*b_size is necessary for some reason
    _dmachannel_adc.disableOnCompletion();    // ISR will hae to restart with other buffer
    _dmachannel_adc.interruptAtCompletion(); //interruptAtHalf or interruptAtCompletion
    _activeObjectPerADC[0] = this;
    _dmachannel_adc.attachInterrupt(&adc_0_dmaISR);
    _dmachannel_adc.triggerAtHardwareEvent(DMAMUX_ADC_0); // start DMA channel when ADC finishes a conversion
    _dmachannel_adc.enable();

    adc->startContinuous(adc_num);
    adc->enableDMA(adc_num);
#ifdef DEBUG_DUMP_DATA
    dumpDMA_TCD(&_dmachannel_adc);
#endif

#endif
}

void ADCBuffer::begin() {
    if (adc_num == 1) {
#ifdef ADC_DUAL_ADCS
        _activeObjectPerADC[1] = this;
        _dmachannel_adc.attachInterrupt(&adc_1_dmaISR);
        _dmachannel_adc.triggerAtHardwareEvent(DMAMUX_ADC_1); // start DMA channel when ADC finishes a conversion
#endif
    } else {
        _activeObjectPerADC[0] = this;
        _dmachannel_adc.attachInterrupt(&adc_0_dmaISR);
        _dmachannel_adc.triggerAtHardwareEvent(DMAMUX_ADC_0); // start DMA channel when ADC finishes a conversion
    }

    logger.print_DMA_settings("Buffer 1", &_dmasettings_adc[0]);
    logger.print_DMA_settings("Buffer 2", &_dmasettings_adc[1]);

    stopOnCompletion(false);
}

void ADCBuffer::end() {
    stopOnCompletion(true);
    _dmachannel_adc.detachInterrupt();
}

void ADCBuffer::stopOnCompletion(bool stop_on_complete)
{
#ifndef KINETISL
    if (stop_on_complete) _dmachannel_adc.TCD->CSR |= DMA_TCD_CSR_DREQ;
    else _dmachannel_adc.TCD->CSR &= ~DMA_TCD_CSR_DREQ;
#else
    if (stop_on_complete) _dmachannel_adc.CFG->DCR |= DMA_DCR_D_REQ;
    else _dmachannel_adc.CFG->DCR &= ~DMA_DCR_D_REQ;
#endif
    _stop_on_completion = stop_on_complete;
}

bool ADCBuffer::clearCompletion()
{
    if (!_stop_on_completion) return false;
        // should probably check to see if we are dsiable or not...
    _dmachannel_adc.enable();
    return true;
}

void  ADCBuffer::processADC_DMAISR() {
    *(buffers[buffer_selector].count_ptr) = buffer_trigger_count++;
    *(buffers[buffer_selector].time_ptr) = micros();
    _queue->enqueue(buffers[buffer_selector].id_ptr);
    buffer_selector = 1 - buffer_selector;
    // update the internal buffer positions
    _dmachannel_adc.clearInterrupt();
#ifdef KINETISL
    // Lets try to clear the previous interrupt, change buffers
    // and restart
    if (_buffer2 && (_interrupt_count & 1)) {
        _dmachannel_adc.destinationBuffer((uint16_t*)_buffer2, _buffer2_count * 2); // 2*b_size is necessary for some reason
    } else {
        _dmachannel_adc.destinationBuffer((uint16_t*)_buffer1, _buffer1_count * 2); // 2*b_size is necessary for some reason
    }

    // If we are not stopping on completion, then reenable...
    if (!_stop_on_completion) _dmachannel_adc.enable();
#endif
}

void ADCBuffer::adc_0_dmaISR() {
    if (_activeObjectPerADC[0]) {
        _activeObjectPerADC[0]->processADC_DMAISR();
    }
#ifdef ETAG_DEBUG
    logger.update_timing_data(hydrophone_buffer_push);
#endif

#if defined(__IMXRT1062__)  // Teensy 4.0
    asm("DSB");
#endif
}

void ADCBuffer::adc_1_dmaISR() {
    if (_activeObjectPerADC[1]) {
        _activeObjectPerADC[1]->processADC_DMAISR();
    }
#if defined(__IMXRT1062__)  // Teensy 4.0
    asm("DSB");
#endif
}

void ADCBuffer::reset() {
    reset_buffer(&buffers[0]);
    reset_buffer(&buffers[1]);
    buffer_selector = 0;
    buffer_trigger_count = 0;
}

void ADCBuffer::reset_buffer(volatile BufferHelper<uint16_t>* buff) {
    *(buff->count_ptr) = 0;
    *(buff->time_ptr) = 0;
    for (size_t i = 0; i < (buff->data_length); i++) {
        *(buff->data_ptr + i) = 0;
    } 
}
