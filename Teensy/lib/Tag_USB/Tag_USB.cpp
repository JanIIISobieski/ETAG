#include "Tag_USB.h"

Tag_USB::Tag_USB(usb_serial_class* serial, DiskManager* diskManagerPtr) : SerialCommunicator(serial, diskManagerPtr) {
    hard_serial = serial;
}

void Tag_USB::init(uint32_t baud_rate) {
    hard_serial->begin(baud_rate);
}

bool Tag_USB::file_send(String file_name) {
    uint64_t bytes_written = 0;
    int bytes_to_send = 0;
    size_t buffer_position = 0;

    union file_size {
        uint64_t as_int;
        uint8_t as_array[8];
    } size_of_file;

    _diskManagerPtr->open_data_file(file_name);
    size_of_file.as_int = _diskManagerPtr->get_file_size();
    write(size_of_file.as_array, 8);

    logger.print_variable("File Size", size_of_file.as_int);

    await_acknowledgment();

    logger.print_message("ACK Recieved");

    while (bytes_written < size_of_file.as_int) { // Start while loop
        FileReadData* read_data = _diskManagerPtr->get_file_data();                                                                                               // This prevents cases where the file was opened/closed unsuccessfully
        if (read_data->bytes_read == -1) {
            if (!_diskManagerPtr->reset()) {
                break;
            }
        }
        else {
            buffer_position = 0;
            bytes_to_send = read_data->bytes_read;
            logger.print_variable("bytes_to_send", bytes_to_send);
            while (bytes_to_send > 0) {  // we read the file correctly, now just send it over the serial
                size_t num_to_write = (bytes_to_send > USB_BUFFER_SIZE) ? USB_BUFFER_SIZE : bytes_to_send;
                logger.print_variable("num_to_write", num_to_write);
                size_t num_written = write(read_data->data, num_to_write);
                if (num_written != num_to_write) logger.print_message("-----------OOPS-------------");
                bytes_to_send -= num_written;
                bytes_written += num_written;
                buffer_position += num_written;
                logger.print_variable("bytes_to_send", bytes_to_send);
            }
        }
    }// end while loop

    _diskManagerPtr->close_file();

    return (bytes_written == size_of_file.as_int);
}