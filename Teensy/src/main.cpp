#include "initialize_variables.h"

void setup() {
    SPI.begin();
    TagComms.init();

#ifdef EEG_TIMING
    pinMode(TTL_INT_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(TTL_INT_PIN), get_timing, RISING);
#endif

#ifdef ETAG_DEBUG
    logger.assign_metadata(&hydrophone_buffer_push, "Hydrophone Push");
    logger.assign_metadata(&eeg_buffer_push, "EEG Push");
    logger.assign_metadata(&imu_buffer_push, "IMU Buffer Push");
    logger.assign_metadata(&pressure_buffer_push, "Pressure Push");
    logger.assign_metadata(&speed_buffer_push, "Speed Push");
    logger.assign_metadata(&sampling_loop, "Loop() Timing");

    stopOnCompletionTimer.dt = 1000;

    SerialUSB1.begin(9600);
    while (!SerialUSB1) {  //wait to open Debug file
        delay(100);
        if (millis() > 10000) break; //wait for 10 seconds before moving after program start to move on
    }
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
        else if (command == '?') dummy_file();
        else if (command == 'o') Tag_Bluetooth.turn_off_comms();
        else if (command == 'w') Tag_Bluetooth.set_WiFi_mode();
        else if (command == 'y') Tag_Bluetooth.set_Bluetooth_mode();
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

    //nirs.update_event();

    if (queue.num_to_write() > 0) {
        bytes_written = diskManager.write_to_file((void *)queue.dequeue(), 8192);
        logger.print_memory("Dequeued", (void *) queue.get_popped());
        if (bytes_written != 8192) {
            logger.print_variable("Incorrect number of bytes written", bytes_written);
            logger.print_SD_error(DiskManager::sd);
        }
    }
}

void begin_sampling() {
#ifdef EEG_TIMING
    ttl_ind = 0;
    init_vector(eeg_timing_info, TTL_INT_LENGTH);
#endif

    logger.reset_metadata();

    queue.reset();
    hydrophone_buffer.reset();
    imu_buffer.reset();
    eeg_buffer.reset();
    pressure_buffer.reset();
    speed_buffer.reset();

    run_data.update_datetime();
    run_data.update_imu_calibration(IMU);
//    run_data.update_animal_name(TagComms.readStringUntil('|', 120U));
//    run_data.update_animal_species(TagComms.readStringUntil('|', 120U));
//    run_data.update_description(TagComms.readStringUntil('|', 120U));

    bytes_written = 0;

#ifdef ETAG_DEBUG
    sampling_timer = 0;
#endif
    logger.print_array("Device Enable", device_start, NUM_DEVICES);
    logger.print_buffer_headers("Hydrophone", hydrophone_buffer.get_buffers(), HYDROPHONE_BUFF_NUM);
    logger.print_buffer_headers("IMU", imu_buffer.get_buffers(), IMU_BUFF_NUM);
    logger.print_buffer_headers("EEG", eeg_buffer.get_buffers(), EEG_BUFFER_NUM);
    logger.print_buffer_headers("PRESSURE", pressure_buffer.get_buffers(), PRESSURE_BUFFER_NUM);
    logger.print_buffer_headers("SPEED", speed_buffer.get_buffers(), SPEED_BUFFER_NUM);

    deviceManager.begin_sampling();
    Tag_Bluetooth.begin_sampling();
    eeg_buffer.init();
}

void stop_sampling() {
    Tag_Bluetooth.end_sampling();
    deviceManager.end_sampling();
    eeg_buffer.reset();
    queue.reset();

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

#ifdef EEG_TIMING
    print_timings(eeg_timing_info, TTL_INT_LENGTH);
#endif
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
    delay(1);
}

void initalize_devices() {
    deviceManager.initialize_devices();
}

void dummy_file() {
    const size_t write_buffer_length = 256;
    const size_t num_buffer_writes =  4000;

    uint8_t write_buffer[write_buffer_length];

    for (size_t i = 0; i < write_buffer_length; i++) {
        write_buffer[i] = 255 - i;
    }
    
    diskManager.create_data_file("DummyFile.bin");

    for (size_t i = 0; i < num_buffer_writes; i++) {
        logger.print_variable("Current count", i);
        reverse_array(write_buffer, write_buffer_length);
        diskManager.write_to_file(write_buffer, write_buffer_length);
    }

    reverse_array(write_buffer, write_buffer_length);
    diskManager.write_to_file(write_buffer, 41); //print partial at the end

    logger.print_message("Done writing dummy file");

    diskManager.close_file();
}

void reverse_array(uint8_t* ptr, size_t length) {
    uint8_t* head = ptr;
    uint8_t* tail = ptr + length - 1; // len is one past the end of the array, we need the address of last element
    uint8_t tmp;
    for (size_t i = 0; i < (length/2); i++) {
        tmp = *(head + i);
        *(head + i) = *(tail - i);
        *(tail - i) = tmp;
    } 
}