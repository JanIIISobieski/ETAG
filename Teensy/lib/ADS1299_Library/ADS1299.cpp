//
// ADS1299.cpp
//

#include "ADS1299.h"

void ADS1299::setup(uint32_t _DRDY, uint32_t _CS) {
    drdy = _DRDY;
    cs = _CS;

    // mV_scale_factor only used if converting to engineering units within main
    mV_scale_factor = 1000.0 * V_REF / ((float)(pow(2, 23) - 1)) / 24.0;
    //data_ready = false;

    ADS_SPI_settings = SPISettings(4000000, MSBFIRST, SPI_MODE1);

    pinMode(drdy, INPUT);
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);

    // This sets dma_spi_finished function to be called at end of DMA SPI transfer
    initialize();
}
String ADS1299::initialize() {
    // recommended power up sequence requiers >Tpor (~32mS)
    delay(50);
    reset();                // reset the on-board ADS registers

    // For register map and settings see Datasheet, pg 44
    wreg(CONFIG1, 0x96);    // 0x90 for 16 kSPS, 0x96 for 250 SPS. Incrementing from 0x90 by one halves the sampling rate, up to 0x96
    wreg(CONFIG2, 0xD5);    // For testing purposes (CAL_AMP = 1, freq = fCLK/2^20)
    wreg(CONFIG3, 0xE8);    // 0xEC for internal bias reference signal, E8 for no bias
    wreg(BIAS_SENSP, 0x00); // 0xFF to add all channels to bias generation
    wreg(BIAS_SENSN, 0x00);
//    wreg(LOFF_SENSP, 0xFF); // 0xFF to enable lead-off detection on all channels
    wreg(MISC1, 0x00);      // 0x20 for SRB1 as reference for all channels, 0x00 for no SRB1 reference to all channels
//    wreg(CONFIG4, 0x02);    // 0x02 to turn on dc lead-off comparators
    wreg(CH1SET, 0x65);     // 0x60 for PGA Gain of 24, no SRB2 connection and normal electrode input, 0x65 for test input, 0x81 for deactivating the pin (short to GND)
    wreg(CH2SET, 0x81);     // 0x60 for PGA Gain of 24, no SRB2 connection and normal electrode input, 0x65 for test input, 0x81 for deactivating the pin (short to GND)
    wreg(CH3SET, 0x81);     // 0x60 for PGA Gain of 24, no SRB2 connection and normal electrode input, 0x65 for test input, 0x81 for deactivating the pin (short to GND)
    wreg(CH4SET, 0x81);     // 0x60 for PGA Gain of 24, no SRB2 connection and normal electrode input, 0x65 for test input, 0x81 for deactivating the pin (short to GND)
    wreg(CH5SET, 0x81);     // 0x60 for PGA Gain of 24, no SRB2 connection and normal electrode input, 0x65 for test input, 0x81 for deactivating the pin (short to GND)
    wreg(CH6SET, 0x81);     // 0x60 for PGA Gain of 24, no SRB2 connection and normal electrode input, 0x65 for test input, 0x81 for deactivating the pin (short to GND)
    wreg(CH7SET, 0x81);     // 0x60 for PGA Gain of 24, no SRB2 connection and normal electrode input, 0x65 for test input, 0x81 for deactivating the pin (short to GND)
    wreg(CH8SET, 0x81);     // 0x60 for PGA Gain of 24, no SRB2 connection and normal electrode input, 0x65 for test input, 0x81 for deactivating the pin (short to GND)
    output_count = 0;
    return "ADS1299 initalized";
}
void ADS1299::spi_start(uint32_t _CS, SPISettings _SPI_settings) {
    SPI.beginTransaction(_SPI_settings);
    digitalWrite(_CS, LOW);  // Start communication
}
void ADS1299::spi_end(uint32_t _CS) {
    delayMicroseconds(2);    // This delay is necessary on the Teensy 4.0 in order to assure communication finished
    digitalWrite(_CS, HIGH); // End communication
    SPI.endTransaction();
}
void ADS1299::send_command(uint8_t cmd) {
    spi_start(cs, ADS_SPI_settings); // Start communication
    SPI.transfer(cmd);
    spi_end(cs);                     // End communication
}

// System Commands
void ADS1299::wakeup() {
    send_command(_WAKEUP);
    delayMicroseconds(3);   // must wait 4 tCLK cycles before sending another command (Datasheet, pg. 40)
}
void ADS1299::standby() {
    send_command(_STANDBY);
}
String ADS1299::reset() {
    send_command(_RESET);
    delayMicroseconds(10);  // must wait 18 tCLK cycles to execute this command (Datasheet, pg. 41)
    return "ADS1299 reset";
}
String ADS1299::start() {
    send_command(_START);
    return "ADS1299 data conversion started";
}
String ADS1299::stop() {
    send_command(_STOP);
    return "ADS1299 data conversion stopped";
}

// Data Read Commands
void ADS1299::rdatac() {
    send_command(_RDATAC);
}
void ADS1299::sdatac() {
    send_command(_SDATAC);
    delayMicroseconds(3);   // must wait 4 tCLK cycles before sending another command (Datasheet, pg. 42)
}
void ADS1299::rdata() {
    send_command(_RDATA);
}

// Register Read/Write Commands
uint8_t ADS1299::get_id() {
    spi_start(cs, ADS_SPI_settings);    // Start communication
    SPI.transfer(_SDATAC);              // Stop data conversion
    SPI.transfer(_RREG);                // Start reading register
    SPI.transfer(0x00);                 // Asking for 1 byte
    uint8_t data = SPI.transfer(0x00);  // Byte to read
    SPI.transfer(_RDATAC);              // Restart data conversion
    spi_end(cs);                        // End communication
    return data;
}
String ADS1299::rreg(uint8_t address) {
    String return_value = "";
    uint8_t opcode1 = _RREG + address;  // 001rrrrr: _RREG = 00100000, adress = rrrrr
    spi_start(cs, ADS_SPI_settings);    // Start communication
    SPI.transfer(_SDATAC);              // Stop data conversion
    SPI.transfer(opcode1);              // Start reading register
    SPI.transfer(0x00);                 // opcode2
    uint8_t data = SPI.transfer(0x00);  // returned byte should match default of register map unless edited manually
    SPI.transfer(_RDATAC);              // Restart data conversion
    spi_end(cs);                        // End communication

    return_value = print_reg_name(address);
    return_value += "0x";
    if (address < 16) return_value += "0";
    return_value += String(address, HEX);
    return_value += ", 0x";
    if (data < 16) return_value += "0";
    return_value += String(data, HEX);
    return_value += ", ";
    for (uint8_t j = 0; j < 8; j++) {
        return_value += String(bitRead(data, 7 - j), BIN);
        if (j != 7) return_value += " ";
    }
    return (return_value += "\n");
}
String ADS1299::wreg(uint8_t address, uint8_t value) {
    String return_value = "";
    uint8_t opcode1 = _WREG + address;  // 010rrrrr: _WREG = 01000000, adress = rrrrr (address offset)
    spi_start(cs, ADS_SPI_settings);    // Start communication
    SPI.transfer(_SDATAC);              // Stop data conversion
    SPI.transfer(opcode1);              // Start reading register
    SPI.transfer(0x00);                 // opcode2 (number of registers to write - 1, and therefore 0)
    SPI.transfer(value);                // value to write to register
    SPI.transfer(_RDATAC);              // Restart data conversion
    spi_end(cs);                        // End communication

    return_value = "Register 0x";
    if (address < 16) return_value += "0";
    return_value += String(address, HEX);
    return (return_value += " modified.\n");
}

String ADS1299::print_reg_name(uint8_t address) {
    String return_value;
    switch (address) {
        case EEG_ID:
            return_value = "ID, ";
            break;
        case CONFIG1:
            return_value = "CONFIG1, ";
            break;
        case CONFIG2:
            return_value = "CONFIG2, ";
            break;
        case CONFIG3:
            return_value = "CONFIG3, ";
            break;
        case LOFF:
            return_value = "LOFF, ";
            break;
        case CH1SET:
            return_value = "CH1SET, ";
            break;
        case CH2SET:
            return_value = "CH2SET, ";
            break;
        case CH3SET:
            return_value = "CH3SET, ";
            break;
        case CH4SET:
            return_value = "CH4SET, ";
            break;
        case CH5SET:
            return_value = "CH5SET, ";
            break;
        case CH6SET:
            return_value = "CH6SET, ";
            break;
        case CH7SET:
            return_value = "CH7SET, ";
            break;
        case CH8SET:
            return_value = "CH8SET, ";
            break;
        case BIAS_SENSP:
            return_value = "BIAS_SENSP, ";
            break;
        case BIAS_SENSN:
            return_value = "BIAS_SENSN, ";
            break;
        case LOFF_SENSP:
            return_value = "LOFF_SENSP, ";
            break;
        case LOFF_SENSN:
            return_value = "LOFF_SENSN, ";
            break;
        case LOFF_FLIP:
            return_value = "LOFF_FLIP, ";
            break;
        case LOFF_STATP:
            return_value = "LOFF_STATP, ";
            break;
        case LOFF_STATN:
            return_value = "LOFF_STATN, ";
            break;
        case GPIO:
            return_value = "GPIO, ";
            break;
        case MISC1:
            return_value = "MISC1, ";
            break;
        case MISC2:
            return_value = "MISC2, ";
            break;
        case CONFIG4:
            return_value = "CONFIG4, ";
            break;
        default:
            break;
    }
    return return_value;
}

// This method moves the data out from the ADS1299 using standard SPI
// It is only reliable for data rates up to 2 kSPS
void ADS1299::update_data() {
    uint32_t i, j;
    uint32_t byte_counter = 0;
    uint8_t data_byte;
    int32_t chan_data = 0;
    ads_status = 0;
    //data_ready = false;

    spi_start(cs, ADS_SPI_settings);    // Start communication
    for (i = 0; i < 3; i++) {
        data_byte = SPI.transfer(0x00); // read status register (1100 + LOFF_STATP + LOFF_STATN + GPIO[7:4])
        ads_status = (ads_status << 8) | data_byte;
    }
    // read 24 bytes of channel data in 8 3 byte chunks
    for (i = 0; i < CHANNELS_PER_SAMPLE; i++) {
        // read 24 bits per channel in 3 byte chunks
        for (j = 0; j < BYTES_PER_CHANNEL; j++) {
            data_byte = SPI.transfer(0x00);
            raw_channel_data[byte_counter] = data_byte;
            byte_counter++;
            chan_data = (chan_data << 8) | data_byte;
        }
        channel_data[i] = chan_data;
        chan_data = 0;
    }
    spi_end(cs);                        // End communication

    // convert 3 byte 2's compliment to 4 byte 2's compliment
    // this is necessary if doing any data manipulation in main
    for (i = 0; i < CHANNELS_PER_SAMPLE; i++) {
        if (bitRead(channel_data[i], 23) == 1) {
            channel_data[i] |= 0xFF000000;
        }
        else {
            channel_data[i] &= 0x00FFFFFF;
        }
    }
    output_count++;
}

bool ADS1299::init() {
    logger.print_message("In ADS1299 Initalization");

    pinMode(rst, OUTPUT);
    digitalWrite(rst, HIGH);

    pinMode(strt, OUTPUT);
    // must hold start pin low to use SPI start command
    digitalWrite(strt, LOW);

    pinMode(pwrdwn, OUTPUT);
    digitalWrite(pwrdwn, HIGH);

    setup(drdy, cs);

    return true;
}

void ADS1299::begin() {
    logger.print_message("Starting ADS1299 Sampling");
    rdatac();
    start();
}

void ADS1299::end() {
    logger.print_message("Stopping ADS1299 Sampling");
    stop();
    delay(1);
    sdatac();
    delay(1);
}
