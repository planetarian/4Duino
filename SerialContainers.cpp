#include "SerialContainers.h"

SerialContainer::SerialContainer(){}



HardwareSerialContainer::HardwareSerialContainer(HardwareSerial &serial)
    : _serial(serial), SerialContainer() {}
void HardwareSerialContainer::begin(uint32_t baudRate) {_serial.begin(baudRate); }
void HardwareSerialContainer::end() { _serial.end(); }
int HardwareSerialContainer::available() { return _serial.available(); }
int HardwareSerialContainer::peek() { return _serial.peek(); }
int HardwareSerialContainer::read() { return _serial.read(); }
void HardwareSerialContainer::flush() { _serial.flush(); }
bool HardwareSerialContainer::overflow() { return false; }
size_t HardwareSerialContainer::write(uint8_t data) { return _serial.write(data); }

SoftwareSerialContainer::SoftwareSerialContainer(SoftwareSerial &serial)
    : _serial(serial), SerialContainer() {}
void SoftwareSerialContainer::begin(uint32_t baudRate) { _serial.begin(baudRate); }
void SoftwareSerialContainer::end() { _serial.end(); }
int SoftwareSerialContainer::available() { return _serial.available(); }
int SoftwareSerialContainer::peek() { return _serial.peek(); }
int SoftwareSerialContainer::read() { return _serial.read(); }
void SoftwareSerialContainer::flush() { _serial.flush(); }
bool SoftwareSerialContainer::overflow() { return _serial.overflow(); }
size_t SoftwareSerialContainer::write(uint8_t data) { return _serial.write(data); }