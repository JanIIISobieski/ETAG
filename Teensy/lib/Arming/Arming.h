#pragma once

#include <Arduino.h>
#include <TagCommunicator.h>
extern TagCommunicator TagComms;

#include <Logger.h>
extern Logger logger;

enum ArmingState {
    NOT_INITIALIZED = 0,
    ARMED          = 1,
    TRIGGERED      = 2
};

class Arming {
private:
    ArmingState state;
    size_t success_counter;
    size_t success_count_to_start;
    size_t threshold;
    ByteArray<uint16_t> read_val;
    bool has_triggered_light;

public:
    Arming(/* args */);
    ~Arming();

    void arm();
    void disarm();
    bool check_trigger();
};