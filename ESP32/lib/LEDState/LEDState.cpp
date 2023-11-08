#include "LEDState.h"

void LEDState::update_LED_state() {
    switch (states) {
        case (LEDStates::OFF):
            digitalWrite(pin, LOW);
            states = CONTINUE;
            break;
        case (LEDStates::ON):
            digitalWrite(pin, HIGH);
            states = CONTINUE;
            break;
        case (LEDStates::TRANSITION):
            digitalWrite(pin, HIGH);
            timer = 0;
            states = TOGGLE;
            break;
        case (LEDStates::TOGGLE):
            if (timer > blink_time_ms) {
                digitalWrite(pin, !digitalRead(pin));
                timer = 0;
            }
            break;
        case (LEDStates::CONTINUE):
            break;
        default:
            break;
    }
}

void LEDState::set_state(LEDStates new_state) {
    states = new_state;
}

void LEDState::set_timer_ms(uint32_t time) {
    blink_time_ms = time;
}