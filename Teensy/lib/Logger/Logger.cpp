#include "Logger.h"

void Logger::assign_metadata(TimingData* data, String name) {
    timing_list.insert(data);
    metadata_names.insert(name);
}

void Logger::log_timing_metadata() {
    for (size_t i = 0; i < timing_list.get_length(); i++) {
        get_average(timing_list.get(i));
        print_timing_metadata(metadata_names.get(i), timing_list.get(i));
    }
}

void Logger::reset_metadata() {
    for (size_t i = 0; i < timing_list.get_length(); i++) {
        TimingData* data = timing_list.get(i);
        data->counter = 0;
        data->min_diff = 0;
        data->max_diff = 0;
        data->avg_diff = 0.0;
        data->prior_time = 0;
        data->first_trigger = 0;
        data->was_triggered = false;
    }
}

void Logger::get_average(TimingData* metadata) {
    metadata->avg_diff /= (float)(metadata->counter - 1);
}

void Logger::update_timing_data(TimingData& data) {
    __disable_irq();  //get rid of interrupts to ensure the timing is correct here
    uint32_t current_time = micros();
    uint32_t diff = current_time - data.prior_time;
    data.counter += 1;
    if (data.prior_time != 0) {
        data.min_diff = (diff < data.min_diff) ? diff : data.min_diff;
        data.max_diff = (diff > data.max_diff) ? diff : data.max_diff;
        data.avg_diff += (float)diff;
    }
    if (!data.was_triggered) {
        data.first_trigger = current_time;
        data.was_triggered = true;
    }
    data.prior_time = current_time;
    __enable_irq();
}
