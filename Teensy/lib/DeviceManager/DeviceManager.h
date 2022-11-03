#pragma once

#include <Arduino.h>
#include "AbstractDevice.h"

#include "Logger.h"
extern Logger logger;

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
         * 
         * This method starts from the start of the AbstractDevice* array, beginning with the methods that take the longest to initialize
         * for starting sampling. NIRS is a blocking button press, and is thus initialized first.
         * Interrupt based methods are initialized closer to the end (hydrophone and eeg).
         * The Pressure sensor which merely toggles a boolean is initialized last.
         */
        void begin_sampling();

        /**
         * @brief Calls the end() methods of each device that has a true in the bool array
         * 
         * This array is iterated from the end of the AbstractDevice* array to the beginning. This way, the methods which can keep sending data
         * (i.e. interrupt based routines) will be stopped first, before a blocking button press on the NIRS will be called (otherwise this would)
         * result in multiple different buffers to be pushed to the \ref Tag_Queue as the blocking press is happening and the interrupt routines are
         * still sampling in the background.
         */
        void end_sampling();
};