//
// ADS1299.h
//

#ifndef _ADS1299_
#define _ADS1299_

// comment out definition to use standard SPI for data retrieval
#define USE_DMA_SPI

#include <Arduino.h>
#include <SPI.h>
#include "ADS1299_Definitions.h"
#include "AbstractDevice.h"
#include "TagSettings.h"

#include "Logger.h"
extern Logger logger;

/**
 * @brief Implements the interface to use with the ADS1299 chip for electroencephalography
 * 
 * <a href="https://www.ti.com/lit/ds/symlink/ads1299.pdf?ts=1642008610691">ADS129 Documentation</a> for reference
 */
class ADS1299 : public AbstractDevice{
public:
    /**
     * @brief Construct a new ADS1299 object
     * 
     * @param data_ready The pin used for the interrupt that flags when new data is ready on the chip
     * @param chip_select The pin used for SPI chip select
     * @param reset_pin The pin used for resetting the ADS1299
     * @param start_pin The pin used for starting the ADS1299
     * @param power_down The pin used for powering down the ADS1299
     */
    ADS1299(uint32_t data_ready, uint32_t chip_select, uint32_t reset_pin, uint32_t start_pin, uint32_t power_down, ADS1299Settings* settings) {
        drdy = data_ready;
        cs = chip_select;
        rst = reset_pin;
        strt = start_pin;
        pwrdwn = power_down;
    };

    uint32_t drdy;      //**< Data Ready pin number */
    uint32_t cs;        //**< Chip Select pin number */
    uint32_t rst;       //**< Reset pin number */
    uint32_t strt;      //**< Start pin number */
    uint32_t pwrdwn;    //**< Power down pin number */

    uint32_t output_count; //**< Counter for the number of times the ADS was sampled (based on explicilty sampling) */
    uint32_t ads_status; //**< Register to store the status bytes of sampling */
    int32_t channel_data[CHANNELS_PER_SAMPLE]; //**< Register to store data based on explicit reads */
    uint8_t raw_channel_data[CHANNELS_PER_SAMPLE * BYTES_PER_CHANNEL]; //**< Register to store the raw byte data based on explicit reads */
    float mV_scale_factor; //**< The scaling factor to go from int24_t to mV */
    volatile uint8_t raw_data[BYTES_PER_SAMPLE]; //**< Register used to use for convering uint8_t to int24_t */
    //volatile boolean data_ready;

    SPISettings ADS_SPI_settings; //**< Storage of the settings for SPI Transfer to the device */

    EventResponder dma_spi_transfer; //**< Event responder to fire after SPI transfer */

    void setup(uint32_t _DRDY, uint32_t _CS); //**< Responsible for the power up of the chip and setting the registers (this function also calls \ref initalize()) */
    String initialize(); //**< Writes values to the ADS1299 register */
    void spi_start(uint32_t _CS, SPISettings _SPI_settings); //**< Begins the SPI transaction */
    void spi_end(uint32_t _CS); //**< Completes an SPI transaction */
    void send_command(uint8_t cmd); //*< Sends commands to the ADS1299, see ADS1299 documentation for what these are */

    /**** SPI Command Defintions (Datasheet, pg 40) ****/
    void wakeup();        //**< Wake-up from standby mode */
    void standby();       //**< Enter standby mode */
    String reset();       //**< Reset the device */
    String start();       //**< Start and restart (synchronize) conversions */
    String stop();        //**< Stop conversion */

    // Data Read Commands
    void rdatac();      //**< Enable Read Data Continuous mode (default at powerup)  */
    void sdatac();      //**< Stop Read Data Continuously mode */
    void rdata();       //**< Read data by command */

    // Register Read/Write Commands

    uint8_t get_id(); //**< Reads the ID register of the ADS1299 */

    /**
     * @brief Read the register at the specified address of the ADS1299 
     * 
     * @param address Address on the chip
     * @return String the read value followed by a newline
     */
    String rreg(uint8_t address);

    /**
     * @brief Write to the register at the specified address of the ADS1299
     * 
     * @param address Address on the chip
     * @param value Value to write to the chip
     * @return String Confirms modification of the register
     */
    String wreg(uint8_t address, uint8_t value);
    /***************************************************/

    /**
     * @brief Print the name of the register
     * 
     * @param address Name of register whose name to print
     * @return String the name of the register
     */
    String print_reg_name(uint8_t address);

    /**
     * @brief Samples the ADS1299 and sends the data to the appropriate buffer: \ref channel_data
     * 
     * This should only be used if explicitly sampling in the main loop
     * 
     */
    void update_data();

    /**
     * @brief Initalize the ADS1299 for sampling
     * 
     * Uses the \ref setup command. init() is the method that has to be called for the DeviceManager to work
     */
    bool init();

    /**
     * @brief Starts continous sampling of the ADS1299
     *
     */
    void begin();

    /**
     * @brief Ends continous sampling of the ADS1299
     * 
     */
    void end();

private:
    ADS1299Settings* eeg_settings;

};

// This let's us call into the class from within the library if necessary
extern ADS1299 eeg;

#endif
