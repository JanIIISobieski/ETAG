#pragma once
#include <Arduino.h>
#include "elapsedMillis.h"

enum LEDStates {
    OFF        = 0,
    ON         = 1,
    TOGGLE     = 2,
    TRANSITION = 3,
    CONTINUE   = 4,
};

class LEDState {
    public:
        LEDState(uint8_t pin_number) : pin(pin_number) { states = ON; blink_time_ms = 1000; }; 
    
        void update_LED_state();
        void set_state(LEDStates new_state);
        void set_timer_ms(uint32_t time);

    private:
        uint8_t pin;
        uint32_t blink_time_ms;
        elapsedMillis timer;
        LEDStates states;
};