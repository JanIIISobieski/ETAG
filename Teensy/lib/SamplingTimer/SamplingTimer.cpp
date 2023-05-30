#include "SamplingTimer.h"

SamplingTimer::SamplingTimer() {
    is_sampling = false;
    sampling_end_time = MAX_TIME;
    sampling_time = 0;
}

void SamplingTimer::begin() {
    sampling_time = 0;
    is_sampling = true;
}

void SamplingTimer::end() {
    is_sampling = false;
}

void SamplingTimer::set_maximum_time(size_t max_time) {
    sampling_end_time = max_time;
}

bool SamplingTimer::check_sampling_time() {
    return is_sampling & (sampling_time > sampling_end_time);
}