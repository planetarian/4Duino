#ifndef OLEDUtil_h
#define OLEDUtil_h

#include <inttypes.h>
#include <Arduino.h>

#define MAX_FLOAT_STRING_LENGTH 32

class OLEDUtil
{
public:
    static uint8_t getByte(uint32_t value, uint8_t zeroBasedByteFromRight = 0);
    static bool readHexAsDec(uint8_t value, uint8_t& result);
    static void blinkLED(uint8_t pin, uint8_t numTimes, uint16_t blinkDelayMs = 100);
    static String byteToString(uint8_t data);
    static String shortToString(uint16_t data);
    static String longToString(uint32_t data);
    static float analogToVoltage(uint16_t value, float refVoltage = 5.0);
    static String analogToVoltageString(uint16_t value, float refVoltage = 5.0);
    static String floatToString(float value, uint8_t decimalPlaces = 2);
    static uint32_t scaleAnalog(uint32_t value, uint32_t max);
    static uint32_t convertValueScale(uint32_t value, uint32_t valueScale, uint32_t targetScale);
    static uint16_t checkMemory();

private:
    OLEDUtil();
    ~OLEDUtil();
};

#endif
