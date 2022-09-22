#include "EEGCommands.h"

void EEGCommands::write_all_registers() {

}

void EEGCommands::write_register(uint8_t address, uint8_t value) {
    uint8_t opcode1 = _WREG + address;  // 010rrrrr: _WREG = 01000000, adress = rrrrr (address offset)
    spi_start();                        // Start communication  
    SPI.transfer(0x11);              // Stop data conversion
    SPI.transfer(opcode1);              // Start reading register 
    SPI.transfer(0x00);                 // opcode2 (number of registers to write - 1, and therefore 0)
    SPI.transfer(value);                // value to write to register
    SPI.transfer(0x10);              // Restart data conversion
    spi_end();                          // End communication
}

void EEGCommands::read_register(uint8_t address) {
    uint8_t opcode1 = _RREG + address;  // 001rrrrr: _RREG = 00100000, adress = rrrrr
    spi_start();                        // Start communication
    SPI.transfer(0x11);              // Stop data conversion
    SPI.transfer(opcode1);              // Start reading register
    SPI.transfer(0x00);                 // opcode2
    uint8_t data = SPI.transfer(0x00);  // returned byte should match default of register map unless edited manually
    SPI.transfer(0x10);              // Restart data conversion
    spi_end();                          // End communication
}

void EEGCommands::spi_start() {
    SPI.beginTransaction(spi_settings);
    digitalWrite(cs, LOW);
}

void EEGCommands::spi_end() {
    delayMicroseconds(EEG_POST_SPI_TRANSFER_DELAY_MICROSECONDS);
    digitalWrite(cs, HIGH);
    SPI.endTransaction();
}

void EEGCommands::send_command(uint8_t command) {
    spi_start();
    SPI.transfer(command);
    spi_end();
}

// System Commands
void EEGCommands::wakeup() {
    send_command(0x02);
    delayMicroseconds(3);   // must wait 4 tCLK cycles before sending another command (Datasheet, pg. 40)
}

void EEGCommands::standby() {
    send_command(0x04);
}

void EEGCommands::reset() {
    send_command(0x06);
    delayMicroseconds(10);  // must wait 18 tCLK cycles to execute this command (Datasheet, pg. 41)
}

void EEGCommands::start() {
    send_command(0x08);
}

void EEGCommands::stop() {
    send_command(0x0A);
}

// Data Read Commands
void EEGCommands::rdatac() {
    send_command(0x10);
}

void EEGCommands::sdatac() {
    send_command(0x11);
    delayMicroseconds(3);   // must wait 4 tCLK cycles before sending another command (Datasheet, pg. 42)
}

void EEGCommands::rdata() {
    send_command(0x12);
}

void EEGCommands::get_id() {
    send_command(0x00);
}