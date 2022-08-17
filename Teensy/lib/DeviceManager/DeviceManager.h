#pragma once

#include <Arduino.h>
#include "AbstractDevice.h"

/**
 * @brief This class manages all of the sampling devices
 * 
 * This class calls each device's init(), while begin(), end() only if the corresponding \ref device_start boolean is true.
 * This allows for only starting select devices. This also results in shorter code for the main function, as only this
 * class's methods must be called to maintain all of the devices. All chips are initalized no matter the state of the bool array.
 * 
 */
class DeviceManager {
    private:
        size_t length;
        AbstractDevice** device_ptr;
        bool* device_start;
    
    public:
        /**
         * @brief Construct a new Device Manager object
         * 
         * @param ptr A pointer to an array of pointers to AbstractDevices
         * @param device_bool A pointer to an array of booleans of the same length as the array pointed to by \ref ptr, true indicating to sample the device, false indicating to not sample
         * @param len The lengths of the arrays pointed to by \ref ptr and \ref device_bool
         */
        DeviceManager(AbstractDevice** ptr, bool* device_bool, size_t len);

        /**
         * @brief Calls each device's init() method
         */
        void initialize_devices();
        
        /**
         * @brief Calls the begin() methods of each device that has a true in the bool array
         */
        void begin_sampling();

        /**
         * @brief Calls the end() methods of each device that has a true in the bool array
         */
        void end_sampling();
};