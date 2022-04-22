/*
#include <Tag_SD.h>

bool SD_begin() {
    return sd.begin(SdioConfig(DMA_SDIO));
}

bool file_open(const char* filename, oflag_t flag) {
    return file.open(filename, flag);
}

void file_flush() {
    file.flush();
}

bool file_close() {
    return file.close();
}

bool file_isOpen() {
    return file.isOpen();
}

size_t file_write(void* buffer, size_t num_bytes) {
    return file.write(buffer, num_bytes);
}

size_t file_write(uint16_t* buffer, size_t num_bytes) {
    return file_write((void *)buffer, num_bytes);
}

size_t file_write(volatile uint16_t* buffer, size_t num_bytes) {
    return file_write((uint16_t *)(buffer), num_bytes);
}

size_t file_serial_write(String string) {
    return file.print(string);
}

bool file_sync() {
    return file.sync();
}

void file_rewind() {
    return file.rewind();
}

int file_read(uint8_t* buffer, size_t num_bytes) {
    return file.read(buffer, num_bytes);
}

int file_read(uint16_t* buffer, size_t num_bytes) {
    return file.read(buffer, num_bytes);
}

bool file_rm(const char* path){
    return sd.remove(path);
}

bool folder_mkdir(String folder_name){
    return sd.mkdir(folder_name);
}

bool folder_rm(String folder_name) {
    return sd.rmdir(folder_name);
}

bool folder_open(const char* path) {
    return folder.open(path);
}

bool folder_cd(String folder_path) {
    return sd.chdir(folder_path);
}

bool go_to_root() {
    return sd.chdir();
}

bool path_exists(String path) {
    return sd.exists(path);
}

void rewind_dir() {
    return folder.rewindDirectory();
}

*/