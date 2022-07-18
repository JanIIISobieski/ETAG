#include <SerialCommunicator.h>

SerialCommunicator::~SerialCommunicator() {

}

int SerialCommunicator::read() {
    if (_serial->available()) {
        read_byte = _serial->read();
        return read_byte;
    }
    else {
        return -1;
    } 
}

size_t SerialCommunicator::read(uint8_t* buffer, size_t size) {
    size_t count = 0;
    while (count < size) {
        int read_val = this->read();
        if (read_val != -1) {
            *(buffer+count) = read_val;
            ++count;
        }
    }
    return count;
}

bool SerialCommunicator::await_acknowledgment() {
    while ((uint8_t)(read() & 0xFF) != 6) { // 6 for ACK
        delay(10);
    }
    return true; 
}

size_t SerialCommunicator::write(uint8_t val) {
    return this->_serial->write(val);
}

size_t SerialCommunicator::write(uint8_t* buffer, size_t size) {
    return this->_serial->write(buffer, size);
}

size_t SerialCommunicator::write(const uint8_t buffer[], size_t size) {
    return this->_serial->write((uint8_t*) buffer, size);
}

size_t SerialCommunicator::write(uint16_t* buffer, size_t size) {
    uint8_t* ptr = (uint8_t *) buffer;
    return this->write(ptr, size);
}

size_t SerialCommunicator::print(String string) {
    return _serial->print(string);
}

size_t SerialCommunicator::println(String string) {
    return _serial->println(string);
}

size_t SerialCommunicator::println(unsigned char n, int base) {
    return _serial->println(n, base);
}

String SerialCommunicator::readStringUntil(char terminator, size_t max) {
    return _serial->readStringUntil(terminator, max);
}