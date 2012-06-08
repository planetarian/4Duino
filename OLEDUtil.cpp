#include "OLEDUtil.h"

// Gets an individual byte from a multibyte value.
uint8_t OLEDUtil::getByte(uint32_t value, uint8_t zeroBasedByteFromRight)
{
    return value >> (zeroBasedByteFromRight * 8) & 0xFF;
}

// This reads a hex value literally as a number.
// e.g. 0x10 as 10 instead of 16
// Returns false if the value is invalid
// e.g. 0x1c
bool OLEDUtil::readHexAsDec(uint8_t value, uint8_t& result)
{
    uint8_t digit1 = value >> 4;
    uint8_t digit2 = value & 0x0F;
    if (digit1 > 9 || digit2 > 9)
        return false;
    
    result = (digit1 * 10) + digit2;
    return true;
}

// Blinks an LED. derp.
void OLEDUtil::blinkLED(uint8_t pin, uint8_t numTimes, uint16_t blinkDelayMs)
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
String OLEDUtil::byteToString(uint8_t data)
{
    const char *table = "0123456789ABCDEF";
    char result[3];
    result[0] = table[data >> 4 & 0x0F];
    result[1] = table[data & 0x0F];
    result[2] = '\0';
    return result;
}

// Converts e.g. (uint16_t)0xFFFF to (String)"0xFFFF"
String OLEDUtil::shortToString(uint16_t data)
{
    return
        byteToString(getByte(data,1)) +
        byteToString(getByte(data,0));
}

// Converts e.g. (uint32_t)0xFFFFFFFF to (String)"0xFFFFFFFF"
String OLEDUtil::longToString(uint32_t data)
{
    return
        shortToString(data >> 16) +
        shortToString(data & 0xFFFF);
}

// Converts an analog reading to a voltage float value.
float OLEDUtil::analogToVoltage(uint16_t value, float refVoltage)
{
    return (refVoltage / 1024.0) * value;
}

String OLEDUtil::analogToVoltageString(uint16_t value, float refVoltage)
{
    return floatToString(analogToVoltage(value, refVoltage), 4);
}

String OLEDUtil::floatToString(float value, uint8_t decimalPlaces)
{
    int32_t valueInt = (int32_t) value;
    uint8_t count = decimalPlaces + 1; // Count the decimal
    bool negative = value < 0;
    if (negative)
    {
        valueInt = 0 - valueInt;
        count++; // Count the sign
    }
    do
    {
        count++;
        valueInt /= 10;
    }
    while (valueInt > 0);

    if (count > MAX_FLOAT_STRING_LENGTH - 1)
        return "Float value too large.";

    char str[MAX_FLOAT_STRING_LENGTH] = "";
    return dtostrf(value, count, decimalPlaces, str);
}

// Scales an analog reading to a value based on another scale.
uint32_t OLEDUtil::scaleAnalog(uint32_t value, uint32_t max)
{
    return convertValueScale(value, 1024, max);
}

// Converts the scale of a value to another scale.
uint32_t OLEDUtil::convertValueScale(uint32_t value, uint32_t valueScale, uint32_t targetScale)
{
    return 1000 * targetScale / valueScale * value / 1000;
}

uint16_t OLEDUtil::checkMemory() {
    uint8_t *heapptr, *stackptr;
    stackptr = (uint8_t *)malloc(4);          // use stackptr temporarily
    heapptr = stackptr;                     // save value of heap pointer
    free(stackptr);      // free up the memory again (sets stackptr to 0)
    stackptr =  (uint8_t *)(SP);           // save value of stack pointer
    return stackptr - heapptr;
}
