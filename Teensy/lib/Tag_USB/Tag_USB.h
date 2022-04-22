#ifndef GUARD_TAG_USB
#define GUARD_TAG_USB

#include <Arduino.h>
#include <SerialCommunicator.h>

#define USB_BUFFER_SIZE 512      // Size of the USB send buffer
#define LOGGER_OUTPUT_BUFFER 60 // Size of buffer used for sprintf

class Tag_USB : public SerialCommunicator {
    public:
        Tag_USB(usb_serial_class* serial, DiskManager* diskManagerPtr);
        ~Tag_USB() {};

        void init(uint32_t baud_rate);
        bool file_send(String file_name);
        
    private:
        usb_serial_class* hard_serial;
};

#endif //GUARD_TAG_USB