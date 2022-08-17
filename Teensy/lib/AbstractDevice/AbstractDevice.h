#pragma once

/**
 * @brief Sets the interface for each Device
 * 
 * Each function is a pure virtual function, and thus will need to be overwritten by any class that inherits from this one
 */
class AbstractDevice {
    public:
        /**
         * @brief Defines the initalizer for the device
         * 
         * @return true for success or false for failure
         * 
         * This initializer is called from DeviceManager.init(), which runs when \ref initalize_devices() is called
         */
        virtual bool init() = 0;

        /**
         * @brief Defines the begin sampling methods for the device
         * 
         * This function is to set the device to be ready for sampling
         */
        virtual void begin() = 0;

        /**
         * @brief Defines the end sampling methods for the device
         *
         * This function is to stop sampling and power down the device 
         */
        virtual void end() = 0;
};