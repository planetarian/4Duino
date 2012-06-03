#include "Color.h"

#include "OLEDUtil.h"

Color::Color()
{
    _init(0,0,0);
}

Color::Color(uint8_t red, uint8_t green, uint8_t blue)
{
    _init(red, green, blue);
}

void Color::_init(uint8_t red, uint8_t green, uint8_t blue)
{
    setRed(red);
    setGreen(green);
    setBlue(blue);
}


Color Color::fromRGB(uint8_t red, uint8_t green, uint8_t blue)
{
    return Color(red, green, blue);
}

Color Color::from16BitRGB(uint16_t colorShort)
{
    uint8_t redComponent = colorShort >> 11;
    uint8_t greenComponent = colorShort >> 5 & 0x3f;
    uint8_t blueComponent = colorShort & 0x1f;
    return Color(
        redComponent << 3,
        greenComponent << 2,
        blueComponent << 3);
}

Color Color::from32BitRGB(uint32_t colorLong)
{
    return Color(
        colorLong >> 16 & 0xFF,
        colorLong >> 8 & 0xFF,
        colorLong & 0xFF);
}


Color Color::rand()
{
    return Color(random(255),random(255),random(255));
}

Color Color::rand(uint8_t max)
{
    return Color(random(max),random(max),random(max));
}

Color Color::rand(uint8_t min, uint8_t max)
{
    return Color(random(min,max),random(min,max),random(min,max));
}


Color Color::blend(Color color1, Color color2, uint8_t color1Amount)
{
    return from32BitRGB(blend32Bit(color1.to32BitRGB(), color2.to32BitRGB(), color1Amount));
}

// blends two colors together using 32-bit color values.
// color1Amount determines how dominant color1 is in the blend.
uint32_t Color::blend32Bit(uint32_t color1, uint32_t color2, uint8_t color1Amount)
{
    // Separate the green from the red/blue,
    // use the space left of the color values to multiply
    uint32_t rbMask = 0x00FF00FF;
    uint32_t gMask = 0x0000FF00;
    uint32_t rbFinalMask = 0xFF00FF00;
    uint32_t gFinalMask = 0x00FF0000;
    uint8_t color2Amount = 256 - color1Amount;
    
    // Remove whatever is in the original color positions,
    // leaving the most significant bits of each color
    uint32_t rb =
        (((color1 & rbMask) * color1Amount) +
        ((color2 & rbMask) * color2Amount)) &
        rbFinalMask;
    uint32_t g =
        (((color1 & gMask) * color1Amount) +
        ((color2 & gMask) * color2Amount)) &
        gFinalMask;

    // Recombine and shift back over to the standard positions
    return (rb | g) >> 8;
}

uint16_t Color::blend16Bit(uint16_t color1, uint16_t color2, uint8_t color1Amount)
{
    // TODO: find a simple method of blending 16-bit colors, this is icky =/
    return to16BitRGB(
        blend32Bit( to32BitRGB(color1), to32BitRGB(color2), color1Amount)
        );
}


uint16_t Color::to16BitRGB()
{
    return to16BitRGB(_red, _green, _blue);
}

uint16_t Color::to16BitRGB(uint8_t red, uint8_t green, uint8_t blue)
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

uint16_t Color::to16BitRGB(uint32_t colorLong)
{
    return to16BitRGB(
        OLEDUtil::getByte(colorLong, 2),
        OLEDUtil::getByte(colorLong, 1),
        OLEDUtil::getByte(colorLong, 0));
}


uint32_t Color::to32BitRGB()
{
    return to32BitRGB(_red, _green, _blue);
}

uint32_t Color::to32BitRGB(uint8_t red, uint8_t green, uint8_t blue)
{
    return ((uint32_t)red << 16) + ((uint32_t)green << 8) + blue;
}

uint32_t Color::to32BitRGB(uint16_t colorShort)
{
    return to32BitRGB(
        colorShort >> 11,
        colorShort >> 5 & 0x3f,
        colorShort & 0x1f);
}


uint8_t Color::getRed()
{
    return _red;
}

uint8_t Color::getGreen()
{
    return _green;
}

uint8_t Color::getBlue()
{
    return _blue;
}


void Color::setRed(uint8_t value)
{
    _red = value;
}

void Color::setGreen(uint8_t value)
{
    _green = value;
}

void Color::setBlue(uint8_t value)
{
    _blue = value;
}

