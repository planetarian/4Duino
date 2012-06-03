#ifndef Color_h
#define Color_h

#include <inttypes.h>
#include "Colors.h"

struct Color
{
public:
    Color();
    Color(uint8_t red, uint8_t green, uint8_t blue);
    
    static Color fromRGB(uint8_t red, uint8_t green, uint8_t blue);
    static Color from16BitRGB(uint16_t colorShort);
    static Color from32BitRGB(uint32_t colorLong);

    static Color rand();
    static Color rand(uint8_t max);
    static Color rand(uint8_t min, uint8_t max);
    
    static Color blend(Color color1, Color color2, uint8_t color1Amount);
    static uint32_t blend32Bit(uint32_t color1, uint32_t color2, uint8_t color1Amount);
    static uint16_t blend16Bit(uint16_t color1, uint16_t color2, uint8_t color1Amount);

    uint16_t to16BitRGB();
    static uint16_t to16BitRGB(uint8_t red, uint8_t green, uint8_t blue);
    static uint16_t to16BitRGB(uint32_t colorLong);
    
    uint32_t to32BitRGB();
    static uint32_t to32BitRGB(uint8_t red, uint8_t green, uint8_t blue);
    static uint32_t to32BitRGB(uint16_t colorShort);

    uint8_t getRed();
    uint8_t getGreen();
    uint8_t getBlue();

    void setRed(uint8_t value);
    void setGreen(uint8_t value);
    void setBlue(uint8_t value);

private:
    void _init(uint8_t red, uint8_t green, uint8_t blue);

    bool _readonly;

    uint8_t _red;
    uint8_t _green;
    uint8_t _blue;
};

#endif