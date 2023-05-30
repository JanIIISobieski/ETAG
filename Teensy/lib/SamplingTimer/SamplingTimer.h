#pragma once

#include <Arduino.h>
#include <elapsedMillis.h>

#define MAX_TIME (4*60*60*1000)  //no more than 4 hours of sampling

class SamplingTimer {
    public:
        SamplingTimer();
        bool check_sampling_time();
        void set_maximum_time(size_t max_time); //max sampling time in milliseconds
        void begin();
        void end();


    private:
        elapsedMillis sampling_time;
        size_t sampling_end_time;
        bool is_sampling;
};