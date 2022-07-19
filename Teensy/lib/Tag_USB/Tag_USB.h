#ifndef GUARD_TAG_USB
#define GUARD_TAG_USB

#include <Arduino.h>
#include <SerialCommunicator.h>

#define USB_BUFFER_SIZE       512 // Size of the USB send buffer
#define LOGGER_OUTPUT_BUFFER  60  // Size of buffer used for sprintf

#define USB_BAUD_RATE         115200 // Needed to pass to serial.begin(), but the USB will run full speed

class Tag_USB : public SerialCommunicator {
    public:
        Tag_USB(usb_serial_class* serial, DiskManager* diskManagerPtr);
        ~Tag_USB() {};

        bool init();
        void begin() {};
        void end() {};

        bool file_send(String file_name);
        
    private:
        usb_serial_class* hard_serial;
};

#endif //GUARD_TAG_USB