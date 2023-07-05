#pragma once

#include <Arduino.h>
#include <elapsedMillis.h>

//#define MAX_TIME (4*60*60*1000)  //no more than 4 hours of sampling, this time is in milliseconds, MAKE SURE THIS COPY GETS UPLOADED UPON DELIVERY
#define MAX_TIME (10*1000)        //no more than 10 seconds of sampling, USE THIS FOR TESTING, COMMENT LINES WITH A // at the beginning of line

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