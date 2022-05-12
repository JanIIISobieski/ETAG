#include "ESP32_Bluetooth.h"

ESP32_Bluetooth::ESP32_Bluetooth(HardwareSerial* bt_serial, DiskManager* diskManagerPtr) : SerialCommunicator(bt_serial, diskManagerPtr) {
    hard_serial = bt_serial;
    chipState = COMMAND;
    is_sampling = false;
}

void ESP32_Bluetooth::init(uint32_t baud_rate) {
    hard_serial->begin(baud_rate);

    while (!hard_serial) {
        delay(50);
    }
}

bool ESP32_Bluetooth::file_send(String file_name) {
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
                size_t num_to_write = (bytes_to_send > BLUETOOTH_BUFFER_SIZE) ? BLUETOOTH_BUFFER_SIZE : bytes_to_send;
                logger.print_variable("num_to_write", num_to_write);
                size_t num_written = write(read_data->data + buffer_position, num_to_write);
                if (num_written != num_to_write) logger.print_message("Wrong number of bytes written");
                bytes_to_send -= num_written;
                bytes_written += num_written;
                buffer_position += num_written;
                logger.print_variable("bytes_to_send", bytes_to_send);
                await_acknowledgment();
            }
        }
    }// end while loop

    _diskManagerPtr->close_file();

    return (bytes_written == size_of_file.as_int);
}

bool ESP32_Bluetooth::sample_pressure_temperature() {
    if (is_sampling & (chipState == COMMAND)) {
        logger.print_message("PS");
        write('p');

        int read_val = read();
        logger.print_variable("Read val", read_val);

        if ((read_val == -1) || ((uint8_t)(read_val & 0xFF) == 0) ) {
            return false;
        } else {
            pressure_temp_vals.tpt_struct.time = micros();
            read_type(pressure_temp_vals.tpt_struct.pressure);
            read_type(pressure_temp_vals.tpt_struct.temperature);
            return true;
        }
    } else {
        return false;
    }
}

bool ESP32_Bluetooth::sample_speed() {
    if (is_sampling & (chipState == COMMAND)) {
        logger.print_message("SS");
        write('v');

        int read_val = read();
        logger.print_variable("Read val", read_val);

        if ((read_val == -1) || ((uint8_t)(read_val & 0xFF) == 0) ) {
            return false;
        } else {
            time_speed.st_struct.time = micros();
            read_type(time_speed.st_struct.speed);
            return true;
        }
    } else {
        return false;
    }
}

int32_t ESP32_Bluetooth::sample_saltwater_sensor() {
    if (chipState == COMMAND) {
        write('z');
        read_type(saltwater_val);
        return saltwater_val.as_type;
    } else {
        return -1;
    }
}

void ESP32_Bluetooth::enable_saltwater_sensor() {
    if (chipState == COMMAND) {
        logger.print_message("Enabling saltwater sensor");
        write('k');
    }
}

void ESP32_Bluetooth::disable_saltwater_sensor() {
    if (chipState == COMMAND) {
        logger.print_message("Disabling saltwater sensor");
        write('l');
    }
}

void ESP32_Bluetooth::disable_release() {
    if (chipState == COMMAND) {
        logger.print_message("Disabling release");
        write('e');
    }
}

void ESP32_Bluetooth::enable_release() {
    if (chipState == COMMAND) {
        logger.print_message("Enabling release");
        write('a');
    }
}

void ESP32_Bluetooth::reset_pressure() {
    if (chipState == COMMAND) {
        logger.print_message("Resetting pressure sensor");
        write('r');
    }
}

void ESP32_Bluetooth::set_WiFi_mode() {
    logger.print_message("Setting WiFi mode");
    if (chipState == PASSTHROUGH) {
        logger.print_message("Writing phrase");
        write(phrase, PHRASE_LEN);
    }
    write('w');
    chipState = COMMAND;
}

void ESP32_Bluetooth::set_Bluetooth_mode() {
    logger.print_message("Setting BlueTooth mode");
    write('y');
    chipState = PASSTHROUGH;
}

void ESP32_Bluetooth::turn_off_comms() {
    logger.print_message("Disabling comms");
    if (chipState == PASSTHROUGH) {
        logger.print_message("Writing phrase");
        write(phrase, PHRASE_LEN);
    }
    write('o');
    chipState = COMMAND;
}