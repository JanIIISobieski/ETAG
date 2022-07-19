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
Tag_USB MICRO_USB = Tag_USB(USB_SERIAL_ADDRESS, &diskManager);

// Bluetooth
#define BLUETOOTH_SERIAL_ADDRESS    &Serial4
ESP32_Bluetooth Tag_Bluetooth = ESP32_Bluetooth(BLUETOOTH_SERIAL_ADDRESS,
                                                &diskManager); //initializes pin values

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
Tag_ADC hydrophone(HYDROPHONE_SAMPLING_PIN, HYDROPHONE_SAMPLING_FREQUENCY, HYDROPHONE_AVG, HYRDOPHONE_RESOLUTION, &hydrophone_buffer);

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
#define NIRS_LEFT_BUTTON 21
#define NIRS_RIGHT_BUTTON 22
#define NIRS_SHORT_PRESS 1000
#define NIRS_LONG_PRESS 3000
NIRS nirs(NIRS_LEFT_BUTTON, NIRS_RIGHT_BUTTON, NIRS_SHORT_PRESS, NIRS_LONG_PRESS);

// EEG
#define EEG_DRDY         14  // ADS1299 Data Ready pin
#define EEG_CS_PARENT     8  // ADS1299 Chip Select pin for master chip
#define EEG_CS_CHILD     10 // ADS1299 Chip Select pin for slave chip
#define EEG_RST          22  // ADS1299 Reset pin
#define EEG_START         9  // ADS1299 Start pin
#define EEG_PWDN         23  // ADS1299 Power-Down pin
ADS1299 eeg(EEG_DRDY, EEG_CS_PARENT, EEG_RST, EEG_START, EEG_PWDN);
//ADS1299 eeg2(EEG_DRDY, EEG_CS_SLAVE, EEG_RST, EEG_START, EEG_PWDN);

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

// Device Manager
#define NUM_DEVICES 4
AbstractDevice* devices[NUM_DEVICES] = {&nirs, &IMU, &eeg, &hydrophone};
bool device_start[NUM_DEVICES] = {false, true, false, false};
DeviceManager deviceManager(devices, device_start, NUM_DEVICES);

// Helpful Functions
inline void sampling();
void begin_sampling();
void stop_sampling();
void test_sampling();
void upload_files();
void update_parameters();
void sync_time();
void update_files();
void download_files();
void initalize_devices();
void delete_file();
void nirs_on();
void nirs_off();

/*
void dummy_file();
void reverse_array(uint8_t* ptr, size_t length);
*/

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
elapsedMillis sampling_timer;  // doesn't need to be initalized unless explicitly debugging
#endif
