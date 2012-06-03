#ifndef OLEDUtil_h
#define OLEDUtil_h

#include <inttypes.h>
#include <Arduino.h>

class OLEDUtil
{
public:
    static uint8_t GetByte(uint32_t value, uint8_t zeroBasedByteFromRight = 0);
    static bool ReadHexAsDec(uint8_t value, uint8_t& result);
    static void BlinkLED(uint8_t pin, uint8_t numTimes);
    static String ByteToString(uint8_t data);
    static String ShortToString(uint16_t data);
    static String LongToString(uint32_t data);
private:
    OLEDUtil();
    ~OLEDUtil();
};

#endif
