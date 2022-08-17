#ifndef GUARD_DISK_MANAGER
#define GUARD_DISK_MANAGER

#include <TimeLib.h>
#include "AbstractDevice.h"
#include "SdFat.h"
#include "Writer.h"

#include "Logger.h"
extern Logger logger;

//YYYY:MM:DD_hh:mm:ss -> 19 ASCII characters, + '\0' from string gives 20
//Header.txt is additionally 10 more characters 
#define FILE_NAME_CHAR_NUM 30
#define FILE_READ_SIZE 2048

struct FileReadData {
    int bytes_read;
    uint8_t data[FILE_READ_SIZE];
};

/**
 * @brief DateTime callback for establishing the proper time datetime on files
 * 
 * Originates from the <a href="https://github.com/greiman/SdFat">SD Fat</a> library.
 * 
 * @param date Pointer to the date will be saved
 * @param time Pointer to the time will be saved
 * @param ms10 Pointer to where milliseconds to the nearest 10 will be saved
 */
void dateTime(uint16_t* date, uint16_t* time, uint8_t* ms10);

/**
 * @brief Class to manage the SD card and files for the tag
 */
class DiskManager : public Writer {
    private:
        size_t data_file_size;        /**< Can store the file size */
        size_t num_experimental_runs; /**< Updated by \ref update_folder_num(), the number of files on the disk */
        bool data_is_open;            /**< The file status */

        FileReadData read_data; /**< Stores buffer for read data */
        uint64_t file_cursor; /**< Used for keeping track of location in the file */
        String current_file;  /**< Stores the current file in use */

        inline bool go_to_root(); /**< @brief Returns to the root directory */
        bool create_data_file(); /**< @brief Opens up data file */

    public:
        static SdFs sd;        /**< Declares the SD card */
        static FsFile file;    /**< Declares a file, used for opening and closing files */
        static FsFile folder;  /**< Declares a folder, used for iterating through the root directory, and rewinding directory back to start */

        /**
         * @brief Construct a new Disk Manager object
         * 
         * @param run_data A pointer to a RunData structure that contains metadata for the trial
         */
        DiskManager(RunData* run_data);

        /**
         * @brief Destroy the Disk Manager object
         * 
         * Should never be called
         */
        ~DiskManager();

        /**
         * @brief Initalizes the SD card
         * 
         * @return bool Whether successful start up or not
         * 
         * Opens the SD card, opens up the root folder
         */
        bool init();

        /**
         * @brief Opens file for recieving data
         * 
         * Opens the file, calls the DateTime callback, and writes the file header
         */
        void pre_sampling_setup();

        /**
         * @brief Ends sampling
         * 
         * Closes file, and rewinds the directory to the beginning
         */
        void post_sampling_conclude();

        size_t update_folder_num(); /**< @brief Gets the total number of files present */

        /**
         * @brief Deletes the file at the ith index
         * 
         * @param i Index of the file (measured from the beginning of the root folder)
         * @return true File was successfully deleted 
         * @return false File was not successfully deleted, some error occured
         */
        bool delete_file(uint8_t i);

        /**
         * @brief Get the filename
         * 
         * @param i Index of the file (measured from the beginning of the root folder)
         * @return String the name of the file
         */
        String get_filename(uint8_t i);

        /**
         * @brief Stop the device and files
         * 
         */
        void stop();

        /**
         * @brief Get the file data object
         * 
         * @return FileReadData* pointer to the file data object
         * 
         * This function will return a data object that contains the number of bytes read,
         * as well as the location to where the data was written.
         */
        FileReadData* get_file_data();

        /**
         * @brief Opens a data file for reading
         * 
         */
        void open_data_file(String filename);

        /**
         * @brief Create a file object
         * 
         * @param filename The name of the file to create and open
         */
        bool create_data_file(String filename);

        /**
         * @brief Writes the RunData metadata as the header for the data file
         * 
         * @param run_data Pointer to the RunData object
         * @return size_t Returns the number of bytes written as the header
         */
        size_t write_header();

        /**
         * @brief Closes all files, restarts the SD card, and reopens the files
         * 
         * 
         * Returns true if successfully restarted, otherwise false
         */
        bool reset();

        /**
         * @brief Get the file size object
         * 
         * @return uint64_t The size of the file in bytes
         */
        inline uint64_t get_file_size() { return file.size(); };

        /**
         * @brief Writes the data to the file, implements function from the abstract class \ref Writer.
         * 
         * @param buffer_ptr The pointer of the data to write
         * @param num_bytes The number of bytes to write
         * @return size_t The number of bytes written
         */
        size_t write_data(void* buffer_ptr, size_t num_bytes) { return file.write(buffer_ptr, num_bytes); };

        /**
         * @brief Closes the data file, as well as bring the file cursor back to the beginning
         * 
         * @return true File successfully closed
         * @return false File did not close successfully
         */
        bool close_file(); /**< @brief Closes data file */
};

#endif //GUARD_DISK_MANAGER