#include "Color.h"

#include "OLEDUtil.h"

Color::Color()
{
    init(0,0,0);
}

Color::Color(uint8_t red, uint8_t green, uint8_t blue)
{
    init(red, green, blue);
}

void Color::init(uint8_t red, uint8_t green, uint8_t blue)
{
    SetRed(red);
    SetGreen(green);
    SetBlue(blue);
}


Color Color::FromRGB(uint8_t red, uint8_t green, uint8_t blue)
{
    return Color(red, green, blue);
}

Color Color::From16BitRGB(uint16_t colorShort)
{
    uint8_t redComponent = colorShort >> 11;
    uint8_t greenComponent = colorShort >> 5 & 0x3f;
    uint8_t blueComponent = colorShort & 0x1f;
    return Color(
        redComponent << 3,
        greenComponent << 2,
        blueComponent << 3);
}

Color Color::From32BitRGB(uint32_t colorLong)
{
    return Color(
        colorLong >> 16 & 0xFF,
        colorLong >> 8 & 0xFF,
        colorLong & 0xFF);
}


Color Color::Random()
{
    return Color(random(255),random(255),random(255));
}

Color Color::Random(uint8_t max)
{
    return Color(random(max),random(max),random(max));
}

Color Color::Random(uint8_t min, uint8_t max)
{
    return Color(random(min,max),random(min,max),random(min,max));
}


Color Color::Blend(Color color1, Color color2, uint8_t color1Amount)
{
    return From32BitRGB(Blend32Bit(color1.To32BitRGB(), color2.To32BitRGB(), color1Amount));
}

uint32_t Color::Blend32Bit(uint32_t color1, uint32_t color2, uint8_t color1Amount)
{
    uint32_t rbMask = 0x00FF00FF;
    uint32_t gMask = 0x0000FF00;
    uint32_t rbFinalMask = 0xFF00FF00;
    uint32_t gFinalMask = 0x00FF0000;
    uint8_t color2Amount = 256 - color1Amount;
    
    uint32_t rb =
        (((color1 & rbMask) * color1Amount) +
        ((color2 & rbMask) * color2Amount)) &
        rbFinalMask;
    uint32_t g =
        (((color1 & gMask) * color1Amount) +
        ((color2 & gMask) * color2Amount)) &
        gFinalMask;

    return (rb | g) >> 8;
}

uint16_t Color::Blend16Bit(uint16_t color1, uint16_t color2, uint8_t color1Amount)
{
    // TODO: find a simple method of blending 16-bit colors, this is icky =/
    return To16BitRGB( Blend32Bit( To32BitRGB(color1), To32BitRGB(color2), color1Amount) );
}


uint16_t Color::To16BitRGB()
{
    return To16BitRGB(_red, _green, _blue);
}

uint16_t Color::To16BitRGB(uint8_t red, uint8_t green, uint8_t blue)
{
    uint8_t r = red >> 3;
    uint8_t g = green >> 2;
    uint8_t b = blue >> 3;

    // 2 bytes (16 bits) define the background colour in RGB format:
    // R4R3R2R1R0 G5G4G3G2G1G0 B4B3B2B1B0 where:
    // msb : R4 R3 R2 R1 R0 G5 G4 G3
    // lsb : G2 G1 G0 B4 B3 B2 B1 B0

    uint16_t result = (r << 11) + (g << 5) + b;
    return result;
}

uint16_t Color::To16BitRGB(uint32_t colorLong)
{
    return To16BitRGB(
        OLEDUtil::GetByte(colorLong, 2),
        OLEDUtil::GetByte(colorLong, 1),
        OLEDUtil::GetByte(colorLong, 0));
}


uint32_t Color::To32BitRGB()
{
    return To32BitRGB(_red, _green, _blue);
}

uint32_t Color::To32BitRGB(uint8_t red, uint8_t green, uint8_t blue)
{
    return ((uint32_t)red << 16) + ((uint32_t)green << 8) + blue;
}

uint32_t Color::To32BitRGB(uint16_t colorShort)
{
    return To32BitRGB(
        colorShort >> 11,
        colorShort >> 5 & 0x3f,
        colorShort & 0x1f);
}


uint8_t Color::GetRed()
{
    return _red;
}

uint8_t Color::GetGreen()
{
    return _green;
}

uint8_t Color::GetBlue()
{
    return _blue;
}


void Color::SetRed(uint8_t value)
{
    _red = value;
}

void Color::SetGreen(uint8_t value)
{
    _green = value;
}

void Color::SetBlue(uint8_t value)
{
    _blue = value;
}

