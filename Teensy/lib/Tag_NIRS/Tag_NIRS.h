#ifndef GUARD_TAG_NIRS
#define GUARD_TAG_NIRS

#include <Arduino.h>

#include "AbstractDevice.h"
#include "Logger.h"
extern Logger logger;

/**
 * @brief Track the NIRS system state
 * 
 * Note that the NIRS system functions in parallel to the tag. The tag merely emulates button presses.
 * Thus, to use the Artinis NIRS system, a laptop containing the NIRS software will have to be used to interface with the system
 * for setting sampling parameters. The tag controls can merely turn the device on or off, start/stop sampling, and create event timers
 * (that can then be used to align the data with the tag).
 */
enum NIRS_state {
    power_off = 0,
    power_on = 1,
    recording = 2
};

/**
 * @brief Implements the NIRS control
 * 
 */
class NIRS : public AbstractDevice {
    public:
        /**
         * @brief Construct a new NIRS object
         * 
         * @param left_button_pin Left button pin
         * @param right_button_pin Right button pin
         * @param short_time The duration of a short button press
         * @param long_time The duration of a long button press
         */
        NIRS(uint8_t left_button_pin, uint8_t right_button_pin, unsigned long short_time, unsigned long long_time);

        /**
         * @brief Turn on the NIRS system 
         */
        void turn_on();

        /**
         * @brief Turn off the NIRS system
         */
        void turn_off();

        /**
         * @brief Initalize the pins
         */
        bool init();

        /**
         * @brief Begin the NIRS sampling
         */
        void begin();

        /**
         * @brief End the NIRS sampling
         */
        void end();

        /**
         * @brief Create a event timer object, setting up how long to press the button and what the period of the button presses is
         * 
         * @param duration Duration of press
         * @param period Period between presses
         */
        void create_event_timer(unsigned long duration, unsigned long period) { press_duration = duration; event_period = period; };

        /**
         * @brief Ends the event timer by setting the event_period to 0
         */
        void end_event();

        /**
         * @brief Start the button press for an event with a period between the events
         * 
         * @param period The duration of time between button presses
         */
        void start_event(unsigned long period);

        /**
         * @brief Function to check if the pin state has to be updated
         * 
         * The button press occurs at regular intervals only if the event timer is greater than 0.
         * 
         */
        void update_event();

    private:
        elapsedMillis button_timer;
        elapsedMillis nirs_timer;

        uint8_t right_button;
        uint8_t left_button;

        int long_press = 3000;
        int short_press = 1000;

        unsigned long press_duration;
        unsigned long event_period;
        bool toggle_flag;

        uint8_t NIRS_state;

        void blocking_press(uint8_t pin, int time);
        void event_half_press();

        unsigned long int event_counter;
};

#endif //GUARD_TAG_NIRS