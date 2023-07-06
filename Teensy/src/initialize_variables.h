#include <Arduino.h>
#include <SPI.h>
#include <ADS1299.h>
#include "Tag_ADC.h"
#include "Tag_SD.h"
#include "DeviceManager.h"
#include "DiskManager.h"
#include "Tag_NIRS.h"
#include "MPU9250.h"
#include "TagCommunicator.h"
#include "ESP32_Bluetooth.h"
#include "Tag_USB.h"
#include "Tag_Queue.h"
#include "EEGBuffer.h"
#include "IMUBuffer.h"
#include "ADCBuffer.h"
#include "Logger.h"
#include "PressureBuffer.h"
#include "SpeedBuffer.h"
#include "WriterManager.h"
#include "BufferManager.h"
#include "LinkedList.h"
#include "NIRSBuffer.h"
#include "SamplingTimer.h"
#include "Arming.h"

// Communication
#define ACK 6

//Settings
RunData run_data; /**< Header file data */

// SD Card
// Disk Manager
DiskManager diskManager(&run_data);
size_t bytes_written = 0; /**< pre-allocate variable in which to write the number of bytes written to the SD card */

// USB
#define USB_SERIAL_ADDRESS &Serial
Tag_USB MICRO_USB = Tag_USB(USB_SERIAL_ADDRESS, &diskManager, &run_data);

// Bluetooth
#define BLUETOOTH_SERIAL_ADDRESS    &Serial4
ESP32_Bluetooth Tag_Bluetooth = ESP32_Bluetooth(BLUETOOTH_SERIAL_ADDRESS,
                                                &diskManager,
                                                &run_data); //initializes pin values

// Communicator Manager
#define NUM_COMMUNICATORS 2
SerialCommunicator* comm_array[NUM_COMMUNICATORS] = {&Tag_Bluetooth, &MICRO_USB};  /**< has to be pointers. SerialCommunicator is an abstract class and so cannot be instantiated. */
TagCommunicator TagComms = TagCommunicator(comm_array, NUM_COMMUNICATORS);

// Global communication variables
char command = '0';     /** Stores the current command */
int read_val = -1;      /** Stores the last read value */

// Writer Manager
#define NUM_WRITERS 3

enum WriterLocation : uint8_t { 
    BLUETOOTH = 0,
    USB = 1,
    SD  = 2
}; /**< Selects to where data should be written during sampling */

Writer* writers[NUM_WRITERS] = {&Tag_Bluetooth, &MICRO_USB, &diskManager};
WriterManager WriteManager(writers, NUM_WRITERS);

// Buffer Queue
Tag_Queue queue;                    /**< Declare the queue */
Tag_Queue* buffer_ptr = &queue;     /**< Get the buffer pointer to use as input for various functions */

// Hydrophone
#define HYDROPHONE_BUFF_ID 128
#define HYDROPHONE_BUFF_LENGTH 4096
#define HYDROPHONE_BUFF_NUM 2
static volatile uint16_t __attribute__((aligned(16))) hydrophone_buff1[HYDROPHONE_BUFF_LENGTH];
static volatile uint16_t __attribute__((aligned(16))) hydrophone_buff2[HYDROPHONE_BUFF_LENGTH];
static volatile uint16_t* adc_buffers[HYDROPHONE_BUFF_NUM] = {hydrophone_buff1, hydrophone_buff2};
ADCBuffer hydrophone_buffer(adc_buffers, HYDROPHONE_BUFF_NUM, HYDROPHONE_BUFF_LENGTH, HYDROPHONE_BUFF_ID, &queue);

#define HYDROPHONE_SAMPLING_PIN         15
#define HYDROPHONE_AVG                  0
#define HYRDOPHONE_RESOLUTION           12
#define HYDROPHONE_SAMPLING_FREQUENCY   40000
ADCSettings adc_settings {};
Tag_ADC hydrophone(HYDROPHONE_SAMPLING_PIN, &adc_settings, &hydrophone_buffer);

// IMU
#define MPU9250_ADDRESS MPU9250_ADDRESS_AD0
#define I2CPort Wire
#define I2Cclock 200000
MPU9250 IMU(MPU9250_ADDRESS, I2CPort, I2Cclock);

#define IMU_BUFF_ID 1
#define IMU_BUFF_LENGTH 4096
#define IMU_BUFF_NUM 4
static volatile int16_t __attribute__((aligned(16))) imu_buff1[IMU_BUFF_LENGTH];
static volatile int16_t __attribute__((aligned(16))) imu_buff2[IMU_BUFF_LENGTH];
static volatile int16_t __attribute__((aligned(16))) imu_buff3[IMU_BUFF_LENGTH];
static volatile int16_t __attribute__((aligned(16))) imu_buff4[IMU_BUFF_LENGTH];
static volatile int16_t* imu_buffers[IMU_BUFF_NUM] = {imu_buff1, imu_buff2, imu_buff3, imu_buff4};
IMUBuffer imu_buffer(imu_buffers, IMU_BUFF_NUM, IMU_BUFF_LENGTH, IMU_BUFF_ID, &queue);

// NIRS
#define NIRS_LEFT_BUTTON 20
#define NIRS_RIGHT_BUTTON 21
#define NIRS_SHORT_PRESS 500
#define NIRS_LONG_PRESS 3000
NIRS nirs(NIRS_LEFT_BUTTON, NIRS_RIGHT_BUTTON, NIRS_SHORT_PRESS, NIRS_LONG_PRESS);

#define NIRS_BUFF_ID 2
#define NIRS_BUFF_LENGTH 8192
#define NIRS_BUFF_NUM 2
static volatile uint8_t nirs_buffer1[NIRS_BUFF_LENGTH];
static volatile uint8_t nirs_buffer2[NIRS_BUFF_LENGTH];
static volatile uint8_t* nirs_buffers[NIRS_BUFF_NUM] = {nirs_buffer1, nirs_buffer2};
NIRSBuffer nirs_buffer(nirs_buffers, NIRS_BUFF_NUM, NIRS_BUFF_LENGTH, NIRS_BUFF_ID, &queue);

// EEG
#define EEG_DRDY         14  // ADS1299 Data Ready pin
#define EEG_CS_PARENT     8  // ADS1299 Chip Select pin for parent chip
#define EEG_CS_CHILD     10 // ADS1299 Chip Select pin for child chip
#define EEG_RST          22  // ADS1299 Reset pin
#define EEG_START         9  // ADS1299 Start pin
#define EEG_PWDN         23  // ADS1299 Power-Down pin
ADS1299Settings parent_eeg_settings {};
ADS1299Settings child_eeg_settings {};
ADS1299 eeg(EEG_DRDY, EEG_CS_PARENT, EEG_RST, EEG_START, EEG_PWDN, &parent_eeg_settings);
//ADS1299 eeg2(EEG_DRDY, EEG_CS_CHILD, EEG_RST, EEG_START, EEG_PWDN, &child_eeg_settings);

#define EEG_BUFF_ID 8
#define EEG_BUFFER_LENGTH 8192
#define EEG_BUFFER_NUM 4
static volatile uint8_t ads_buffer1[EEG_BUFFER_LENGTH];
static volatile uint8_t ads_buffer2[EEG_BUFFER_LENGTH];
static volatile uint8_t ads_buffer3[EEG_BUFFER_LENGTH];
static volatile uint8_t ads_buffer4[EEG_BUFFER_LENGTH];
static volatile uint8_t* ads_buffers[EEG_BUFFER_NUM] = {ads_buffer1, ads_buffer2, ads_buffer3, ads_buffer4};
EEGBuffer eeg_buffer(ads_buffers, EEG_BUFFER_NUM, EEG_BUFFER_LENGTH, EEG_BUFF_ID, &queue);

// Pressure Buffer
#define PRESSURE_BUFF_ID 16
#define PRESSURE_BUFFER_LENGTH 8192
#define PRESSURE_BUFFER_NUM 4
static volatile uint8_t pressure_buffer1[PRESSURE_BUFFER_LENGTH];
static volatile uint8_t pressure_buffer2[PRESSURE_BUFFER_LENGTH];
static volatile uint8_t pressure_buffer3[PRESSURE_BUFFER_LENGTH];
static volatile uint8_t pressure_buffer4[PRESSURE_BUFFER_LENGTH];
static volatile uint8_t* pressure_buffers[PRESSURE_BUFFER_NUM] = {pressure_buffer1, pressure_buffer2, pressure_buffer3, pressure_buffer4};
PressureBuffer pressure_buffer(pressure_buffers, PRESSURE_BUFFER_NUM, PRESSURE_BUFFER_LENGTH, PRESSURE_BUFF_ID, &queue);

// Speed Buffer
#define SPEED_BUFF_ID 64
#define SPEED_BUFFER_LENGTH 8192
#define SPEED_BUFFER_NUM 4
static volatile uint8_t speed_buffer1[SPEED_BUFFER_LENGTH];
static volatile uint8_t speed_buffer2[SPEED_BUFFER_LENGTH];
static volatile uint8_t speed_buffer3[SPEED_BUFFER_LENGTH];
static volatile uint8_t speed_buffer4[SPEED_BUFFER_LENGTH];
static volatile uint8_t* speed_buffers[SPEED_BUFFER_NUM] = {speed_buffer1, speed_buffer2, speed_buffer3, speed_buffer4};
SpeedBuffer speed_buffer(speed_buffers, SPEED_BUFFER_NUM, SPEED_BUFFER_LENGTH, SPEED_BUFF_ID, &queue);

//Buffer Manager
#define NUM_BUFFER_TYPES 5
BufferBase* buffers[NUM_BUFFER_TYPES] = {&imu_buffer, &eeg_buffer, &hydrophone_buffer, &pressure_buffer, &speed_buffer};
BufferManager buffer_manager(buffers, NUM_BUFFER_TYPES);

// Device Manager
#define NUM_DEVICES 5
AbstractDevice* devices[NUM_DEVICES] = {&nirs, &IMU, &eeg, &hydrophone, &Tag_Bluetooth};
DeviceEnable device_settings {};
DeviceManager deviceManager(devices, device_settings.raw_bytes, NUM_DEVICES);

// Sampling Timer -> ensures that sampling ends and release is started
SamplingTimer samplingTimer;

// Tag Arming -> checks the saltwater switch to start sampling
Arming arming;

// Helpful Functions

/**
 * @brief Function to handle sampling of devices
 * 
 * The devices whose code runs in this sampling loop are set to poll data, and thus run in the main loop.
 * Other devices, such as the Hydrophone (using the ADC converter) or the EEG (ADS1299 library) sample based on
 * interrupts defined in their corresponding begin() and end() methods for sampling as part of the \ref AbstractDevice class inheritance.
 * This loop is also responsible for writing data to the corresponding writer (Bluetooth, USB, or SD card) once the \ref queue
 */
inline void sampling();

/**
 * @brief Starts the sampling of all the devices
 * 
 * This function performs several tasks:
 *   #. Reads which writer (see \ref AbstractWriter) is going to be responsible for logging the data
 *      0 -> Bluetooth, 1 -> USB, 2 -> SD card
 *   #. Reads any of the additional metadata sent over the \ref TagCommunicator (animal name, animal species, description)
 *   #. Updates the IMU calibration structure with last recorded data from the IMU
 *   #. Resets the Queue (see \ref Tag_Queue) to the default, empty state
 *   #. Resets each of the buffers to an empty state, with only the ID byte kept for each buffer
 *   #. Runs the pre_sampling_setup routine for the used Writer (see \ref AbstractWriter)
 *      For these routines, the SD card writing is the one that needs a pre-sampling routine to open a file
 *      and then write to it.
 */
void begin_sampling();

/**
 * @brief Stops the sampling devices and does any post-sampling tasks
 * 
 * The post-sampling tasks are especially applicable for the SD card \ref Writer, as the file has
 * to be closed and synced.
 */
void stop_sampling();

/**
 * @brief Function that will test the sampling and stream this data over Bluetooth.
 * 
 * Note that this function still has to be implemented.
 * 
 */
void test_sampling();

/**
 * @brief This function updates the structures that hold the settings for each sampling device
 * 
 * note that this function only updates the structures for \ref ADCSettings, \ref ADS1299Settings,
 * and \ref DeviceSettings. None of the devices get updated with the new settings though. For that,
 * \ref initialize_devices() must be run. The data itself comes as a series of raw bytes. 
 * 
 */
void update_parameters();

void sync_time();

/**
 * @brief This function reads all the available files in the SD card root directory, and sends the file names over the \ref TagCommunicator
 * 
 * This function reads first the number of files present in the root directory (/) on the SD card, and then sends
 * the filename. Thus, the sents results would look like this:
 * 
 * Sent to the Teensy |  File Index
 * -------------------|:------------:
 * Filename1.bin      |      0
 * Filename2.bin      |      1
 * Filename3.bin      |      2
 * Filename4.bin      |      3
 * 
 * The File Index is what is sent over the \ref TagCommunicator to indicate on which file to do operations.
 * The File Index is sent for \ref download_file, \ref delete_file to indicate which file to download or delete.
 */
void update_files();

/**
 * @brief Downloads a file from the tag to the computer
 * 
 * This function will read a file index from the serial line, and then
 * call the appropriate \ref TagCommunicator to send the file. A file is sent
 * in its entirety.
 * 
 */
void download_files();

/**
 * @brief Initialize the sampling devices
 * 
 * Based on the device settings (see \ref DeviceEnable, \ref ADCSettings, \ref ADS1299Settings), the
 * devices are initialized to prepare them for sampling. This function must be run each time
 * \ref update_parameters() is run, as this function is actually responsible for updating each devices settings,
 * unlike \ref update_parameters() which only reads the new settings but does not update the devices.
 * 
 */
void initalize_devices();

/**
 * @brief This function will delete a file from the SD card, freeing up space
 * 
 * This function first reads a file index (the numerical position of the file from the root).
 * Then, this file is deleted from the SD card.
 * 
 */
void delete_file();

/**
 * @brief Turns on the NIRS system
 * 
 * Unfortunately, the Teensy does not currently track the state of the NIRS system.
 * Thus, this function only ever presses a button, but does not know when the NIRS system is actually on.
 * 
 */
void nirs_on();

/**
 * @brief Turns off the NIRS system
 * 
 * Unfortunately, the Teensy does not currently track the state of the NIRS system.
 * Thus, this function only ever presses a button, but does not know when the NIRS is actually off.
 * 
 */
void nirs_off();

/**
 * @brief Forwards logging calls from ESP32 to the logger
 * 
 * All logging calls from the ESP32 are in the form of L[message]\n.
 * Thus, when the command is L, the rest of the message will be read and sent to the logger.
 * Additionally, thus function should not be relied upon too heavily. This solves the issue when
 * a logging call comes in while the main() loop is happening. However, if a logging call occurs
 * when the Teensy is currently expecting data, it will result in improper data being written.
 * I (Gabriel) think the only way of avoiding this is to seperate out another Serial line on the
 * ESP32 to handle logging.
 * 
 */
inline void forward_ESP32_logging();

// Allow the logger to be initialized even in RELEASE mode, along with the summary
// structures.
// Flag passed in platformini.io will prevent writes (ETAG_LOG_LEVEL = 0 in RELEASE mode)
// Allows for eliminating some of the very nested #ifdef ETAG_DEBUG #else #endif patterns
// that had to exist before. Now the ETAG_DEBUG flags simply prevent from executing more
// code than is necessary, esepcially in more time-sensitive methods (interrupts).
// Parts of the code that would additionally run due to the ETAG_DEBUG flag (but not
// needed their own seperate path) were kept. i.e. if the code previously had a 
// #ifdef ETAG_DEBUG ... #endif, those were kept
Logger logger(&SerialUSB1);
TimingData hydrophone_buffer_push{}; //these timers are structures which are initialized and then never used when not debugging
TimingData eeg_buffer_push{};
TimingData imu_buffer_push{};
TimingData speed_buffer_push{};
TimingData pressure_buffer_push{};
TimingData sampling_loop{};
PrintTiming stopOnCompletionTimer{};

#ifdef ETAG_DEBUG
elapsedMillis sampling_timer;  // shouldn't be initalized unless explicitly debugging
#endif
