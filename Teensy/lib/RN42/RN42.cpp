#include "RN42.h"

RN42_Bluetooth::RN42_Bluetooth(uint8_t bt_master, uint8_t bt_discovery, uint8_t bt_RTS, uint8_t bt_CTS, HardwareSerial* bt_serial, DiskManager* diskManagerPtr) :
    SerialCommunicator(bt_serial, diskManagerPtr) {
    master = bt_master;
    discovery = bt_discovery;
    CTS = bt_RTS;   //From the point of view of the Bluetooth module, this will be the CTS line on the Teensy (Bluetooth RTS -> Teensy CTS)
    RTS = bt_CTS;   //From the point of view of the Bluetooth module, this will be the RTS line on the Teensy (Teensy CTS -> Bluetooth RTS)

    hard_serial = bt_serial;
}

void RN42_Bluetooth::begin(uint32_t baud_rate) {
    pinMode(master, OUTPUT);
    pinMode(discovery, OUTPUT);
    pinMode(CTS, OUTPUT);

    hard_serial->begin(baud_rate);

    //This must be called after begin. This also internally set the pin state as INPUT.
    //In theory, the Bluetooth module can tell the Teensy when the Teensy is not cleared to send more data.
    hard_serial->attachRts(RTS);

    while(!hard_serial) {
        delay(50);
    }
}

void RN42_Bluetooth::set_discovery_pin(uint8_t val) {
    digitalWrite(discovery, val);
}

void RN42_Bluetooth::set_master_pin(uint8_t val) {
    digitalWrite(master, val);
}

void RN42_Bluetooth::set_CTS_pin(uint8_t val) {
    digitalWrite(CTS, val);
}

void RN42_Bluetooth::set_RTS_pin(uint8_t val) {
    digitalWrite(RTS, val);
}

void RN42_Bluetooth::init(uint32_t baud_rate) {
    begin(baud_rate);

    set_master_pin(LOW);
    set_discovery_pin(HIGH);
}

bool RN42_Bluetooth::file_send(String file_name) {
    return true;
    /*
    uint64_t bytes_written = 0;
    uint8_t file_buffer[FILE_READ_SIZE] = {0};

    uint64_t cursor_position = 0;
    uint64_t file_position = 0;

    size_t try_counter = 0;
    size_t buffer_position = 0;
    bool break_flag = false;

    union file_size {
        uint64_t as_int;
        uint8_t as_array[8];
    } size_of_file;

    if (file.open(file_name, O_READ)) logger.print_message("File opened succesfully");
    else logger.print_message("Opening file failed");

    size_of_file.as_int = file.size();
    write(size_of_file.as_array, 8);  //rather than writing a method to send a uint64_t, send an array of bytes instead, using write(buffer_ptr, buffer_length)
    
    await_acknowledgment();

    logger.print_message("ACK Recieved");

    while (bytes_written < size_of_file.as_int) {
        file_position = file.curPosition();  // get the current position in the file
        cursor_position = (cursor_position > file_position) ? cursor_position : file_position;  // if the current position in the file is greater than the cursor,
                                                                                                // set the cursor equal to the file position
                                                                                                // This prevents cases where the file was opened/closed unsuccessfully
                                                                                                // resulting in opening the file in the beginning, as now cursor_position
                                                                                                // contains the index from which reading should begin

        int bytes_read = file.read(file_buffer, FILE_READ_SIZE);
        if (bytes_read == -1) {  // file threw an error, we need to restart the SD card and try again
            if (file.close()) logger.print_message("File closed successfully");
            else logger.print_message("Could not close file");

            while (!sd.begin(SdioConfig(FIFO_SDIO))) {
                ++try_counter;
                logger.print_message("Trying to restart SD card");
                logger.print_variable("Try Counter", try_counter);

                if (try_counter > 3) {  // we tried to read from the file 3 times, but still didn't work, error out
                    logger.print_message("Too many read failures");
                    break_flag = true;
                    break;  // get out of the restart SD card while loop (inner-most while)
                }
            }

            if (break_flag) break;  // break out of the file-writing file loop (outer-most while)

            if (file.open(file_name)) logger.print_message("File reopened");
            else logger.print_message("Could not open file");

            if (file.seekSet(cursor_position)) {
                char position_buffer[30];
                sprintf(position_buffer, "Opened file at: %llu", cursor_position);
                logger.print_message(position_buffer);
            }
            else logger.print_message("Could not reach file position");
        }
        else {  // we read the file correctly, now just send it over the serial
            buffer_position = 0;
            while (bytes_read > 0) {
                size_t num_to_write = (bytes_read > BLUETOOTH_BUFFER_SIZE) ? BLUETOOTH_BUFFER_SIZE : bytes_read;
                size_t num_written = write(&file_buffer[buffer_position], num_to_write);
                buffer_position += num_written;
                bytes_written += num_written;
                bytes_read -= num_written;
                try_counter = 0;
                await_acknowledgment();  // avoid sending too much data to the buffer
            }
        }
    }
    
    if (file.close()) logger.print_message("File closed successfully");
    else logger.print_message("File could not be closed");
    
    return (bytes_written == size_of_file.as_int);
    */
}
