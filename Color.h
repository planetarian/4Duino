#ifndef Color_h
#define Color_h

#include <inttypes.h>
#include "Colors.h"

struct Color
{
public:
    Color();
    Color(uint8_t red, uint8_t green, uint8_t blue);
    
    static Color FromRGB(uint8_t red, uint8_t green, uint8_t blue);
    static Color From16BitRGB(uint16_t colorShort);
    static Color From32BitRGB(uint32_t colorLong);

    static Color Random();
    static Color Random(uint8_t max);
    static Color Random(uint8_t min, uint8_t max);
    
    static Color Blend(Color color1, Color color2, uint8_t color1Amount);
    static uint32_t Blend32Bit(uint32_t color1, uint32_t color2, uint8_t color1Amount);
    static uint16_t Blend16Bit(uint16_t color1, uint16_t color2, uint8_t color1Amount);

    uint16_t To16BitRGB();
    static uint16_t To16BitRGB(uint8_t red, uint8_t green, uint8_t blue);
    static uint16_t To16BitRGB(uint32_t colorLong);
    
    uint32_t To32BitRGB();
    static uint32_t To32BitRGB(uint8_t red, uint8_t green, uint8_t blue);
    static uint32_t To32BitRGB(uint16_t colorShort);

    uint8_t GetRed();
    uint8_t GetGreen();
    uint8_t GetBlue();

    void SetRed(uint8_t value);
    void SetGreen(uint8_t value);
    void SetBlue(uint8_t value);

private:
    void init(uint8_t red, uint8_t green, uint8_t blue);

    bool _readonly;

    uint8_t _red;
    uint8_t _green;
    uint8_t _blue;
};

#endif