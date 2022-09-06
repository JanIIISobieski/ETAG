#include "initialize_variables.h"

void setup() {
    SPI.begin();
    MICRO_USB.init();
    Tag_Bluetooth.init();
    SerialUSB1.begin(9600);

#ifdef ETAG_DEBUG
    logger.assign_metadata(&hydrophone_buffer_push, "Hydrophone Push");
    logger.assign_metadata(&eeg_buffer_push, "EEG Push");
    logger.assign_metadata(&imu_buffer_push, "IMU Buffer Push");
    logger.assign_metadata(&pressure_buffer_push, "Pressure Push");
    logger.assign_metadata(&speed_buffer_push, "Speed Push");
    logger.assign_metadata(&sampling_loop, "Loop() Timing");
    stopOnCompletionTimer.dt = 1000;
#endif
}

void loop() {
#ifdef ETAG_DEBUG
    logger.update_timing_data(sampling_loop);
#endif
    read_val = TagComms.check_for_commands();
    sampling();
    if (read_val != -1) {
        command = (char)(read_val & 0xFF);
        if (command == 'b') begin_sampling();
        else if (command == 's') stop_sampling();
        else if (command == 't') test_sampling();
        else if (command == 'd') download_files();
        else if (command == 'f') update_files();
        else if (command == 'u') update_parameters();
        else if (command == 'c') sync_time();
        else if (command == 'i') initalize_devices();
        else if (command == 'x') delete_file();
        else if (command == '+') nirs.turn_on();
        else if (command == '-') nirs.turn_off();
        else if (command == 'o') Tag_Bluetooth.turn_off_comms();
        else if (command == 'w') Tag_Bluetooth.set_WiFi_mode();
        else if (command == 'y') Tag_Bluetooth.set_Bluetooth_mode();
        else if (command == 'L') forward_ESP32_logging();
        else logger.print_variable("Recieved byte but don't know what to do with it", read_val);
        read_val = -1;
    }
}

inline void sampling() {
    if (IMU.sample()) {
        imu_buffer.write(IMU.buffer, 9);
    }

    if (Tag_Bluetooth.sample_pressure_temperature()) {
        pressure_buffer.write(Tag_Bluetooth.get_pressure_temperature(), 12);
    }

    if (Tag_Bluetooth.sample_speed()) {
        speed_buffer.write(Tag_Bluetooth.get_speed(), 8);
    }

    nirs.update_event();

    if (queue.num_to_write() > 0) {
        bytes_written = WriteManager.write_data((void *)queue.dequeue(), 8192);
        logger.print_memory("Dequeued", (void *) queue.get_popped());
        if (bytes_written != 8192) {
            logger.print_variable("Incorrect number of bytes written", bytes_written);
            logger.print_SD_error(DiskManager::sd);
        }
    }
}

void begin_sampling() {
    ByteArray<uint8_t> writer_ind;
    TagComms.read_type(writer_ind);  //this read is blocking, unlike read() which will return -1 if there is no byte ready

    logger.print_message("Recieved Write Index");

    WriteManager.select_writer(writer_ind.as_type);
    
    run_data.update_animal_name(TagComms.readStringUntil('|', 120U));
    run_data.update_animal_species(TagComms.readStringUntil('|', 120U));
    run_data.update_description(TagComms.readStringUntil('|', 120U));

    run_data.update_datetime();
    run_data.update_imu_calibration(IMU);

    logger.print_message("Updated run data");

    logger.reset_metadata();
    queue.reset();
    buffer_manager.reset();

    logger.print_message("Successful buffer reset");

    bytes_written = 0;

#ifdef ETAG_DEBUG
    sampling_timer = 0;
#endif
    logger.print_DeviceEnable_settings("Device Enable", device_settings);
    logger.print_ADC_settings("Hydrophone Settings", adc_settings);

    logger.print_buffer_headers("Hydrophone", hydrophone_buffer.get_buffers(), HYDROPHONE_BUFF_NUM);
    logger.print_buffer_headers("IMU", imu_buffer.get_buffers(), IMU_BUFF_NUM);
    logger.print_buffer_headers("EEG", eeg_buffer.get_buffers(), EEG_BUFFER_NUM);
    logger.print_buffer_headers("PRESSURE", pressure_buffer.get_buffers(), PRESSURE_BUFFER_NUM);
    logger.print_buffer_headers("SPEED", speed_buffer.get_buffers(), SPEED_BUFFER_NUM);

    WriteManager.pre_sampling_setup();
    WriteManager.write_header();
    deviceManager.begin_sampling();
    eeg_buffer.init();
}

void stop_sampling() {
    deviceManager.end_sampling();
    WriteManager.post_sampling_conclude();
#ifdef ETAG_DEBUG
    logger.print_variable("Sampling Time", sampling_timer);
    logger.log_timing_metadata();
    logger.print_variable("Length Linked List", logger.get_length());
    logger.print_buffer_headers("Hydrophone", hydrophone_buffer.get_buffers(), HYDROPHONE_BUFF_NUM);
    logger.print_buffer_headers("IMU", imu_buffer.get_buffers(), IMU_BUFF_NUM);
    logger.print_buffer_headers("EEG", eeg_buffer.get_buffers(), EEG_BUFFER_NUM);
    logger.print_array("EEG Sample", eeg_buffer.get_buffer(), 32);
    logger.print_array("IMU Sample", IMU.buffer, 9);
#endif
    eeg_buffer.reset();
}

void sync_time() {
    TagComms.write(ACK);

    ByteArray<uint32_t> time_buffer;
    TagComms.read_type(time_buffer);

    setTime(time_buffer.as_type);
    rtc_set(now());

    logger.print_variable("Set Teensy Time", time_buffer.as_type);
}

void test_sampling() {
    delay(1);
}

void update_files() {
    TagComms.write(ACK);

    uint8_t num_files = diskManager.update_folder_num();

    for (uint8_t i = 0; i < num_files; i++) {
        String folder_name = diskManager.get_filename(i);
        TagComms.println(folder_name);
        delay(10);
    }
    TagComms.print(">");
}

void download_files() {
    TagComms.write(ACK);

    ByteArray<uint8_t> file_index;
    TagComms.read_type(file_index);

    TagComms.send_file(diskManager.get_filename(file_index.as_type));
}

void delete_file() {
    TagComms.write(ACK);
    
    ByteArray<uint8_t> file_index;
    TagComms.read_type(file_index);

    diskManager.delete_file(file_index.as_type);

    TagComms.write(ACK);
}

void update_parameters() {
    logger.print_message("Updating params");
    TagComms.read(parent_eeg_settings.raw_bytes, sizeof(ADS1299Settings));
    TagComms.read(adc_settings.raw_bytes, sizeof(ADCSettings));
    TagComms.read((uint8_t*)device_settings.raw_bytes, sizeof(DeviceEnable));

    logger.print_ADS1299_settings("ADS Main Settings", parent_eeg_settings);
    logger.print_ADC_settings("Hydrophone Settings", adc_settings);
    logger.print_DeviceEnable_settings("Device Setting", device_settings);
}

void initalize_devices() {
    diskManager.init();
    deviceManager.initialize_devices();

    logger.print_ADC_settings("ADC Settings", adc_settings);
    logger.print_DeviceEnable_settings("Device Enable Settings", device_settings);
}

inline void forward_ESP32_logging() {
    logger.print_message(TagComms.readStringUntil('\n', 120U));  //logging messages are of the form "L[...Message...]\n". We want to capture just the ...Message..., so readline until newline is perfect
}