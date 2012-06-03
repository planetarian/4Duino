#ifndef SerialContainers_h
#define SerialContainers_h

#include <Arduino.h>
#include "SoftwareSerial.h"

class SerialContainer
{
public:
    SerialContainer();
    virtual void begin(uint32_t baudRate) = 0;
    virtual void end() = 0;
    virtual int available() = 0;
    virtual int peek() = 0;
    virtual int read() = 0;
    virtual void flush() = 0;
    virtual bool overflow() = 0;
    virtual size_t write(uint8_t data) = 0;
};

class HardwareSerialContainer : public SerialContainer
{
public:
    HardwareSerialContainer(HardwareSerial &serial);
    HardwareSerial _serial;
    void begin(uint32_t baudRate);
    void end();
    int available();
    int peek();
    int read();
    void flush();
    bool overflow();
    size_t write(uint8_t data);
};

class SoftwareSerialContainer : public SerialContainer
{
public:
    SoftwareSerialContainer(SoftwareSerial &serial);
    SoftwareSerial _serial;
    void begin(uint32_t baudRate);
    void end();
    int available();
    int peek();
    int read();
    void flush();
    bool overflow();
    size_t write(uint8_t data);
};

#endif