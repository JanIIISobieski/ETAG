/**
 * @file Tag_SD.h
 * @author your name (you@domain.com)
 * @brief Defines SD card
 * @version 0.1
 * @date 2022-01-13
 * 
 * @copyright Copyright (c) 2022
 * 
 * \todo This file is actually extraneous, all that is needed is SdFat to be imported.
 * Future versions of the code will refactor this file away.
 * 
 */

#pragma once

#include "SdFat.h"
#include "Tag_Info.h"

#include "Logger.h"
extern Logger logger;

#define SD_FAT_TYPE 3

// Teensy 4.0 uses first SPI port.
// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Initialize the SD card and set up the file
#if SD_FAT_TYPE == 0
extern SdFat sd;
extern File file;
extern File folder;
#elif SD_FAT_TYPE == 1
extern SdFat32 sd;
extern File32 file;
extern File32 folder;
#elif SD_FAT_TYPE == 2
extern SdExFat sd;
extern ExFile file;
extern ExFile folder;
#elif SD_FAT_TYPE == 3
extern SdFs sd;
extern FsFile file;
extern FsFile folder;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

/*
bool SD_begin();
bool file_open(const char* filename, oflag_t flag);
bool file_close();
bool file_isOpen();

size_t file_write(void* buffer, size_t num_bytes);
size_t file_write(uint16_t* buffer, size_t num_bytes);
size_t file_write(volatile uint16_t* buffer, size_t num_bytes);
size_t file_serial_write(String string);

int file_read(uint8_t* buffer, size_t num_bytes);
int file_read(uint16_t* buffer, size_t num_bytes);

bool file_sync();
void file_flush();
void file_rewind();
void rewind_dir();

bool file_rm(const char* path);

bool folder_mkdir(String folder_name);
bool folder_rm(String folder_name);
bool folder_open(const char* path);
bool folder_cd(String folder_path);
bool path_exists(String path);
bool go_to_root();
*/