#include "ADCBuffer2.h"

ADCBuffer2 *ADCBuffer2::_activeObjectPerADC[2] = {nullptr, nullptr};

void ADCBuffer2::init(ADC* adc, int8_t adc_num) {
    logger.print_message("In init of ADCBuffer2");
    this->adc_num = adc_num;
    logger.print_variable("Num buffers", num_buffers);

    for (size_t i = 0; i < num_buffers; i++)
    {
        size_t ind = i;
        _dmasettings_adc[i].source((volatile uint16_t&)((adc_num == 1) ? SOURCE_ADC_1 : SOURCE_ADC_0));
        _dmasettings_adc[i].destinationBuffer((uint16_t*)(buffers[i].data_ptr), (buffers[i].data_length) * 2);  //2*b_size is necessary for some reason
        _dmasettings_adc[i].replaceSettingsOnCompletion(_dmasettings_adc[((++ind == num_buffers) ? 0 : ind)]);    //change buffer to the next, if at the end of the buffer, then use the settings of the first
        _dmasettings_adc[i].interruptAtCompletion();

        logger.print_variable("Current Index", i);
        logger.print_variable("Next Index", ((ind == num_buffers) ? 0 : ind));
    }

    _dmachannel_adc = _dmasettings_adc[0];
    _stop_on_completion = false;

    _dmachannel_adc.enable();

    adc->adc[adc_num]->continuousMode();
    adc->adc[adc_num]->enableDMA();

    for (size_t i = 0; i < num_buffers; i++) {
        logger.print_DMA_settings("Buffer", &_dmasettings_adc[i]);
    }
    logger.print_variable("Stop on completion", _stop_on_completion);
}

void ADCBuffer2::begin() {
    if (adc_num == 1) {
        _activeObjectPerADC[1] = this;
        _dmachannel_adc.attachInterrupt(&adc_1_dmaISR);
        _dmachannel_adc.triggerAtHardwareEvent(DMAMUX_ADC_1);
    } else {
        _activeObjectPerADC[0] = this;
        _dmachannel_adc.attachInterrupt(&adc_0_dmaISR);
        _dmachannel_adc.triggerAtHardwareEvent(DMAMUX_ADC_0);
    }

    stopOnCompletion(false);

    logger.print_variable("Stop on completion", _stop_on_completion);
}

void ADCBuffer2::end() {
    stopOnCompletion(true);
    _dmachannel_adc.detachInterrupt();
    logger.print_variable("Stop on completion", _stop_on_completion);
}

void ADCBuffer2::stopOnCompletion(bool stop_on_complete) {
    if (stop_on_complete) _dmachannel_adc.TCD->CSR |= DMA_TCD_CSR_DREQ;
    else _dmachannel_adc.TCD->CSR &= ~DMA_TCD_CSR_DREQ;

    _stop_on_completion = stop_on_complete;
}

bool ADCBuffer2::clearCompletion() {
    if (!_stop_on_completion) return false;
    _dmachannel_adc.enable();

    return true;
}

void ADCBuffer2::processADC_DMAISR() {
    *(buffers[buffer_selector].count_ptr) = buffer_trigger_count++;
    *(buffers[buffer_selector].time_ptr) = micros();
    _queue->enqueue(buffers[buffer_selector].id_ptr);
    buffer_selector = ((++buffer_selector == num_buffers) ? 0 : buffer_selector);
    _dmachannel_adc.clearInterrupt();
}

void ADCBuffer2::adc_0_dmaISR() {
    if (_activeObjectPerADC[0]) {
        _activeObjectPerADC[0]->processADC_DMAISR();
    }
#ifdef ETAG_DEBUG
    logger.update_timing_data(hydrophone_buffer_push);
#endif
    asm("DSB");  //Data synchronization barrier
}

void ADCBuffer2::adc_1_dmaISR() {
    if (_activeObjectPerADC[1]) {
        _activeObjectPerADC[1]->processADC_DMAISR();
    }
#ifdef ETAG_DEBUG
    logger.update_timing_data(hydrophone_buffer_push);
#endif
    asm("DSB");
}

void ADCBuffer2::reset() {
    for (size_t i = 0; i < num_buffers; i++)
    {
        reset_buffer(&buffers[i]);
    }
    
    buffer_selector = 0;
    buffer_trigger_count = 0;
}

void ADCBuffer2::reset_buffer(volatile BufferHelper<uint16_t>* buff) {
    *(buff->count_ptr) = 0;
    *(buff->time_ptr) = 0;
    for (size_t i = 0; i < (buff->data_length); i++) {
        *(buff->data_ptr + i) = 0;
    } 
}