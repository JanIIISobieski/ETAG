#include <Arming.h>

Arming::Arming(/* args */) {
    state = NOT_INITIALIZED;

    success_counter = 0;
    success_count_to_start = 10;
    threshold = 128;
    read_val.as_type = 0;
}

Arming::~Arming() {
}

void Arming::arm() {
    TagComms.write(0, static_cast<uint8_t>('k')); // Enables the saltwater sensor on ESP32 co-board
    state = ARMED;
    logger.print_message("Tag is armed");
}

void Arming::disarm() {
    TagComms.write(0, static_cast<uint8_t>('l'));  // Disables the saltwater sensor on ESP32 co-board
    state = NOT_INITIALIZED;
    logger.print_message("Tag is disarmed");
}

bool Arming::check_trigger() {
    // Make sure tag is armed if we are going to check the trigger
    if (state != ARMED) return false;
    
    success_counter = 0; //reset the success counter

    // Now that we know the tag is armed, we will be trying to check for the values coming from the ESP32
    while (success_counter < success_count_to_start) {
        TagComms.write(0, static_cast<uint8_t>('z')); // requests saltwater sensor value
        TagComms.read_type(read_val);                 // reads the returned saltwater sensor value

        if (read_val.as_type > threshold) return false;  // value read from tag is not under threshold, therefore the reading was not a success and we should end it
        
        ++success_counter;
        delay(10);
    }

    state = TRIGGERED;
    logger.print_message("Tag has been triggered");
    return true;
}