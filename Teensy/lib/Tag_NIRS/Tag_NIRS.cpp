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

    logger.print_message("NIRS Initalized");

    return true; 
}

void NIRS::blocking_press(uint8_t pin, int time) {
    digitalWrite(pin, HIGH);
    delay(time);
    digitalWrite(pin, LOW);
}

void NIRS::turn_on() {
    blocking_press(left_button, long_press);
    logger.print_message("ATTEMPTED NIRS POWER ON");
}

void NIRS::turn_off() {
    blocking_press(left_button, long_press);
    logger.print_message("ATTEMPTED NIRS POWER OFF");
}

void NIRS::begin() {
    blocking_press(right_button, long_press);
    logger.print_message("NIRS SAMPLING STARTED");
    start_event(10000);  //starts a series of events every 10 seconds
}

void NIRS::end() {
    end_event();
    blocking_press(right_button, long_press);
    
    // reset the pin state to LOW
    digitalWrite(left_button, LOW);
    digitalWrite(right_button, LOW);

    nirs_timer = 0;
    button_timer = 0;
    toggle_flag = false;

    logger.print_message("NIRS SAMPLING STOPPED");
}

void NIRS::start_event(unsigned long period) {
    create_event_timer(short_press, period);
    event_half_press();
    logger.print_message("Created event");
}

void NIRS::end_event() {
    event_period = 0;
    logger.print_message("Ended events");
}

void NIRS::update_event() {
    if (toggle_flag) {//button state was pressed, need to bring back to LOW after a short button press
        if (button_timer > press_duration) {
            digitalWriteFast(left_button, LOW);
            toggle_flag = false;
            logger.print_variable("Stopped button press", millis());
        }
    }

    if ((nirs_timer > event_period) & (event_period > 0)) {
        event_half_press();
    }

}

void NIRS::event_half_press() {
    digitalWriteFast(left_button, HIGH);
    button_timer = 0;
    nirs_timer = 0;
    toggle_flag = true;
    logger.print_variable("Started button press", millis());
}