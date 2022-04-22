#include "DeviceManager.h"

DeviceManager::DeviceManager(AbstractDevice** ptr, bool* device_bool, size_t len) {
    device_ptr = ptr;
    length = len;
    device_start = device_bool;
}

void DeviceManager::initialize_devices() {
    for (size_t i = 0; i < length; i++) {
        if (device_start[i]) (*(device_ptr + i))->init();  //initalize all devices to their default state
    }
}

void DeviceManager::begin_sampling() {
    for (size_t i = 0; i < length; i++) {
        if (device_start[i]) (*(device_ptr + i))->begin();
    }
}

void DeviceManager::end_sampling() {
    for (size_t i = 0; i < length; i++) {
        if (device_start[i]) (*(device_ptr + i))->end();
    }
}