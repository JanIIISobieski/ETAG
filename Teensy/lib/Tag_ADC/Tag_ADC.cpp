#include "Tag_ADC.h"

Tag_ADC::Tag_ADC(uint8_t pin, int frequency, uint8_t avg, int resolution, ADCBuffer* adc_buffer) {
    _adc = new ADC();
    _avg = avg;
    _resolution = resolution;
    _pin = pin;
    _frequency = frequency;

    adc_dma = adc_buffer;
}

Tag_ADC::~Tag_ADC() {
    delete _adc;
}

bool Tag_ADC::init() {
    logger.print_message("In init of Tag_ADC");

    pinMode(_pin, INPUT);

    _adc->adc0->setAveraging(_avg);
    _adc->adc0->setResolution(_resolution);
    _adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
    _adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);
    _adc->adc0->setReference(ADC_REFERENCE::REF_3V3);
    _adc->adc0->recalibrate();

    adc_dma->init(_adc, ADC_0);

    return true;
}

void Tag_ADC::begin() {
    logger.print_message("Starting Hydrophone ADC");
    _adc->adc0->startSingleRead(_pin);
    _adc->adc0->startTimer(_frequency);

    adc_dma->begin();
}

void Tag_ADC::end() {
    logger.print_message("Ending Hydrophone ADC");
    _adc->adc0->stopTimer();
    adc_dma->end();
}