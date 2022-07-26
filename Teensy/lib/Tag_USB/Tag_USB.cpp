#include "Tag_USB.h"

Tag_USB::Tag_USB(usb_serial_class* serial, DiskManager* diskManagerPtr, RunData* run_data) : SerialCommunicator(serial, diskManagerPtr), Writer(run_data) {
    hard_serial = serial;
}

bool Tag_USB::init() {
    hard_serial->begin(USB_BAUD_RATE);
    return true;
}

size_t Tag_USB::write_data(void* buff_ptr, size_t num_bytes) {
    return write(reinterpret_cast<uint8_t*>(buff_ptr), num_bytes);
}

size_t Tag_USB::write_header() {
    size_t header_size = 0;

    create_data_header(); // from Writer.h
    header_size += serializeJson(header_info, (*hard_serial));  //hard_serial is a pointer, we need to pass the actual object
    header_size += hard_serial->print("\n");

    return header_size;
}

bool Tag_USB::file_send(String file_name) {
    uint64_t bytes_written = 0;
    int num_bytes_to_send = 0;
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

    while (bytes_written < size_of_file.as_int) {
        FileReadData* read_data = _diskManagerPtr->get_file_data();                                                                                               // This prevents cases where the file was opened/closed unsuccessfully
        if (read_data->bytes_read == -1) {  // if the read failed, then reset the SD card
            if (!_diskManagerPtr->reset()) {
                break;
            }
        }
        else {  // successful read, we want to send the data
            buffer_position = 0;
            num_bytes_to_send = read_data->bytes_read;
            logger.print_variable("bytes_to_send", num_bytes_to_send);
            while (num_bytes_to_send > 0) {  // we read the file correctly, now just send it over the serial
                //Do not want to overwhelm the serial nor the SD, we can read a large amount from the SD card, and send in smaller chunks over the USB
                size_t num_to_write = (num_bytes_to_send > USB_BUFFER_SIZE) ? USB_BUFFER_SIZE : num_bytes_to_send;
                size_t num_written = write(read_data->data + buffer_position, num_to_write);
                
                if (num_written != num_to_write) logger.print_message("Wrong number of bytes written to USB from buffer");

                num_bytes_to_send -= num_written;  //how many more bytes left from the read SD card chunk do we have to send
                bytes_written += num_written;      //how many bytes in total have we sent from the file
                buffer_position += num_written;    //increment the position in the buffer with the read SD card chunk so that we send the whole buffer across

                logger.print_variable("bytes_to_send", num_bytes_to_send);
            }  // we finished sending the SD card chunk we just read, we can go back and read a new chunk to send if there are still bytes remaining to be written
        }
    }// end while (bytes_written < size_of_file.as_int) loop.

    _diskManagerPtr->close_file();

    return (bytes_written == size_of_file.as_int);
}