#ifndef GUARD_BLUETOOTH
#define GUARD_BLUETOOTH

#include <Arduino.h>
#include <SerialCommunicator.h>

#define FILE_READ_SIZE 2048 // Read large amount from file to prevent constant reads from SD card.
#define BLUETOOTH_BUFFER_SIZE 64
#define BLUETOOTH_WRITE_BUFFER 64

class RN42_Bluetooth : public SerialCommunicator {
    public:
    /**
     * @brief Construct a new Bluetooth object
     * 
     * See <a href=https://ww1.microchip.com/downloads/en/DeviceDoc/Class-2-Bluetooth-Module-with-EDR-Support-DS50002328B.pdf>RN42 Datasheet</a> for more specifics.
     * 
     * @param bt_master Master pin for bluetooth
     * @param bt_discovery Discovery pin for the bluetooth
     * @param bt_RTS RTS Pin (for flow control)
     * @param bt_CTS CTS Pin (for flow control)
     * @param bt_serial Hardware Serial to use
     */
    RN42_Bluetooth(uint8_t bt_master, uint8_t bt_discovery, uint8_t bt_RTS, uint8_t bt_CTS, HardwareSerial* bt_serial, DiskManager* diskManagerPtr);

    /**
     * @brief Destroy the Bluetooth object
     * 
     */
    ~RN42_Bluetooth() {};

    /**
     * @brief Set the discovery pin state
     * 
     * @param val HIGH or LOW
     */
    void set_discovery_pin(uint8_t val);

    /**
     * @brief Set the master pin state
     * 
     * @param val HIGH or LOW
     */
    void set_master_pin(uint8_t val);

    /**
     * @brief Set the CTS pin state
     * 
     * @param val HIGH or LOW
     */
    void set_CTS_pin(uint8_t val);

    /**
     * @brief Set the RTS pin state
     * 
     * @param val HIGH or LOW
     */
    void set_RTS_pin(uint8_t val);

    /**
     * @brief Initalize serial state and the baud rate
     * 
     * @param baud_rate Baud rate
     */
    void init(uint32_t baud_rate);

    /**
     * @brief Methods for sending a file over Bluetooth
     * 
     * Note that this is not recommended as the file upload method, this method is slow
     * 
     * @param file_name Name of file to upload
     * @return true File uploaded successfully
     * @return false File not uploaded sucessfully
     */
    bool file_send(String file_name);

    private:
    uint8_t master;     /**< Master pin */
    uint8_t discovery;  /**< Discovery pin */
    uint8_t RTS;        /**< RTS pin */
    uint8_t CTS;        /**< CTS pin */
    HardwareSerial* hard_serial;   /**< Serial port to use, SerialCommunicator takes in a Stream Object (parent of HardwareSerial), as serial.begin() only lives in HardwareSerial */

    void begin(uint32_t baud_rate); /**< Begin the serial */

    static uint8_t bluetooth_write_buffer[BLUETOOTH_WRITE_BUFFER];
};

#endif  //GUARD_BLUETOOTH
