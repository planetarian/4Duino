#include "OLEDUtil.h"

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

void OLEDUtil::BlinkLED(uint8_t pin, uint8_t numTimes)
{
    digitalWrite(pin, LOW);
    for (uint8_t i = 0; i < numTimes; i++)
    {
        delay(100);
        digitalWrite(pin, HIGH);
        delay(100);
        digitalWrite(pin, LOW);
    }
    delay(200);
}

String OLEDUtil::ByteToString(uint8_t data)
{
    const char *table = "0123456789ABCDEF";
    char result[3];
    result[0] = table[data >> 4 & 0x0F];
    result[1] = table[data & 0x0F];
    result[2] = '\0';
    return result;
}

String OLEDUtil::ShortToString(uint16_t data)
{
    return
        ByteToString(GetByte(data,1)) +
        ByteToString(GetByte(data,0));
}

String OLEDUtil::LongToString(uint32_t data)
{
    return
        ShortToString(data >> 16) +
        ShortToString(data & 0xFFFF);
}
