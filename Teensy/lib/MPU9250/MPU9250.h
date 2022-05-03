#ifndef _MPU9250_H_
#define _MPU9250_H_

#include <SPI.h>
#include <Wire.h>
#include <MPU9250_Definitions.h>
#include "AbstractDevice.h"

#include "Logger.h"
extern Logger logger;

#define MPU9250_ADDRESS MPU9250_ADDRESS_AD0

/**
 * @brief Class to implement MPU9250 functionality
 * 
 * Based on the libary written by Sparkfun (see <a href=https://github.com/sparkfun/SparkFun_MPU-9250_Breakout_Arduino_Library>here</a>), which
 * itself implemented a library by Kris Winer (see <a href=https://github.com/kriswiner/MPU9250>here</a>).
 * The interface of these libraries was updated to work with the Abstract Device scheme.
 * 
 * \todo Rewrite this library in a much cleaner way, eliminating the jank and lack of extensibility, and write
 * proper getter and setter methods for the device status.
 * 
 * \todo Finish documentation, but this will likely be finished only after this code is refactored
 */
class MPU9250 : public AbstractDevice
{
  protected:
    public: // temporary

    /**
     * @brief Accelerometer resolution
     * 
     */
    enum Ascale
    {
      AFS_2G = 0,  /**< 2g acceleration */
      AFS_4G,      /**< 4g acceleration */
      AFS_8G,      /**< 8g acceleration */
      AFS_16G      /**< 16g acceleration */
    };

    /**
     * @brief Gyroscope resolution
     * 
     */
    enum Gscale {
      GFS_250DPS = 0, /**< 250 degrees per second */
      GFS_500DPS,     /**< 500 degrees per second */
      GFS_1000DPS,    /**< 1000 degrees per second */
      GFS_2000DPS     /**< 2000 degrees per second */
    };

    /**
     * @brief Magnetometer resolution
     * 
     */
    enum Mscale {
      MFS_14BITS = 0, /**< 0.6 mG per LSB */
      MFS_16BITS      /**< 0.15 mG per LSB */
    };

    /**
     * @brief Magentometer sampling mode
     */
    enum M_MODE {
      M_8HZ = 0x02,  /**< 8 Hz update */
      M_100HZ = 0x06 /**< 100 Hz continuous magnetometer */
    };

    
    TwoWire * _wire;						/**< Allows for use of various I2C ports */
    uint8_t _I2Caddr = MPU9250_ADDRESS_AD0;	/**< Use AD0 by default */

 	SPIClass * _spi;						/**< Allows for use of different SPI ports */
    int8_t _csPin; 							/**< SPI chip select pin */

    uint32_t _interfaceSpeed;				/**< Stores the desired I2C or SPi clock rate */

    /** TODO: Add setter methods for this hard coded stuff */
    uint8_t Gscale = GFS_250DPS; /**< Specifies the gyroscope scale */
    uint8_t Ascale = AFS_2G;     /**< Specifiers the accelerometer scale */
    // Choose either 14-bit or 16-bit magnetometer resolution
    uint8_t Mscale = MFS_16BITS; /**< Specifiers resolution of magnetometer */

    // 2 for 8 Hz, 6 for 100 Hz continuous magnetometer data read
    uint8_t Mmode = M_8HZ;    /**< Specifiers the sampling rate of mangetometer */

    uint8_t writeByteWire(uint8_t, uint8_t, uint8_t);
    uint8_t writeByteSPI(uint8_t, uint8_t);
    uint8_t writeMagByteSPI(uint8_t subAddress, uint8_t data);
    uint8_t readByteSPI(uint8_t subAddress);
    uint8_t readMagByteSPI(uint8_t subAddress);
    uint8_t readByteWire(uint8_t address, uint8_t subAddress);
    bool magInit();
    void kickHardware();
    void select();
    void deselect();
    void setupMagForSPI();
// TODO: Remove this next line
public:
    uint8_t ak8963WhoAmI_SPI();

  public:
    float pitch, yaw, roll;
    float temperature;   // Stores the real internal chip temperature in Celsius
    int16_t tempCount;   // Temperature raw count output
    uint32_t delt_t = 0; // Used to control display output rate

    uint32_t count = 0, sumCount = 0; // used to control display output rate
    float deltat = 0.0f, sum = 0.0f;  // integration interval for both filter schemes
    uint32_t lastUpdate = 0, firstUpdate = 0; // used to calculate integration interval
    uint32_t Now = 0;        // used to calculate integration interval

    int16_t gyroCount[3];   // Stores the 16-bit signed gyro sensor output
    int16_t magCount[3];    // Stores the 16-bit signed magnetometer sensor output
    // Scale resolutions per LSB for the sensors
    float aRes, gRes, mRes;
    // Variables to hold latest sensor data values
    float ax, ay, az, gx, gy, gz, mx, my, mz;
    // Factory mag calibration and mag bias
    float factoryMagCalibration[3] = {0, 0, 0}, factoryMagBias[3] = {0, 0, 0};
    // Bias corrections for gyro, accelerometer, and magnetometer
    float gyroBias[3]  = {0, 0, 0},
          accelBias[3] = {0, 0, 0},
          magBias[3]   = {0, 0, 0},
          magScale[3]  = {0, 0, 0};
    float selfTest[6];
    // Stores the 16-bit signed accelerometer sensor output
    int16_t accelCount[3];
    int16_t buffer[9];

    // Public method declarations
    MPU9250( int8_t csPin, SPIClass &spiInterface = SPI, uint32_t spi_freq = SPI_DATA_RATE);

    /**
     * @brief Construct a new MPU9250 object using default settings
     * 
     * @param address Default address is MPU9250_ADDRESS_AD0
     * @param wirePort Default write port is Wire
     * @param clock_frequency Default clock frequency is 100000
     */
    MPU9250( uint8_t address = MPU9250_ADDRESS_AD0, TwoWire &wirePort = Wire, uint32_t clock_frequency = 100000 );
    void getMres();
    void getGres();
    void getAres();
    void readAccelData(int16_t *);
    void readGyroData(int16_t *);
    void readMagData(int16_t *);
    bool sample();
    int16_t readTempData();
    void updateTime();
    void initAK8963(float *);
    void initMPU9250();
    void calibrateMPU9250(float * gyroBias, float * accelBias);
    void MPU9250SelfTest(float * destination);
    void magCalMPU9250(float * dest1, float * dest2);
    uint8_t writeByte(uint8_t, uint8_t, uint8_t);
    uint8_t readByte(uint8_t, uint8_t);
    uint8_t readBytes(uint8_t, uint8_t, uint8_t, uint8_t *);
    // TODO: make SPI/Wire private
    uint8_t readBytesSPI(uint8_t, uint8_t, uint8_t *);
    uint8_t readBytesWire(uint8_t, uint8_t, uint8_t, uint8_t *);
    bool isInI2cMode() { return _csPin == -1; }
    //bool begin();   // This is an inbuilt begin for using SPI, not what we are using

    //Implements the AbstractSamplingManager
    /**
     * @brief Changes the sampling flag to true to allow for reads during the loop() function
     * 
     * This device is queried during the loop function, does not work off interrupts just yet.
     * 
     */
    void begin() { allow_reads = true; };

    /**
     * @brief Changes the sampling flag to false to stop reads during the loop() function
     * 
     */   
    void end() { allow_reads = false; };


    /**
     * @brief Initializes the MPU9250
     * 
     * @return true once finishedd
     * 
     * Runs a self test, calibrates the device, starts both the MPU9250 (accel/gyro),
     * and then starts the AK8963 (magnetometer), as well as updating the resolutions
     * of all the devices based on the settings.
     * 
     */   
    bool init();

    private:
        bool allow_reads;
};  // class MPU9250

#endif // _MPU9250_H_
