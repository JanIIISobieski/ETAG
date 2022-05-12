#include <DiskManager.h>

SdFs DiskManager::sd;
FsFile DiskManager::file;
FsFile DiskManager::folder;

DiskManager::DiskManager(RunData* run_data) {
    data_file_size = 0;
    data_is_open = false;
    num_experimental_runs = 0;

    run_data = run_data;
}

DiskManager::~DiskManager() {
    
}

bool DiskManager::init() {
    bool return_val = sd.begin(SdioConfig(FIFO_SDIO));

    if (!sd.chdir()) {
        logger.print_message("Could not open root directory");  // open root
        logger.print_SD_error(sd);
        return_val = false;
    }

    if (!folder.open("/")) {
        logger.print_message("Could not open the root folder");
        logger.print_SD_error(sd);
        return_val = false;
    }

    if (return_val) logger.print_message("Successful SD card startup");

    return return_val;
}

bool DiskManager::reset() {
    uint8_t try_counter = 0;

    file.close();  //just close the file, don't clear the current filename or current position
    stop();

    while (!init()) {
        logger.print_SD_error(sd);
        ++try_counter;
        logger.print_variable("Trying to restart SD card:", try_counter);
        if (try_counter > 3) {  // we tried to read from the file 3 times, but still didn't work, error out
            logger.print_message("Too many read failures");
            file_cursor = 0;
            current_file = "";
            return false; // could not restart, quit out of function
        }
    }

    if (file.open(current_file.c_str(), O_READ)) logger.print_variable("Reopened file", current_file);
    else {
        logger.print_variable("Could not reopen file", current_file);
        logger.print_SD_error(sd);
    }

    if (file.seekSet(file_cursor)) logger.print_variable("Position", file_cursor);
    else {
        logger.print_variable("Could not reach file position", file_cursor);
        logger.print_SD_error(sd);
    }

    return true; // was able to restart, return true
}

void DiskManager::stop() {
    sd.end();
}

void DiskManager::begin() {
    create_data_file();
    write_header(&run_data);
}

void DiskManager::end() {
    close_file();
    folder.rewindDirectory();
}

inline bool DiskManager::go_to_root() {
    return sd.chdir();
}

size_t DiskManager::write_header(RunData* run_data) {
    size_t header_size = 0;

    StaticJsonDocument<512> header_info;
    StaticJsonDocument<256> json_imu_calibration;

    json_imu_calibration["aRes"] = (run_data->get_imu())->accel_resolution;
    json_imu_calibration["gRes"] = (run_data->get_imu())->gyro_resolution;
    json_imu_calibration["mRes"] = (run_data->get_imu())->magnetometer_resolution;

    JsonArray aBias = json_imu_calibration.createNestedArray("aBias");
    JsonArray mBias = json_imu_calibration.createNestedArray("mBias");
    JsonArray mCal  = json_imu_calibration.createNestedArray("mCal");

    write_json_array(&aBias, (run_data->get_imu())->accel_biases, 3);
    write_json_array(&mBias, (run_data->get_imu())->magnetometer_biases, 3);
    write_json_array(&mCal,  (run_data->get_imu())->magnetometer_calibration, 3);

    header_info["imu_calibration"] = json_imu_calibration;
    header_info["name"] = (run_data->get_animal())->name;
    header_info["species"] = (run_data->get_animal())->species;
    header_info["description"] = (run_data->get_description());

    header_size += serializeJson(header_info, file);
    header_size += file.print("\n");

    logger.print_json(header_info);

    return header_size;
}

bool DiskManager::create_data_file() {
    String data_name = run_data.get_datetime() + "_Data.bin";
    return create_data_file(data_name);
}

bool DiskManager::create_data_file(String filename) {
    logger.print_variable("Filename", filename);
    FsDateTime::setCallback(dateTime);
    data_is_open = file.open(filename.c_str(), O_CREAT | O_TRUNC | O_RDWR);
    this->current_file = filename;
    if (data_is_open) logger.print_message("Successfully opened file");
    else {
        logger.print_message("Opening file failed");
        logger.print_SD_error(sd);
    }
    return data_is_open;
}

bool DiskManager::close_file() {
    data_is_open = file.close();
    this->current_file = "";
    file_cursor = 0;
    return !data_is_open;
}

bool DiskManager::delete_file(uint8_t i) {
    return sd.remove(get_filename(i));
}

size_t DiskManager::update_folder_num() {
    logger.print_message("In update_folder_num()");

    // figure out how many directory files there are
    char filename[FILE_NAME_CHAR_NUM];
    num_experimental_runs = 0;
    
    while (file.openNext(&folder, O_READ)) {
        if (!file.isDir()) {
            file.getName(filename, FILE_NAME_CHAR_NUM);
            logger.print_variable("Filename", filename);
            num_experimental_runs++;
        }
    }

    folder.rewindDirectory(); // go back to the start of the directory

    logger.print_variable("Num Files", num_experimental_runs);

    return num_experimental_runs;
}

String DiskManager::get_filename(uint8_t i) {
    uint8_t ind = 0;
    char filename[FILE_NAME_CHAR_NUM];

    //send the appropriate file
    while (file.openNext(&folder, O_READ)) {
        if (!file.isDir()) {
            file.getName(filename, FILE_NAME_CHAR_NUM);
            if (ind++ == i) {
                break; //we got the file we wanted, can break out of while loop
            }
        }
    }

    folder.rewindDirectory(); //go back to the beginning of the SD card

    String stringified_name = String(filename);
    logger.print_variable("Name of file sent", stringified_name);

    return String(filename);
}

FileReadData* DiskManager::get_file_data() {
    uint64_t current_position = file.position();
    file_cursor = (file_cursor < current_position) ? current_position : file_cursor;
    read_data.bytes_read = file.read(read_data.data, FILE_READ_SIZE);
    logger.print_variable("Read Size", read_data.bytes_read);
    return &read_data;
}

void DiskManager::open_data_file(String filename) {
    if (file.open(filename.c_str(), O_RDONLY)) logger.print_variable("Succesfully opened file", filename);
    else logger.print_variable("Could not open file", filename);

    current_file = filename;
}

void dateTime(uint16_t* date, uint16_t* time, uint8_t* ms10) {
    *date = FS_DATE(year(), month(), day());
    *time = FS_TIME(hour(), minute(), second());
    *ms10 = second() & 1 ? 100 : 0;
}
