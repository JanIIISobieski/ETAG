#include <Tag_NIRS.h>

NIRS::NIRS(uint8_t left_button_pin, uint8_t right_button_pin, unsigned long short_time, unsigned long long_time) {
    right_button = right_button_pin;
    left_button = left_button_pin;

    short_press = short_time;
    long_press = long_time;

    toggle_flag = false;

    NIRS_state = power_off;
    event_counter = 0;
}

bool NIRS::init() {
    pinMode(right_button, OUTPUT);
    pinMode(left_button, OUTPUT);

    digitalWrite(right_button, LOW);
    digitalWrite(left_button, LOW);  

    return true; 
}

void NIRS::blocking_press(uint8_t pin, int time) {
    digitalWrite(pin, HIGH);
    delay(time);
    digitalWrite(pin, LOW);
}

void NIRS::turn_on() {
    if (NIRS_state == power_off) {
        blocking_press(left_button, long_press);
        NIRS_state = power_on;
    }
}

void NIRS::turn_off() {
    if (NIRS_state != power_off) {
        blocking_press(left_button, long_press);
        NIRS_state = power_off;
    }
}

void NIRS::begin() {
    if (NIRS_state == power_on) {
        blocking_press(right_button, long_press);
        NIRS_state = recording;
    }
}

void NIRS::end() {
    if (NIRS_state == recording) {
        blocking_press(right_button, long_press);
        NIRS_state = power_on;
    }
}

void NIRS::start_event() {
    nirs_timer = 0;
    digitalWriteFast(left_button, HIGH);
    toggle_flag = true;
}

void NIRS::update_event() {
    if (nirs_timer > event_period) {
        nirs_timer = 0;
        digitalWriteFast(left_button, HIGH);
        toggle_flag = true;
    }
    else if (nirs_timer > press_duration) {
        if (toggle_flag) {
            digitalWriteFast(left_button, LOW);
            toggle_flag = false;
        }
    }
}