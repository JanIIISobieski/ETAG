#include "Tag_ADC.h"

Tag_ADC::Tag_ADC(uint8_t pin, ADCSettings* adc_settings, ADCBuffer* adc_buffer) {
    _adc = new ADC();
    _pin = pin;
    this->adc_settings = adc_settings;

    adc_dma = adc_buffer;
}

Tag_ADC::~Tag_ADC() {
    delete _adc;
}

bool Tag_ADC::init() {
    logger.print_message("In init of Tag_ADC");

    pinMode(_pin, INPUT);

    _adc->adc0->setAveraging((adc_settings->fields).avg);
    _adc->adc0->setResolution((adc_settings->fields).resolution);
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
    _adc->adc0->startTimer((adc_settings->fields).frequency);

    adc_dma->begin();
}

void Tag_ADC::end() {
    logger.print_message("Ending Hydrophone ADC");
    _adc->adc0->stopTimer();
    adc_dma->end();
}

void Tag_ADC::update_settings() {
    _adc->adc0->setAveraging((adc_settings->fields).avg);
    _adc->adc0->setResolution((adc_settings->fields).resolution);
}