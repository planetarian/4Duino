#ifndef OLEDUtil_h
#define OLEDUtil_h

#include <inttypes.h>
#include <Arduino.h>

class OLEDUtil
{
public:
    static uint8_t GetByte(uint32_t value, uint8_t zeroBasedByteFromRight = 0);
    static bool ReadHexAsDec(uint8_t value, uint8_t& result);
    static void BlinkLED(uint8_t pin, uint8_t numTimes, uint16_t blinkDelayMs = 100);
    static String ByteToString(uint8_t data);
    static String ShortToString(uint16_t data);
    static String LongToString(uint32_t data);
    static float AnalogToVoltage(uint16_t value, float refVoltage = 5.0);
    static uint16_t ScaleAnalog(uint16_t value, uint16_t max);
    static uint16_t ConvertValueScale(uint16_t value, uint16_t valueScale, uint16_t targetScale);

private:
    OLEDUtil();
    ~OLEDUtil();
};

#endif
