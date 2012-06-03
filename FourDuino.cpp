#include "FourDuino.h"

uint8_t *heapptr, *stackptr;
uint16_t OLED::CheckMemory() {
  stackptr = (uint8_t *)malloc(4);          // use stackptr temporarily
  heapptr = stackptr;                     // save value of heap pointer
  free(stackptr);      // free up the memory again (sets stackptr to 0)
  stackptr =  (uint8_t *)(SP);           // save value of stack pointer
  return stackptr - heapptr;
}


OLED::OLED(uint8_t pinReset, HardwareSerial serial, uint32_t baudRate, uint16_t initDelay)
{
    _pinReset = pinReset;
    _baudRate = baudRate;
    _initDelay = initDelay;
    _serial = new HardwareSerialContainer(serial);
}

OLED::OLED(uint8_t pinReset, SoftwareSerial serial, uint32_t baudRate, uint16_t initDelay)
{
    _pinReset = pinReset;
    _baudRate = baudRate;
    _initDelay = initDelay;
    _serial = new SoftwareSerialContainer(serial);
}

OLED::~OLED()
{
    delete _serial;
    _serial = 0;
}

bool OLED::Init()
{
    _deviceType = "unknown";
    _hardwareRevision = 0;
    _firmwareRevision = 0;
    _deviceWidth = 0;
    _deviceHeight = 0;
    _fontSize = OLED_FONT_SMALL;
    _fontOpacity = OLED_FONT_TRANSPARENT;
    _fontProportional = OLED_FONT_NONPROPORTIONAL;


    for (uint8_t i = 0; i < 32; i++)
        charIndexList[i] = false;

    pinMode(_pinReset, OUTPUT);

    bool _run = false;
    for (uint8_t r = 0; r < OLED_INIT_RETRIES && !_run; r++)
    {
        Reset();
        // Wait for the OLED/SD to initialize
        delay(OLED_INIT_DELAY_MS);

        SerialBegin(_baudRate);
        // Let the OLED auto-detect baud rate
        Write(OLED_CMD_BAUD_AUTO);	
        _run = GetAck();
    }
    if (!_run)
        return false;

    GetDeviceInfo(false);
    if (_deviceWidth > 255 || _deviceHeight > 255)
    {
        // This device is not supported, x/y are out of bounds.
        // Reset to ensure no further commands can be used.
        Reset();
        return false;
    }
    SetFont(OLED_FONT_SMALL);
    SetFontOpacity(OLED_FONT_TRANSPARENT);
    SetFontProportional(OLED_FONT_NONPROPORTIONAL);
    SetFontColor(OLED_FONT_COLOR_DEFAULT);
    SetButtonOpacity(false);
    SetButtonColor(OLED_BUTTON_COLOR_DEFAULT);
    SetButtonFontColor(OLED_BUTTON_FONT_COLOR_DEFAULT);
    return true;
}

void OLED::Reset()
{
    digitalWrite(_pinReset, LOW);
    delay(OLED_RESET_DELAY_MS);
    digitalWrite(_pinReset, HIGH);
    delay(OLED_RESET_DELAY_MS);
}


//
// Helper functions
//

String OLED::ConvertDeviceType(uint8_t deviceTypeResponse)
{
    switch (deviceTypeResponse)
    {
    case OLED_DEVICETYPE_LCD:
        return "LCD";
    case OLED_DEVICETYPE_OLED:
        return "OLED";
    case OLED_DEVICETYPE_VGA:
        return "VGA";
    default:
        return "unknown";
    }
}

uint16_t OLED::ConvertResolution(uint8_t resolutionResponse)
{
    switch (resolutionResponse)
    {
    case OLED_RES_64:
        return 64;
    case OLED_RES_96:
        return 96;
    case OLED_RES_128:
        return 128;
    case OLED_RES_160:
        return 160;
    case OLED_RES_176:
        return 176;
    case OLED_RES_220:
        return 220;
    case OLED_RES_320:
        return 320;
    default:
        return 0;
    }
}



//
// OLED read/write functions
//

void OLED::SerialBegin(uint32_t baudRate)
{
    _serial->begin(baudRate);
}

void OLED::Write(uint8_t value)
{
    _serial->write(value);
}

void OLED::Write(uint8_t numValues, uint8_t value1, ...)
{
    Write(value1);

    va_list ap;
    uint8_t current = 1;
    va_start(ap, value1);
    while (current < numValues)
    {
        Write((uint8_t)va_arg(ap, int));
        current++;
    }
    va_end(ap);
}

void OLED::WriteShort(uint16_t value)
{
    Write(2, OLEDUtil::GetByte(value, 1), OLEDUtil::GetByte(value));
}

void OLED::WriteShort(uint8_t numValues, uint16_t value1, ...)
{
    WriteShort(value1);

    va_list ap;
    uint8_t current = 1;
    va_start(ap, value1);
    while (current < numValues)
    {
        WriteShort(va_arg(ap, uint16_t));
        current++;
    }
    va_end(ap);
}

void OLED::WriteLong(uint32_t value)
{
    Write(4,
        OLEDUtil::GetByte(value, 3),
        OLEDUtil::GetByte(value, 2),
        OLEDUtil::GetByte(value, 1),
        OLEDUtil::GetByte(value));
}

void OLED::WriteLong(uint8_t numValues, uint32_t value1, ...)
{
    WriteLong(value1);

    va_list ap;
    uint8_t current = 1;
    va_start(ap, value1);
    while (current < numValues)
    {
        WriteLong(va_arg(ap, uint16_t));
        current++;
    }
    va_end(ap);
}

void OLED::WriteText(char* text)
{
    for(uint32_t c = 0; c < strlen(text); c++)
    {
        if (text[c] == 0x00)
            return;
        Write(text[c]);
    }
}

void OLED::WriteString(String text)
{
    for(uint32_t c = 0; c < text.length(); c++)
    {
        if (text[c] == 0x00)
            return;
        Write(text[c]);
    }
}


bool OLED::GetResponse(uint8_t& result)
{
    for (uint32_t i = 0; i < OLED_RESPONSE_RETRIES; i++)
    {
        if (_serial->available())
        {
            result = _serial->read();
            return true;
        }
        delayMicroseconds(OLED_RESPONSE_RETRY_DELAY_US);
    }
    return false;
}

bool OLED::GetResponseShort(uint16_t& result)
{
    uint8_t byte1, byte2;
    if (!GetResponse(byte1) || !GetResponse(byte2))
        return false;
    result = ((uint16_t)byte1 << 8) + byte2;
    return true;
}

bool OLED::GetAck()
{
    uint8_t result;
    return (GetResponse(result) && result == OLED_ACK);
}



//
// OLED general/system commands
//



bool OLED::GetDeviceInfo(bool displayOnScreen)
{
    uint8_t output = displayOnScreen
        ? OLED_PRM_BOOL_TRUE
        : OLED_PRM_BOOL_FALSE;

    Write(2, OLED_CMD_INFO, output);

    uint8_t response[5];

    // NOTE: revision values may not be what you expect them to be;
    // I haven't got much to go by.
    // They seem to take the hex value as an integer:
    // 10 hex == 10 decimal.
    // TODO: Automatically convert to int.


    uint8_t hw, fw;
    if (!GetResponse(response[0]) ||
        !GetResponse(response[1]) || 
        !GetResponse(response[2]) ||	
        !GetResponse(response[3]) ||
        !GetResponse(response[4]) ||
        !OLEDUtil::ReadHexAsDec(response[1], hw) ||
        !OLEDUtil::ReadHexAsDec(response[2], fw))
        return false;
    

    _deviceType = ConvertDeviceType(response[0]);
    _hardwareRevision = hw;
    _firmwareRevision = fw;
    _deviceWidth = ConvertResolution(response[3]);
    _deviceHeight = ConvertResolution(response[4]);

    return true;
}

String OLED::GetDeviceType() { return _deviceType; }
uint8_t OLED::GetDeviceWidth()
{ 
    return _deviceWidth > 255
        ? 0 : (uint8_t)_deviceWidth;
}
uint8_t OLED::GetDeviceHeight()
{
    return _deviceHeight > 255
        ? 0 : (uint8_t)_deviceHeight;
}
uint8_t OLED::GetHardwareRevision() { return _hardwareRevision; }
uint8_t OLED::GetFirmwareRevision() { return _firmwareRevision; }


bool OLED::Clear()
{
    Write(OLED_CMD_CLEAR_SCREEN);
    return GetAck();
}


bool OLED::SetPower(bool on)
{
    Write(3,OLED_CMD_CTLFUNC, OLED_PRM_CTLFUNC_POWER,
        on ? OLED_PRM_CTLFUNC_POWER_ON : OLED_PRM_CTLFUNC_POWER_OFF);
    return GetAck();
}

bool OLED::On()
{
    return SetPower(true);
}

bool OLED::Off()
{
    return SetPower(false);
}

bool OLED::SetContrast(uint8_t value)
{
    // Accepts 0-255 but will simply convert it to 0-16
    Write(3, OLED_CMD_CTLFUNC, OLED_PRM_CTLFUNC_CONTRAST, value >> 4);
    return GetAck();
}

bool OLED::LowPowerShutdown()
{
    Write(3, OLED_CMD_CTLFUNC, OLED_PRM_CTLFUNC_LOWPOWER,
        OLED_PRM_CTLFUNC_LOWPOWER_SHUTDOWN);
    return GetAck();
}

bool OLED::LowPowerPowerUp()
{
    Write(3, OLED_CMD_CTLFUNC, OLED_PRM_CTLFUNC_LOWPOWER,
        OLED_PRM_CTLFUNC_LOWPOWER_POWERUP);
    return GetAck();
}


bool OLED::TurnOffSD()
{
    Write(3, OLED_CMD_SLEEP, OLED_PRM_SLEEP_SD_OFF, OLED_PRM_NA);
    return GetAck();
}

bool OLED::WakeOnJoystick()
{
    Write(3, OLED_CMD_SLEEP, OLED_PRM_SLEEP_WAKE_JOY, OLED_PRM_NA);
    return GetAck();
}

bool OLED::WakeOnSerial()
{
    Write(3, OLED_CMD_SLEEP, OLED_PRM_SLEEP_WAKE_SERIAL, OLED_PRM_NA);
    return GetAck();
}


// TODO: NOT IMPLEMENTED YET
bool OLED::Joystick()
{
    /*
    Joystick(option)
    This command returns the status of the Buttons-Joystick
    in several options.

    option:
    08hex : Return Buttons-Joystick Status
    0Fhex : Wait for Buttons-Joystick to be pressed and released
    00hex : Wait until any Buttons-Joystick pressed
    01hex : Wait until SW1 (UP) released.
    02hex : Wait until SW2 (LEFT) released.
    03hex : Wait until SW3 (DOWN) released.
    04hex : Wait until SW4 (RIGHT) released.
    05hex : Wait until SW5 (FIRE) released.

    response:
    00hex : No Buttons pressed (or pressed button has been released).
    01hex : SW1 (UP) pressed.
    02hex : SW2 (LEFT) pressed.
    03hex : SW3 (DOWN) pressed.
    04hex : SW4 (RIGHT) pressed.
    05hex : SW5 (FIRE) pressed.
    */
    return false;
}

// TODO: NOT IMPLEMENTED YET
bool OLED::JoystickWait()
{
    /*
    WaitJoystick(option,waitTime)
    This command asks for the status of the Buttons-Joystick
    in several options with a wait time.

    option:
    00hex : Wait until any Buttons-Joystick pressed.
    01hex : Wait until SW1 (UP) released.
    02hex : Wait until SW2 (LEFT) released.
    03hex : Wait until SW3 (DOWN) released.
    04hex : Wait until SW4 (RIGHT) released.
    05hex : Wait until SW5 (FIRE) released.

    waitTime:
    2 bytes (big endian) define the wait time (in milliseconds).

    response:
    00hex : Time-Out (or Button released).
    01hex : SW1 (UP) pressed.
    02hex : SW2 (LEFT) pressed.
    03hex : SW3 (DOWN) pressed.
    04hex : SW4 (RIGHT) pressed.
    */
    return false;
}


// TODO: NOT IMPLEMENTED YET
bool OLED::Sound(uint16_t note, uint16_t duration)
{
    /*
    Sound(note, duration)
    This command will generate a specified note or frequency
    for a certain duration.

    note:
    2 bytes (big endian) define the note or frequency of the sound.
    0 : No sound, silence.
    1-84 : 5 octaves piano range + 2 more.
    100-20000 : Frequency in Hz.

    duration:
    2 bytes (big endian) define the duration of the note
    (in milliseconds).

    response:
    acknowledge
    */
    return false;
}

// TODO: NOT IMPLEMENTED YET
// Use varargs?
bool OLED::Tune(uint8_t length, uint16_t note, uint16_t duration)
{
    /*
    Tune(length, note1, duration1, note2, duration2, ..... noteN, durationN)
    This command will generate a sequence of specified note or frequency
    for a specified duration.

    length:
    1byte, Number of note/duration pairs to follow: Maximum 64.

    note:
    2 bytes (big endian) define the note or frequency of the sound.
    0 : No sound, silence.
    1-84 : 5 octaves piano range + 2 more.
    100-20000 : Frequency in Hz.

    duration:
    2 bytes (big endian) define the duration of the note (in milliseconds).

    response:
    acknowledge
    */
    return false;
}



//
// Graphics commands
//

bool OLED::ReadPixel(uint8_t x, uint8_t y, uint16_t& resultShort)
{
    if (x < 0 || x >= _deviceWidth ||
        y < 0 || y >= _deviceHeight)
        return false;
    Write(3, OLED_CMD_READ_PIXEL, x, y);
    
    if (!GetResponseShort(resultShort))
        return false;
        
    return true;
}

bool OLED::ReadPixel(uint8_t x, uint8_t y, Color& resultColor)
{
    uint16_t resultShort;
    if (!ReadPixel(x, y, resultShort))
        return false;
    resultColor = Color::From16BitRGB(resultShort);
    return true;
}


bool OLED::DrawPixel(uint8_t x, uint8_t y, uint16_t color)
{
    Write(3, OLED_CMD_DRAW_PIXEL, x, y);
    WriteShort(color);
    return GetAck();
}

bool OLED::DrawPixel(uint8_t x, uint8_t y, Color color)
{
    return DrawPixel(x, y, color.To16BitRGB());
}


bool OLED::DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color)
{
    Write(5, OLED_CMD_DRAW_LINE, x1, y1, x2, y2);
    WriteShort(color);
    return GetAck();
}

bool OLED::DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, Color color)
{
    return DrawLine(x1, y1, x2, y2, color.To16BitRGB());
}


bool OLED::DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color)
{
    Write(5, OLED_CMD_DRAW_RECTANGLE, x1, y1, x2, y2);
    WriteShort(color);
    return GetAck();
}

bool OLED::DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, Color color)
{
    return DrawRectangle(x1,y1,x2,y2,color.To16BitRGB());
}

bool OLED::DrawRectangleWH(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color)
{
    if (width < 1 || height < 1)
        return false;
    uint8_t x2 = x + (width-1);
    uint8_t y2 = y + (height-1);
    return DrawRectangle(x,y,x2,y2,color);
}

bool OLED::DrawRectangleWH(uint8_t x, uint8_t y, uint8_t width, uint8_t height, Color color)
{
    return DrawRectangleWH(x,y,width,height,color.To16BitRGB());
}

bool OLED::DrawProgressBar(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
    uint8_t progressPercent, uint16_t foreColor, uint16_t backColor)
{
    uint8_t progressWidth = width * progressPercent / 100;
    return
        DrawRectangleWH(x, y, progressWidth, height, foreColor) &&
        DrawRectangleWH(x+progressWidth, y, width-progressWidth, height, backColor);
}

bool OLED::DrawProgressBar(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
    uint8_t progressPercent, Color foreColor, Color backColor)
{
    return DrawProgressBar(x, y, width, height,
        progressPercent, foreColor.To16BitRGB(), backColor.To16BitRGB());
}


bool OLED::DrawTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3,
    uint16_t color)
{
    Write(7, OLED_CMD_DRAW_TRIANGLE, x1, y1, x2, y2, x3, y3);
    WriteShort(color);
    return GetAck();
}

bool OLED::DrawTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3,
    Color color)
{
    return DrawTriangle(x1,y1,x2,y2,x3,y3,color.To16BitRGB());
}


bool OLED::DrawPolygonV(uint16_t color, uint8_t numVertices, uint8_t x1, uint8_t y1, va_list ap)
{
    // No more than 7 vertices
    if (numVertices > 7)
        return false;
    // Polygon must be at least 3 vertices, but no reason to flat-out reject it...
    if (numVertices == 1)
        return DrawPixel(x1, y1, color);
    if (numVertices == 2)
        return DrawLine(x1, y1, (uint8_t)va_arg(ap, int), (uint8_t)va_arg(ap, int), color);


    Write(4, OLED_CMD_DRAW_POLYGON, numVertices, x1, y1);
    for (uint8_t i = 1; i < numVertices; i++)
        Write(2, (uint8_t)va_arg(ap, int), (uint8_t)va_arg(ap, int));
    WriteShort(color);
    return GetAck();
}

bool OLED::DrawPolygon(uint16_t color, uint8_t numVertices, uint8_t x1, uint8_t y1, ...)
{
    va_list ap;
    va_start(ap, y1);
    bool result = DrawPolygonV(color, numVertices, x1, y1, ap);
    va_end(ap);
    return result;
}

bool OLED::DrawPolygon(Color color, uint8_t numVertices, uint8_t x1, uint8_t y1, ...)
{
    va_list ap;
    va_start(ap, y1);
    bool result = DrawPolygonV(color.To16BitRGB(), numVertices, x1, y1, ap);
    va_end(ap);
    return result;
}

bool OLED::DrawPolygon(uint16_t color, uint8_t numVertices, uint8_t vertices[][2])
{
    if (numVertices > 7) return false;
    if (numVertices == 1)
        return DrawPixel(vertices[0][0], vertices[0][1], color);
    if (numVertices == 2)
        return DrawLine(vertices[0][0], vertices[0][1], vertices[1][0], vertices[1][1], color);
    
    Write(2, OLED_CMD_DRAW_POLYGON, numVertices);
    for (uint8_t v = 0; v < numVertices; v++)
        Write(2, vertices[v][0], vertices[v][1]);
    WriteShort(color);
    return GetAck();
}

bool OLED::DrawPolygon(Color color, uint8_t numVertices, uint8_t vertices[][2])
{
    return DrawPolygon(color.To16BitRGB(), numVertices, vertices);
}

bool OLED::DrawCircle(uint8_t x, uint8_t y, uint8_t radius, uint16_t color)
{
    Write(4, OLED_CMD_DRAW_CIRCLE, x, y, radius);
    WriteShort(color);
    return GetAck();
}

bool OLED::DrawCircle(uint8_t x, uint8_t y, uint8_t radius, Color color)
{
    return DrawCircle(x, y, radius, color.To16BitRGB());
}

// NOT YET IMPLEMENTED
// The width MUST match the width of the original image to display correctly.
// Height can be anything <= original height, image will be truncated accordingly.
bool OLED::DrawImage8Bit(uint8_t x, uint8_t y, uint8_t imageWidth, uint8_t imageHeight,
        uint8_t pixel1, ...)
{
    return false;
    //Write(6, OLED_CMD_DRAW_IMAGE, x, y, imageWidth, imageHeight, OLED_PRM_DRAW_IMAGE_8BIT);
    // TODO: process/send pixel data
    //Write(imageWidth*imageHeight, pixel1, ...);
    //return GetAck();
}

// NOT YET IMPLEMENTED
// The width MUST match the width of the original image to display correctly.
// Height can be anything <= original height, image will be truncated accordingly.
bool OLED::DrawImage16Bit(uint8_t x, uint8_t y, uint8_t imageWidth, uint8_t imageHeight,
        uint16_t pixel1, ...)
{
    return false;
    //Write(6, OLED_CMD_DRAW_IMAGE, x, y, imageWidth, imageHeight, OLED_PRM_DRAW_IMAGE_16BIT);
    // TODO: process/send pixel data
    //WriteShort(imageWidth*imageHeight, pixel1, ...);
    //return GetAck();
}

bool OLED::AddUserBitmap(uint8_t charIndex,
    uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4,
    uint8_t data5, uint8_t data6, uint8_t data7, uint8_t data8)
{
    charIndexList[charIndex] = true;
    Write(10, OLED_CMD_ADD_USER_BITMAP, charIndex,
        data1, data2, data3, data4, data5, data6, data7, data8);
    return GetAck();
}

bool OLED::DrawUserBitmap(uint8_t charIndex, uint8_t x, uint8_t y, uint16_t color)
{
    if (!charIndexList[charIndex])
        return false;

    Write(4, OLED_CMD_DRAW_USER_BITMAP, charIndex, x, y);
    WriteShort(color);
    return GetAck();
}

bool OLED::DrawUserBitmap(uint8_t charIndex, uint8_t x, uint8_t y, Color color)
{
    return DrawUserBitmap(charIndex, x, y, color.To16BitRGB());
}

bool OLED::SetFill(bool fillShapes)
{
    Write(2, OLED_CMD_SET_PEN_SIZE,
        fillShapes ? OLED_PRM_SET_PEN_SIZE_FILL : OLED_PRM_SET_PEN_SIZE_EMPTY);
    return GetAck();
}

bool OLED::ScreenCopyPaste(uint8_t sourceX, uint8_t sourceY, uint8_t destX, uint8_t destY,
    uint8_t sourceWidth, uint8_t sourceHeight)
{
    Write(7, OLED_CMD_SCREEN_COPY_PASTE, sourceX, sourceY, destX, destY,
        sourceWidth, sourceHeight);
    return GetAck();
}


bool OLED::SetBackground(uint16_t color)
{
    Write(OLED_CMD_SET_BACKGROUND);
    WriteShort(color);
    return GetAck();
}

bool OLED::SetBackground(Color color)
{
    return SetBackground(color.To16BitRGB());
}

bool OLED::ReplaceBackground(uint16_t color)
{
    Write(OLED_CMD_REPLACE_BACKGROUND);
    WriteShort(color);
    return GetAck();
}

bool OLED::ReplaceBackground(Color color)
{
    return ReplaceBackground(color.To16BitRGB());
}



//
// Text commands
//

bool OLED::SetFont(uint8_t fontSize)
{
    if (fontSize != OLED_FONT_SMALL &&
        fontSize != OLED_FONT_MEDIUM &&
        fontSize != OLED_FONT_LARGE)
        return false;
    Write(2, OLED_CMD_SET_FONT, fontSize);
    bool result = GetAck();
    if (result) _fontSize = fontSize;
    return result;
}

bool OLED::SetFontOpacity(bool opaque)
{
    uint8_t opacity = opaque ? OLED_FONT_OPAQUE : OLED_FONT_TRANSPARENT;
    Write(2, OLED_CMD_SET_FONT_OPACITY, opacity);
    bool result = GetAck();
    if (result) _fontOpacity = opacity;
    return result;
}

void OLED::SetButtonOpacity(bool opaque)
{
    _buttonOpacity = opaque;
}

void OLED::SetFontColor(uint16_t color)
{
    _fontColor = color;
}

void OLED::SetFontColor(Color color)
{
    _fontColor = color.To16BitRGB();
}

void OLED::SetButtonColor(uint16_t color)
{
    _buttonColor = color;
}

void OLED::SetButtonColor(Color color)
{
    _buttonColor = color.To16BitRGB();
}

void OLED::SetButtonFontColor(uint16_t color)
{
    _buttonFontColor = color;
}

void OLED::SetButtonFontColor(Color color)
{
    _buttonFontColor = color.To16BitRGB();
}

void OLED::SetFontProportional(bool proportional)
{
    _fontProportional = proportional;
}


bool OLED::DrawText(uint8_t col, uint8_t row, String text, uint16_t color,
    uint8_t fontSize, uint8_t opacity, uint8_t proportional)
{
    if ((fontSize != OLED_FONT_SMALL &&
        fontSize != OLED_FONT_MEDIUM &&
        fontSize != OLED_FONT_LARGE &&
        fontSize != OLED_FONT_SIZE_NOT_SET) ||
        (proportional != OLED_FONT_PROPORTIONAL &&
        proportional != OLED_FONT_NONPROPORTIONAL &&
        proportional != OLED_FONT_PROPORTIONAL_NOT_SET) ||
        (opacity != OLED_FONT_OPAQUE &&
        opacity != OLED_FONT_TRANSPARENT &&
        opacity != OLED_FONT_OPACITY_NOT_SET))
        return false;
    
    if (fontSize == OLED_FONT_SIZE_NOT_SET)
        fontSize = _fontSize;

    if (proportional == OLED_FONT_PROPORTIONAL_NOT_SET)
        proportional = _fontProportional
            ? OLED_FONT_PROPORTIONAL : OLED_FONT_NONPROPORTIONAL;

    uint8_t oldOpacity = _fontOpacity;
    if (opacity != OLED_FONT_OPACITY_NOT_SET)
        SetFontOpacity(opacity == OLED_FONT_OPAQUE);
    
    bool result;
    Write(4, OLED_CMD_DRAW_STRING_TEXT, col, row, fontSize | proportional);
    WriteShort(color);
    WriteString(text);
    Write(0x00);
    result = GetAck();
    
    if (opacity != OLED_FONT_OPACITY_NOT_SET)
        SetFontOpacity(oldOpacity == OLED_FONT_OPAQUE);

    return result;
}

bool OLED::DrawText(uint8_t col, uint8_t row, String text, Color color,
    uint8_t fontSize, uint8_t opacity, uint8_t proportional)
{
    return DrawText(col, row, text, color.To16BitRGB(),
        fontSize, opacity, proportional);
}

bool OLED::DrawText(uint8_t col, uint8_t row, String text)
{
    return DrawText(col, row, text, _fontColor,
        OLED_FONT_SIZE_NOT_SET, OLED_FONT_OPACITY_NOT_SET, OLED_FONT_PROPORTIONAL_NOT_SET);
}


bool OLED::DrawTextGraphic(uint8_t x, uint8_t y, String text, uint8_t width, uint8_t height,
    uint16_t color,	uint8_t fontSize, uint8_t opacity, uint8_t proportional)
{
    if ((fontSize != OLED_FONT_SMALL &&
        fontSize != OLED_FONT_MEDIUM &&
        fontSize != OLED_FONT_LARGE &&
        fontSize != OLED_FONT_SIZE_NOT_SET) ||
        (proportional != OLED_FONT_PROPORTIONAL &&
        proportional != OLED_FONT_NONPROPORTIONAL &&
        proportional != OLED_FONT_PROPORTIONAL_NOT_SET) ||
        (opacity != OLED_FONT_OPAQUE &&
        opacity != OLED_FONT_TRANSPARENT &&
        opacity != OLED_FONT_OPACITY_NOT_SET) ||
        width < 1 || height < 1)
        return false;
    
    if (fontSize == OLED_FONT_SIZE_NOT_SET)
        fontSize = _fontSize;

    if (proportional == OLED_FONT_PROPORTIONAL_NOT_SET)
        proportional = _fontProportional
            ? OLED_FONT_PROPORTIONAL : OLED_FONT_NONPROPORTIONAL;

    uint8_t oldOpacity = _fontOpacity;
    if (opacity != OLED_FONT_OPACITY_NOT_SET)
        SetFontOpacity(opacity == OLED_FONT_OPAQUE);

    bool result;
    Write(4, OLED_CMD_DRAW_STRING_GFX, x, y, fontSize | proportional);
    WriteShort(color);
    Write(2, width, height);
    WriteString(text);
    Write(0x00);
    result = GetAck();
    
    if (opacity != OLED_FONT_OPACITY_NOT_SET)
        SetFontOpacity(oldOpacity == OLED_FONT_OPAQUE);

    return result;
}

bool OLED::DrawTextGraphic(uint8_t x, uint8_t y, String text, uint8_t width, uint8_t height,
    Color color, uint8_t fontSize, uint8_t opacity, uint8_t proportional)
{
    return DrawTextGraphic(x,y,text,width,height,color.To16BitRGB(),fontSize,opacity,proportional);
}

bool OLED::DrawTextGraphic(uint8_t x, uint8_t y, String text, uint8_t width, uint8_t height)
{
    return DrawTextGraphic(x, y, text, width, height, _fontColor,
        OLED_FONT_SIZE_NOT_SET, OLED_FONT_OPACITY_NOT_SET, OLED_FONT_PROPORTIONAL_NOT_SET);
}

bool OLED::DrawTextGraphic(uint8_t x, uint8_t y, String text)
{
    return DrawTextGraphic(x, y, text, 1, 1);
}


bool OLED::DrawTextButton(uint8_t x, uint8_t y, String text, uint8_t width, uint8_t height,
    bool pressed, uint16_t fontColor, uint16_t buttonColor,
    uint8_t fontSize, uint8_t opacity, uint8_t proportional)
{
    if ((fontSize != OLED_FONT_SMALL &&
        fontSize != OLED_FONT_MEDIUM &&
        fontSize != OLED_FONT_LARGE &&
        fontSize != OLED_FONT_SIZE_NOT_SET) ||
        (proportional != OLED_FONT_PROPORTIONAL &&
        proportional != OLED_FONT_NONPROPORTIONAL &&
        proportional != OLED_FONT_PROPORTIONAL_NOT_SET) ||
        (opacity != OLED_FONT_OPAQUE &&
        opacity != OLED_FONT_TRANSPARENT &&
        opacity != OLED_FONT_OPACITY_NOT_SET) ||
        width < 1 || height < 1)
        return false;
    
    if (fontSize == OLED_FONT_SIZE_NOT_SET)
        fontSize = _fontSize;

    if (proportional == OLED_FONT_PROPORTIONAL_NOT_SET)
        proportional = _fontProportional
            ? OLED_FONT_PROPORTIONAL : OLED_FONT_NONPROPORTIONAL;

    uint8_t oldOpacity = _buttonOpacity;
    if (opacity != OLED_FONT_OPACITY_NOT_SET)
        SetButtonOpacity(opacity == OLED_FONT_OPAQUE);

    bool changedFontOpacity = false;
    uint8_t oldFontOpacity = _fontOpacity;
    if (_buttonOpacity != _fontOpacity)
    {
        changedFontOpacity = true;
        oldFontOpacity = _fontOpacity;
        SetFontOpacity(_buttonOpacity == OLED_FONT_OPAQUE);
    }

    bool result;
    Write(4, OLED_CMD_DRAW_STRING_BUTTON,
        pressed ? OLED_PRM_BUTTON_DOWN : OLED_PRM_BUTTON_UP,
        x, y);
    WriteShort(buttonColor);
    Write(fontSize | proportional);
    WriteShort(fontColor);
    Write(2, width, height);
    WriteString(text);
    Write(0x00);
    result = GetAck();

    if (changedFontOpacity)
        SetFontOpacity(oldFontOpacity);
    
    if (opacity != OLED_FONT_OPACITY_NOT_SET)
        SetButtonOpacity(oldOpacity == OLED_FONT_OPAQUE);

    return result;
}

bool OLED::DrawTextButton(uint8_t x, uint8_t y, String text, uint8_t width, uint8_t height,
    bool pressed, Color fontColor, Color buttonColor,
    uint8_t fontSize, uint8_t opacity, uint8_t proportional)
{
    return DrawTextButton(x, y, text, width, height,
        pressed, fontColor.To16BitRGB(), buttonColor.To16BitRGB(),
        fontSize, opacity, proportional);
}

bool OLED::DrawTextButton(uint8_t x, uint8_t y, String text, uint8_t width, uint8_t height,
    bool pressed)
{
    return DrawTextButton(x, y, text, width, height, pressed, _fontColor, _buttonColor,
        OLED_FONT_SIZE_NOT_SET, OLED_FONT_OPACITY_NOT_SET, OLED_FONT_PROPORTIONAL_NOT_SET);
}



//
// SD Card
//

bool OLED::SDInitialize()
{
    Write(2, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_INITIALIZE_CARD);
    return GetAck();
}

bool OLED::SDSetAddressPointer(uint32_t address)
{
    Write(2, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_SET_ADDRESS_POINTER);
    WriteLong(address);
    return GetAck();
}


bool OLED::SDRead(uint8_t &data)
{
    Write(2, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_READ_BYTE);
    return GetResponse(data);
}

bool OLED::SDReadShort(uint16_t &data)
{
    uint8_t byte1, byte2;
    if (!SDRead(byte1) || !SDRead(byte2))
        return false;

    data = ((uint16_t)byte1 << 8) | byte2;
    return true;
}

bool OLED::SDReadLong(uint32_t &data)
{
    uint16_t short1, short2;
    if (!SDReadShort(short1) || !SDReadShort(short2))
        return false;

    data = ((uint32_t)short1 << 16) | short2;
    return true;
}

bool OLED::SDReadString(String &data)
{
    data = "";

    for (uint32_t c = 0; c < OLED_SD_READ_STRING_MAX_LENGTH; c++)
    {
        uint8_t readByte;
        bool readOk = SDRead(readByte);
        if (!readOk)
            return false;

        if (readByte == 0x00)
            return true;

        data += (char)readByte;
    }
    return false;
}


bool OLED::SDWrite(uint8_t data)
{
    Write(3, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_WRITE_BYTE, data);
    return GetAck();
}

bool OLED::SDWrite(uint16_t numValues, uint8_t *values)
{
    for (uint16_t v = 0; v < numValues; v++)
    {
        if (!SDWrite(values[v]))
            return false;
    }
    return true;
}

bool OLED::SDWrite(uint16_t numValues, uint8_t value1, ...)
{
    if (!SDWrite(value1))
        return false;

    va_list ap;
    uint16_t current = 1;
    va_start(ap, value1);
    while (current < numValues)
    {
        if (!SDWrite((uint8_t)va_arg(ap, int)))
            return false;
        current++;
    }
    va_end(ap);
    return true;
}

bool OLED::SDWriteShort(uint16_t data)
{
    return
        SDWrite(OLEDUtil::GetByte(data,1)) &&
        SDWrite(OLEDUtil::GetByte(data));
}

bool OLED::SDWriteShort(uint16_t numValues, uint16_t *values)
{
    for (uint16_t v = 0; v < numValues; v++)
    {
        if (!SDWriteShort(values[v]))
            return false;
    }
    return true;
}

bool OLED::SDWriteShort(uint16_t numValues, uint16_t value1, ...)
{
    if (!SDWriteShort(value1))
        return false;

    va_list ap;
    uint16_t current = 1;
    va_start(ap, value1);
    while (current < numValues)
    {
        if (!SDWriteShort((uint16_t)va_arg(ap, int)))
            return false;
        current++;
    }
    va_end(ap);
    return true;
}

bool OLED::SDWriteLong(uint32_t data)
{
    return
        SDWrite(OLEDUtil::GetByte(data,3)) &&
        SDWrite(OLEDUtil::GetByte(data,2)) &&
        SDWrite(OLEDUtil::GetByte(data,1)) &&
        SDWrite(OLEDUtil::GetByte(data));
}

bool OLED::SDWriteLong(uint16_t numValues, uint32_t *values)
{
    for (uint16_t v = 0; v < numValues; v++)
    {
        if (!SDWriteLong(values[v]))
            return false;
    }
    return true;
}

bool OLED::SDWriteLong(uint16_t numValues, uint32_t value1, ...)
{
    if (!SDWriteLong(value1))
        return false;

    va_list ap;
    uint16_t current = 1;
    va_start(ap, value1);
    while (current < numValues)
    {
        if (!SDWriteLong((uint32_t)va_arg(ap, int)))
            return false;
        current++;
    }
    va_end(ap);
    return true;
}

bool OLED::SDWriteText(char* text)
{
    for(uint8_t c = 0; c < strlen(text); c++)
        if (!SDWrite(text[c]))
            return false;
    return SDWrite(0x00);
}

bool OLED::SDWriteString(String text)
{
    for(uint8_t c = 0; c < text.length(); c++)
        if (!SDWrite(text[c]))
            return false;
    return SDWrite(0x00);
}


bool OLED::SDReadSector(uint32_t sectorAddress, uint8_t *data)
{
    uint16_t bytesRead;
    return SDReadSector(sectorAddress, data, bytesRead);
}

bool OLED::SDReadSector(uint32_t sectorAddress, uint8_t *data, uint16_t &bytesRead)
{
    Write(5, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_READ_SECTOR_BLOCK,
        OLEDUtil::GetByte(sectorAddress, 2),
        OLEDUtil::GetByte(sectorAddress, 1),
        OLEDUtil::GetByte(sectorAddress));

    delay(OLED_SD_SECTOR_READ_DELAY_MS);

    for (uint16_t b = 0; b < OLED_SD_SECTOR_SIZE; b++)
    {
        bytesRead = b;
        uint8_t result;
        
        if (!GetResponse(result))
            return false;
        data[b] = result;
    }
    return true;
}

bool OLED::SDWriteSector(uint32_t sectorAddress, uint8_t *data)
{
    Write(5, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_WRITE_SECTOR_BLOCK,
        OLEDUtil::GetByte(sectorAddress, 2),
        OLEDUtil::GetByte(sectorAddress, 1),
        OLEDUtil::GetByte(sectorAddress));

    for (uint16_t b = 0; b < OLED_SD_SECTOR_SIZE; b++)
        Write(data[b]);

    return GetAck();
}

bool OLED::SDWipeSector(uint32_t sectorAddress, uint8_t wipeData)
{
    Write(5, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_WRITE_SECTOR_BLOCK,
        OLEDUtil::GetByte(sectorAddress, 2),
        OLEDUtil::GetByte(sectorAddress, 1),
        OLEDUtil::GetByte(sectorAddress));

    for (uint16_t b = 0; b < OLED_SD_SECTOR_SIZE; b++)
        Write(wipeData);

    return GetAck();
}

bool OLED::SDWipeSectors(uint32_t sectorAddress, uint32_t numSectors,
    uint32_t &sectorsWiped, bool displayProgress, uint8_t wipeData)
{    
    sectorsWiped = 0;
    uint32_t percent = 0;

    if (numSectors == 0)
        return true;

    uint8_t height = GetDeviceHeight();
    uint8_t width = GetDeviceWidth();

    bool success = true;
    for (uint32_t sector = sectorAddress; sector < numSectors; sector++)
    {
        if (displayProgress)
        {

            percent = sector * 100 / numSectors;
            
            DrawProgressBar(0,height-9,width,9,
                percent, COLOR_DARKSLATEGRAY, Color::From32BitRGB(0x182828));
            
            String output = "s:" + (String)sector + " (" + percent + "%)";
            DrawTextGraphic(1, height-8, output, 1, 1,
                COLOR_WHITE, OLED_FONT_SMALL, OLED_FONT_TRANSPARENT, OLED_FONT_PROPORTIONAL);
        }
        
        if (!SDWipeSector(sector, wipeData))
        {
            success = false;
            break;
        }
        
        sectorsWiped = (sector - sectorAddress) + 1;
    }
    
    if (displayProgress && success)
    {
        DrawRectangleWH(0, height-9, width, 9, COLOR_DARKSLATEGRAY);
        DrawTextGraphic(1, height-8, "s:"+(String)sectorsWiped+" (100%)", 1, 1,
            COLOR_WHITE, OLED_FONT_SMALL, OLED_FONT_TRANSPARENT, OLED_FONT_PROPORTIONAL);
    }

    return success;
}

uint32_t OLED::SDWipeCard(uint8_t wipeData)
{
    SDWriteScreen(0x00);

    uint32_t sector = 1;
    while (SDWipeSector(sector))
    {
        DrawTextGraphic(0,GetDeviceHeight()-8,(String)""+sector+" ", 1, 1, COLOR_WHITE,
            OLED_FONT_SMALL, OLED_FONT_OPAQUE, OLED_FONT_PROPORTIONAL);
        sector++;
    }

    SDDrawScreen(0x00);
    SDWipeSector(0x00);

    return sector;
}

bool OLED::SDWriteScreen(uint32_t sectorAddress)
{
    return SDWriteScreen(sectorAddress, 0, 0, GetDeviceWidth(), GetDeviceHeight());
}

bool OLED::SDWriteScreen(uint32_t sectorAddress,
    uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    Write(9, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_WRITE_SCREENSHOT,
        x, y, width, height,
        OLEDUtil::GetByte(sectorAddress, 2),
        OLEDUtil::GetByte(sectorAddress, 1),
        OLEDUtil::GetByte(sectorAddress));
    return GetAck();
}

 bool OLED::SDDrawScreen(uint32_t sectorAddress)
 {
     return SDDrawImage(sectorAddress, 0, 0, GetDeviceWidth(), GetDeviceHeight());
 }

// The width MUST match the width of the original image to display correctly.
// Height can be anything <= original height, image will be truncated accordingly.
bool OLED::SDDrawImage(uint32_t sectorAddress,
    uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    Write(10, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_DISPLAY_IMAGE,
        x, y, width, height, OLED_PRM_DRAW_IMAGE_16BIT,
        OLEDUtil::GetByte(sectorAddress, 2),
        OLEDUtil::GetByte(sectorAddress, 1),
        OLEDUtil::GetByte(sectorAddress));
    return GetAck();
}

bool OLED::SDRunCommand(uint32_t address)
{
    Write(2, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_DISPLAY_OBJECT);
    WriteLong(address);
    return GetAck();
}

// UNTESTED!!!
bool OLED::SDPlayVideo(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
    uint8_t delayMs, uint16_t frameCount, uint32_t sectorAddress)
{
    Write(8, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_DISPLAY_VIDEO,
        x, y, width, height, OLED_PRM_DRAW_IMAGE_16BIT, delayMs);
    WriteShort(frameCount);
    Write(3,
        OLEDUtil::GetByte(sectorAddress, 2),
        OLEDUtil::GetByte(sectorAddress, 1),
        OLEDUtil::GetByte(sectorAddress));
    return GetAck();
}

// UNTESTED!!!
bool OLED::SDRunScript(uint32_t address)
{
    Write(2, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_RUN_4DSL_SCRIPT);
    WriteLong(address);

    // This command will not return a response if successful.
    // If unsuccessful (or no SD is installed), NAK is returned.
    uint8_t response = 0x00;
    return (!GetResponse(response) || response != OLED_NAK);
}
