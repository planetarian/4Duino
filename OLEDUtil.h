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
    static String byteToString(uint8_t data); // "0x12"
    static String shortToString(uint16_t data); // "0x1234"
    static String longToString(uint32_t data); // "0x12345678"
    static float analogToVoltage(uint16_t value, float refVoltage = 5.0); // e.g. 4.98f
    static String analogToVoltageString(uint16_t value, float refVoltage = 5.0); // e.g. "4.98"
    static String floatToString(float value, uint8_t decimalPlaces = 2);
    static uint32_t scaleAnalog(uint32_t value, uint32_t targetScale); // shortcut to convertValueScale(value, 1024, targetScale)
    static uint32_t convertValueScale(uint32_t value, uint32_t valueScale, uint32_t targetScale);
private:
    OLEDUtil();
    ~OLEDUtil();
};

#endif
