#include "OLEDUtil.h"

// Gets an individual byte from a multibyte value.
uint8_t OLEDUtil::GetByte(uint32_t value, uint8_t zeroBasedByteFromRight)
{
    return value >> (zeroBasedByteFromRight * 8) & 0xFF;
}

// This reads a hex value literally as a number.
// e.g. 0x10 as 10 instead of 16
// Returns false if the value is invalid
// e.g. 0x1c
bool OLEDUtil::ReadHexAsDec(uint8_t value, uint8_t& result)
{
    uint8_t digit1 = value >> 4;
    uint8_t digit2 = value & 0x0F;
    if (digit1 > 9 || digit2 > 9)
        return false;
    
    result = (digit1 * 10) + digit2;
    return true;
}

// Blinks an LED. derp.
void OLEDUtil::BlinkLED(uint8_t pin, uint8_t numTimes, uint16_t blinkDelayMs)
{
    digitalWrite(pin, LOW);
    for (uint8_t i = 0; i < numTimes; i++)
    {
        delay(blinkDelayMs);
        digitalWrite(pin, HIGH);
        delay(blinkDelayMs);
        digitalWrite(pin, LOW);
    }
    delay(blinkDelayMs*2);
}

// Converts e.g. (uint8_t)0xFF to (String)"0xFF"
String OLEDUtil::ByteToString(uint8_t data)
{
    const char *table = "0123456789ABCDEF";
    char result[3];
    result[0] = table[data >> 4 & 0x0F];
    result[1] = table[data & 0x0F];
    result[2] = '\0';
    return result;
}

// Converts e.g. (uint16_t)0xFFFF to (String)"0xFFFF"
String OLEDUtil::ShortToString(uint16_t data)
{
    return
        ByteToString(GetByte(data,1)) +
        ByteToString(GetByte(data,0));
}

// Converts e.g. (uint32_t)0xFFFFFFFF to (String)"0xFFFFFFFF"
String OLEDUtil::LongToString(uint32_t data)
{
    return
        ShortToString(data >> 16) +
        ShortToString(data & 0xFFFF);
}

// Converts an analog reading to a voltage float value.
float OLEDUtil::AnalogToVoltage(uint16_t value, float refVoltage)
{
    return (refVoltage / 1024.0) * value;
}

// Scales an analog reading to a value based on another scale.
uint16_t OLEDUtil::ScaleAnalog(uint16_t value, uint16_t max)
{
    return ConvertValueScale(value, 1024, max);
}

// Converts the scale of a value to another scale.
uint16_t OLEDUtil::ConvertValueScale(uint16_t value, uint16_t valueScale, uint16_t targetScale)
{
    return targetScale * 1000 / valueScale * value / 1000;
}