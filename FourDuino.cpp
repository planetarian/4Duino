#include "FourDuino.h"


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

bool OLED::init()
{
    _deviceType = Unknown;
    _hardwareRevision = 0;
    _firmwareRevision = 0;
    _deviceWidth = 0;
    _deviceHeight = 0;
    _fontSize = OLED_FONT_SMALL;
    _fontOpacity = OLED_FONT_TRANSPARENT;
    _fontProportional = OLED_FONT_NONPROPORTIONAL;


    for (uint8_t i = 0; i < OLED_MAX_USER_BITMAPS; i++)
        _charIndexList[i] = false;

    pinMode(_pinReset, OUTPUT);

    // Initialize the display using auto-baud command at 9600 baud.
    bool _run = false;
    for (uint8_t r = 0; r < OLED_INIT_RETRIES && !_run; r++)
    {
        reset();
        // Wait for the OLED/SD to initialize
        delay(OLED_INIT_DELAY_MS);

        _serial->begin(9600);
        // Let the OLED auto-detect baud rate
        write(OLED_CMD_BAUD_AUTO);
        if (!getAck())
            continue;
        // Change to the desired baud
        _run = setBaud(_baudRate);
    }
    if (!_run)
        return false;

    getDeviceInfo(false);

    setFill(OLED_SHAPE_FILL_DEFAULT);
    setFont(OLED_FONT_SIZE_DEFAULT);
    setFontOpacity(OLED_FONT_OPACITY_DEFAULT);
    setFontProportional(OLED_FONT_PROPORTIONAL_DEFAULT);
    setFontColor(OLED_FONT_COLOR_DEFAULT);
    setButtonOpacity(OLED_BUTTON_OPACITY_DEFAULT);
    setButtonColor(OLED_BUTTON_COLOR_DEFAULT);
    setButtonFontColor(OLED_BUTTON_FONT_COLOR_DEFAULT);
    return true;
}

bool OLED::setBaud(uint32_t baudRate)
{
    uint8_t baudByte = 0;
    if (!_getBaudByte(baudRate, baudByte))
        return false;
    write(OLED_CMD_BAUD);
    write(baudByte);
    _serial->begin(_baudRate);

    return getAck();
}

bool OLED::_getBaudByte(uint32_t baudRate, uint8_t &baudByte)
{
    baudByte = 0xFF;
    switch (baudRate)
    {
    case 110:
        baudByte = OLED_PRM_BAUD_110;
        break;
    case 300:
        baudByte = OLED_PRM_BAUD_300;
        break;
    case 600:
        baudByte = OLED_PRM_BAUD_600;
        break;
    case 1200:
        baudByte = OLED_PRM_BAUD_1200;
        break;
    case 2400:
        baudByte = OLED_PRM_BAUD_2400;
        break;
    case 4800:
        baudByte = OLED_PRM_BAUD_4800;
        break;
    case 9600:
        baudByte = OLED_PRM_BAUD_9600;
        break;
    case 14400:
        baudByte = OLED_PRM_BAUD_14400;
        break;
    case 19200:
        baudByte = OLED_PRM_BAUD_19200;
        break;
    case 31250:
        baudByte = OLED_PRM_BAUD_31250;
        break;
    case 38400:
        baudByte = OLED_PRM_BAUD_38400;
        break;
    case 56000:
        baudByte = OLED_PRM_BAUD_56000;
        break;
    case 57600:
        baudByte = OLED_PRM_BAUD_57600;
        break;
    case 115200:
        baudByte = OLED_PRM_BAUD_115200;
        break;
    case 129032:
        baudByte = OLED_PRM_BAUD_129032;
        break;
    case 282353:
        baudByte = OLED_PRM_BAUD_282353;
        break;
    case 128000:
        baudByte = OLED_PRM_BAUD_128000;
        break;
    case 256000:
        baudByte = OLED_PRM_BAUD_256000;
        break;
    default:
        return false;
    }
    return true;
}

void OLED::reset()
{
    digitalWrite(_pinReset, LOW);
    delay(OLED_RESET_DELAY_MS);
    digitalWrite(_pinReset, HIGH);
    delay(OLED_RESET_DELAY_MS);
}


//
// Helper functions
//

OLED::DeviceType OLED::_convertDeviceType(uint8_t deviceTypeResponse)
{
    switch (deviceTypeResponse)
    {
    case OLED_DEVICETYPE_LCD:
        return uLCD;
    case OLED_DEVICETYPE_OLED:
        return uOLED;
    case OLED_DEVICETYPE_VGA:
        return VGA;
    default:
        return Unknown;
    }
}

uint16_t OLED::_convertResolution(uint8_t resolutionResponse)
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
    case OLED_RES_UNKNOWN:
    default:
        return 0xFFFF;
    }
}



//
// OLED read/write functions
//

void OLED::write(uint8_t value)
{
    _serial->write(value);
}

void OLED::write(uint8_t numValues, uint8_t value1, ...)
{
    write(value1);

    va_list ap;
    uint8_t current = 1;
    va_start(ap, value1);
    while (current < numValues)
    {
        write((uint8_t)va_arg(ap, int));
        current++;
    }
    va_end(ap);
}

void OLED::writeShort(uint16_t value)
{
    write(2, OLEDUtil::getByte(value, 1), OLEDUtil::getByte(value));
}

void OLED::writeShort(uint8_t numValues, uint16_t value1, ...)
{
    writeShort(value1);

    va_list ap;
    uint8_t current = 1;
    va_start(ap, value1);
    while (current < numValues)
    {
        writeShort(va_arg(ap, uint16_t));
        current++;
    }
    va_end(ap);
}

void OLED::writeLong(uint32_t value)
{
    write(4,
        OLEDUtil::getByte(value, 3),
        OLEDUtil::getByte(value, 2),
        OLEDUtil::getByte(value, 1),
        OLEDUtil::getByte(value));
}

void OLED::writeLong(uint8_t numValues, uint32_t value1, ...)
{
    writeLong(value1);

    va_list ap;
    uint8_t current = 1;
    va_start(ap, value1);
    while (current < numValues)
    {
        writeLong(va_arg(ap, uint16_t));
        current++;
    }
    va_end(ap);
}

void OLED::writeText(char* text)
{
    for(uint32_t c = 0; c < strlen(text); c++)
    {
        if (text[c] == 0x00)
            return;
        write(text[c]);
    }
}

void OLED::writeString(String text)
{
    for(uint32_t c = 0; c < text.length(); c++)
    {
        if (text[c] == 0x00)
            return;
        write(text[c]);
    }
}

void OLED::writeSpatial(uint16_t value)
{
    if (_controllerType == Picaso)
        writeShort(value);
    else if (_controllerType == Goldelox)
        write(value & 0xFF);
}

void OLED::writeSpatial(uint8_t numValues, uint16_t value1, ...)
{
    writeSpatial(value1);

    va_list ap;
    uint8_t current = 1;
    va_start(ap, value1);
    while (current < numValues)
    {
        writeSpatial((uint16_t)va_arg(ap, int));
        current++;
    }
    va_end(ap);
}


bool OLED::getResponse(uint8_t& result)
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

bool OLED::getResponseShort(uint16_t& result)
{
    uint8_t byte1, byte2;
    if (!getResponse(byte1) || !getResponse(byte2))
        return false;
    result = ((uint16_t)byte1 << 8) + byte2;
    return true;
}

bool OLED::getAck()
{
    uint8_t result;
    return (getResponse(result) && result == OLED_ACK);
}



//
// OLED general/system commands
//



bool OLED::getDeviceInfo(bool displayOnScreen)
{
    uint8_t output = displayOnScreen
        ? OLED_PRM_BOOL_TRUE
        : OLED_PRM_BOOL_FALSE;

    write(2, OLED_CMD_INFO, output);

    uint8_t response[5];

    uint8_t hw, fw;
    if (!getResponse(response[0]) ||
        !getResponse(response[1]) || 
        !getResponse(response[2]) ||
        !getResponse(response[3]) ||
        !getResponse(response[4]) ||
        !OLEDUtil::readHexAsDec(response[1], hw) ||
        !OLEDUtil::readHexAsDec(response[2], fw))
        return false;
    
    _deviceType = _convertDeviceType(response[0]);
    _hardwareRevision = hw;
    _firmwareRevision = fw;
    if (response[3] != OLED_RES_UNKNOWN &&
        response[4] != OLED_RES_UNKNOWN)
    {
        _deviceWidth = _convertResolution(response[3]);
        _deviceHeight = _convertResolution(response[4]);
    }
    else
    {
        _getDeviceResolution();
    }

    _controllerType = max(_deviceHeight, _deviceWidth) > 0xFF
        ? Picaso
        : Goldelox;

    return true;
}

OLED::ControllerType OLED::getControllerType() { return _controllerType; }
OLED::DeviceType OLED::getDeviceType() { return _deviceType; }
uint16_t OLED::getDeviceWidth()
{ 
    return _deviceWidth;
}
uint16_t OLED::getDeviceHeight()
{
    return _deviceHeight;
}
uint8_t OLED::getHardwareRevision() { return _hardwareRevision; }
uint8_t OLED::getFirmwareRevision() { return _firmwareRevision; }

bool OLED::_getDeviceResolution()
{

    write(OLED_CMD_GET_RESOLUTION);
    uint16_t width, height;

    if (!getResponseShort(width) ||
        !getResponseShort(height))
        return false;

    _deviceWidth = width;
    _deviceHeight = height;
    return true;
}

bool OLED::clear()
{
    write(OLED_CMD_CLEAR_SCREEN);
    return getAck();
}


bool OLED::setPower(bool on)
{
    write(3,OLED_CMD_CTLFUNC, OLED_PRM_CTLFUNC_POWER,
        on ? OLED_PRM_CTLFUNC_POWER_ON : OLED_PRM_CTLFUNC_POWER_OFF);
    return getAck();
}

bool OLED::on()
{
    return setPower(true);
}

bool OLED::off()
{
    return setPower(false);
}

bool OLED::setContrast(uint8_t value)
{
    if (value > OLED_CONTRAST_MAX)
        return false;
    write(3, OLED_CMD_CTLFUNC, OLED_PRM_CTLFUNC_CONTRAST, value);
    return getAck();
}

bool OLED::setContrastFromAnalog(uint8_t analogPin, uint8_t minimumContrast)
{
    uint16_t analogValue = analogRead(analogPin);
    if (minimumContrast >= OLED_CONTRAST_MAX || analogValue >= 1024)
        return false;
    uint8_t scaled = OLEDUtil::scaleAnalog(analogValue, OLED_CONTRAST_MAX-minimumContrast);
    return setContrast(scaled+minimumContrast);
}

bool OLED::lowPowerShutdown()
{
    write(3, OLED_CMD_CTLFUNC, OLED_PRM_CTLFUNC_LOWPOWER,
        OLED_PRM_CTLFUNC_LOWPOWER_SHUTDOWN);
    return getAck();
}

bool OLED::lowPowerPowerUp()
{
    write(3, OLED_CMD_CTLFUNC, OLED_PRM_CTLFUNC_LOWPOWER,
        OLED_PRM_CTLFUNC_LOWPOWER_POWERUP);
    return getAck();
}


bool OLED::turnOffSD()
{
    write(3, OLED_CMD_SLEEP, OLED_PRM_SLEEP_SD_OFF, OLED_PRM_NA);
    return getAck();
}

bool OLED::wakeOnJoystick()
{
    write(3, OLED_CMD_SLEEP, OLED_PRM_SLEEP_WAKE_JOY, OLED_PRM_NA);
    return getAck();
}

bool OLED::wakeOnSerial()
{
    write(3, OLED_CMD_SLEEP, OLED_PRM_SLEEP_WAKE_SERIAL, OLED_PRM_NA);
    return getAck();
}


// TODO: NOT IMPLEMENTED YET
bool OLED::joystick()
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
bool OLED::joystickWait()
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
bool OLED::sound(uint16_t note, uint16_t duration)
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
bool OLED::tune(uint8_t length, uint16_t note, uint16_t duration)
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

bool OLED::readPixel(uint16_t x, uint16_t y, uint16_t& resultShort)
{
    if (x < 0 || x >= getDeviceWidth() ||
        y < 0 || y >= getDeviceHeight())
        return false;
    write(OLED_CMD_READ_PIXEL);
    writeSpatial(2, x, y);
    
    if (!getResponseShort(resultShort))
        return false;
        
    return true;
}

bool OLED::readPixel(uint16_t x, uint16_t y, Color& resultColor)
{
    uint16_t resultShort;
    if (!readPixel(x, y, resultShort))
        return false;
    resultColor = Color::from16BitRGB(resultShort);
    return true;
}


bool OLED::drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    write(OLED_CMD_DRAW_PIXEL);
    writeSpatial(2, x, y);
    writeShort(color);
    return getAck();
}

bool OLED::drawPixel(uint16_t x, uint16_t y, Color color)
{
    return drawPixel(x, y, color.to16BitRGB());
}


bool OLED::drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    write(OLED_CMD_DRAW_LINE);
    writeSpatial(4, x1, y1, x2, y2);
    writeShort(color);
    return getAck();
}

bool OLED::drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color color)
{
    return drawLine(x1, y1, x2, y2, color.to16BitRGB());
}


bool OLED::drawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    write(OLED_CMD_DRAW_RECTANGLE);
    writeSpatial(4, x1, y1, x2, y2);
    writeShort(color);
    return getAck();
}

bool OLED::drawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color color)
{
    return drawRectangle(x1,y1,x2,y2,color.to16BitRGB());
}

bool OLED::drawRectangleWH(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    if (width < 1 || height < 1)
        return false;
    uint16_t x2 = x + (width-1);
    uint16_t y2 = y + (height-1);
    return drawRectangle(x,y,x2,y2,color);
}

bool OLED::drawRectangleWH(uint16_t x, uint16_t y, uint16_t width, uint16_t height, Color color)
{
    return drawRectangleWH(x,y,width,height,color.to16BitRGB());
}

bool OLED::drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
    uint8_t progressPercent, uint16_t foreColor, uint16_t backColor)
{
    uint16_t progressWidth = width * progressPercent / 100;
    return
        drawRectangleWH(x, y, progressWidth, height, foreColor) &&
        drawRectangleWH(x+progressWidth, y, width-progressWidth, height, backColor);
}

bool OLED::drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
    uint8_t progressPercent, Color foreColor, Color backColor)
{
    return drawProgressBar(x, y, width, height,
        progressPercent, foreColor.to16BitRGB(), backColor.to16BitRGB());
}


bool OLED::drawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
    uint16_t color)
{
    write(OLED_CMD_DRAW_TRIANGLE);
    writeSpatial(6, x1, y1, x2, y2, x3, y3);
    writeShort(color);
    return getAck();
}

bool OLED::drawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
    Color color)
{
    return drawTriangle(x1,y1,x2,y2,x3,y3,color.to16BitRGB());
}


bool OLED::_drawPolygonVa(uint16_t color, uint8_t numVertices, uint16_t x1, uint16_t y1, va_list ap)
{
    // Vertex count limited by the serial interface
    if (numVertices > OLED_MAX_POLYGON_VERTICES)
        return false;
    // Polygon must be at least 3 vertices, but no reason to flat-out reject it...
    if (numVertices == 1)
        return drawPixel(x1, y1, color);
    if (numVertices == 2)
        return drawLine(x1, y1, (uint16_t)va_arg(ap, int), (uint16_t)va_arg(ap, int), color);


    write(2, OLED_CMD_DRAW_POLYGON, numVertices);
    writeSpatial(2, x1, y1);
    for (uint8_t i = 1; i < numVertices; i++)
        writeSpatial(2, (uint16_t)va_arg(ap, int), (uint16_t)va_arg(ap, int));
    writeShort(color);
    return getAck();
}

bool OLED::drawPolygon(uint16_t color, uint8_t numVertices, uint16_t x1, uint16_t y1, ...)
{
    va_list ap;
    va_start(ap, y1);
    bool result = _drawPolygonVa(color, numVertices, x1, y1, ap);
    va_end(ap);
    return result;
}

bool OLED::drawPolygon(Color color, uint8_t numVertices, uint16_t x1, uint16_t y1, ...)
{
    va_list ap;
    va_start(ap, y1);
    bool result = _drawPolygonVa(color.to16BitRGB(), numVertices, x1, y1, ap);
    va_end(ap);
    return result;
}

bool OLED::drawPolygon(uint16_t color, uint8_t numVertices, uint16_t vertices[][2])
{
    if (numVertices > OLED_MAX_POLYGON_VERTICES) return false;
    if (numVertices == 1)
        return drawPixel(vertices[0][0], vertices[0][1], color);
    if (numVertices == 2)
        return drawLine(vertices[0][0], vertices[0][1], vertices[1][0], vertices[1][1], color);
    
    write(2, OLED_CMD_DRAW_POLYGON, numVertices);
    for (uint8_t v = 0; v < numVertices; v++)
        writeSpatial(2, vertices[v][0], vertices[v][1]);
    writeShort(color);
    return getAck();
}

bool OLED::drawPolygon(Color color, uint8_t numVertices, uint16_t vertices[][2])
{
    return drawPolygon(color.to16BitRGB(), numVertices, vertices);
}

bool OLED::drawCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color)
{
    write(OLED_CMD_DRAW_CIRCLE);
    writeSpatial(3, x, y, radius);
    writeShort(color);
    return getAck();
}

bool OLED::drawCircle(uint16_t x, uint16_t y, uint16_t radius, Color color)
{
    return drawCircle(x, y, radius, color.to16BitRGB());
}

// NOT YET IMPLEMENTED
// The width MUST match the width of the original image to display correctly.
// Height can be anything <= original height, image will be truncated accordingly.
bool OLED::drawImage8Bit(uint16_t x, uint16_t y, uint16_t imageWidth, uint16_t imageHeight,
        uint8_t pixel1, ...)
{
    return false;
    //write(6, OLED_CMD_DRAW_IMAGE, x, y, imageWidth, imageHeight, OLED_PRM_DRAW_IMAGE_8BIT);
    // TODO: process/send pixel data
    //write(imageWidth*imageHeight, pixel1, ...);
    //return getAck();
}

// NOT YET IMPLEMENTED
// The width MUST match the width of the original image to display correctly.
// Height can be anything <= original height, image will be truncated accordingly.
bool OLED::drawImage16Bit(uint16_t x, uint16_t y, uint16_t imageWidth, uint16_t imageHeight,
        uint16_t pixel1, ...)
{
    return false;
    //write(6, OLED_CMD_DRAW_IMAGE, x, y, imageWidth, imageHeight, OLED_PRM_DRAW_IMAGE_16BIT);
    // TODO: process/send pixel data
    //writeShort(imageWidth*imageHeight, pixel1, ...);
    //return getAck();
}

bool OLED::addUserBitmap(uint8_t charIndex,
    uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4,
    uint8_t data5, uint8_t data6, uint8_t data7, uint8_t data8)
{
    _charIndexList[charIndex] = true;
    write(10, OLED_CMD_ADD_USER_BITMAP, charIndex,
        data1, data2, data3, data4, data5, data6, data7, data8);
    return getAck();
}

bool OLED::drawUserBitmap(uint8_t charIndex, uint16_t x, uint16_t y, uint16_t color)
{
    if (!_charIndexList[charIndex])
        return false;

    write(2, OLED_CMD_DRAW_USER_BITMAP, charIndex);
    writeSpatial(2, x, y);
    writeShort(color);
    return getAck();
}

bool OLED::drawUserBitmap(uint8_t charIndex, uint16_t x, uint16_t y, Color color)
{
    return drawUserBitmap(charIndex, x, y, color.to16BitRGB());
}

bool OLED::setFill(bool fillShapes)
{
    write(2, OLED_CMD_SET_SHAPE_FILL,
        fillShapes ? OLED_PRM_SHAPE_FILL_SOLID : OLED_PRM_SHAPE_FILL_EMPTY);
    return getAck();
}

bool OLED::screenCopyPaste(uint16_t sourceX, uint16_t sourceY, uint16_t destX, uint16_t destY,
    uint16_t sourceWidth, uint16_t sourceHeight)
{
    write(OLED_CMD_SCREEN_COPY_PASTE);
    writeSpatial(6, sourceX, sourceY, destX, destY, sourceWidth, sourceHeight);
    return getAck();
}


bool OLED::setBackground(uint16_t color)
{
    write(OLED_CMD_SET_BACKGROUND);
    writeShort(color);
    return getAck();
}

bool OLED::setBackground(Color color)
{
    return setBackground(color.to16BitRGB());
}

bool OLED::replaceBackground(uint16_t color)
{
    write(OLED_CMD_REPLACE_BACKGROUND);
    writeShort(color);
    return getAck();
}

bool OLED::replaceBackground(Color color)
{
    return replaceBackground(color.to16BitRGB());
}



//
// Text commands
//

bool OLED::setFont(uint8_t fontSize)
{
    if (fontSize != OLED_FONT_SMALL &&
        fontSize != OLED_FONT_MEDIUM &&
        fontSize != OLED_FONT_LARGE)
        return false;
    write(2, OLED_CMD_SET_FONT, fontSize);
    bool result = getAck();
    if (result) _fontSize = fontSize;
    return result;
}

bool OLED::setFontOpacity(bool opaque)
{
    uint8_t opacity = opaque ? OLED_FONT_OPAQUE : OLED_FONT_TRANSPARENT;
    write(2, OLED_CMD_SET_FONT_OPACITY, opacity);
    bool result = getAck();
    if (result) _fontOpacity = opacity;
    return result;
}

void OLED::setButtonOpacity(bool opaque)
{
    _buttonOpacity = opaque;
}

void OLED::setFontColor(uint16_t color)
{
    _fontColor = color;
}

void OLED::setFontColor(Color color)
{
    _fontColor = color.to16BitRGB();
}

void OLED::setButtonColor(uint16_t color)
{
    _buttonColor = color;
}

void OLED::setButtonColor(Color color)
{
    _buttonColor = color.to16BitRGB();
}

void OLED::setButtonFontColor(uint16_t color)
{
    _buttonFontColor = color;
}

void OLED::setButtonFontColor(Color color)
{
    _buttonFontColor = color.to16BitRGB();
}

void OLED::setFontProportional(bool proportional)
{
    _fontProportional = proportional;
}


bool OLED::drawText(uint8_t col, uint8_t row, String text, uint16_t color,
    uint8_t fontSize, uint8_t opacity, uint8_t proportional)
{
    if (!_checkDrawTextParameters(fontSize, opacity, proportional))
        return false;

    if (fontSize == OLED_FONT_SIZE_NOT_SET)
        fontSize = _fontSize;

    if (proportional == OLED_FONT_PROPORTIONAL_NOT_SET)
        proportional = _fontProportional
            ? OLED_FONT_PROPORTIONAL : OLED_FONT_NONPROPORTIONAL;

    uint8_t oldOpacity = _fontOpacity;
    if (opacity != OLED_FONT_OPACITY_NOT_SET)
        setFontOpacity(opacity == OLED_FONT_OPAQUE);
    
    bool result;
    write(4, OLED_CMD_DRAW_STRING_TEXT, col, row, fontSize | proportional);
    writeShort(color);
    writeString(text);
    write(0x00);
    result = getAck();
    
    if (opacity != OLED_FONT_OPACITY_NOT_SET)
        setFontOpacity(oldOpacity == OLED_FONT_OPAQUE);

    return result;
}

bool OLED::drawText(uint8_t col, uint8_t row, String text, Color color,
    uint8_t fontSize, uint8_t opacity, uint8_t proportional)
{
    return drawText(col, row, text, color.to16BitRGB(),
        fontSize, opacity, proportional);
}

bool OLED::drawText(uint8_t col, uint8_t row, String text)
{
    return drawText(col, row, text, _fontColor,
        OLED_FONT_SIZE_NOT_SET, OLED_FONT_OPACITY_NOT_SET, OLED_FONT_PROPORTIONAL_NOT_SET);
}


bool OLED::drawTextGraphic(uint16_t x, uint16_t y, String text, uint8_t width, uint8_t height,
    uint16_t color, uint8_t fontSize, uint8_t opacity, uint8_t proportional)
{
    if (!_checkDrawTextParameters(fontSize, opacity, proportional) ||
        width < 1 || height < 1)
        return false;
    
    if (fontSize == OLED_FONT_SIZE_NOT_SET)
        fontSize = _fontSize;

    if (proportional == OLED_FONT_PROPORTIONAL_NOT_SET)
        proportional = _fontProportional
            ? OLED_FONT_PROPORTIONAL : OLED_FONT_NONPROPORTIONAL;

    uint8_t oldOpacity = _fontOpacity;
    if (opacity != OLED_FONT_OPACITY_NOT_SET)
        setFontOpacity(opacity == OLED_FONT_OPAQUE);

    bool result;
    write(OLED_CMD_DRAW_STRING_GFX);
    writeSpatial(2, x, y);
    write(fontSize | proportional);
    writeShort(color);
    write(2, width, height);
    writeString(text);
    write(0x00);
    result = getAck();
    
    if (opacity != OLED_FONT_OPACITY_NOT_SET)
        setFontOpacity(oldOpacity == OLED_FONT_OPAQUE);

    return result;
}

bool OLED::drawTextGraphic(uint16_t x, uint16_t y, String text, uint8_t width, uint8_t height,
    Color color, uint8_t fontSize, uint8_t opacity, uint8_t proportional)
{
    return drawTextGraphic(x,y,text,width,height,color.to16BitRGB(),fontSize,opacity,proportional);
}

bool OLED::drawTextGraphic(uint16_t x, uint16_t y, String text, uint8_t width, uint8_t height)
{
    return drawTextGraphic(x, y, text, width, height, _fontColor,
        OLED_FONT_SIZE_NOT_SET, OLED_FONT_OPACITY_NOT_SET, OLED_FONT_PROPORTIONAL_NOT_SET);
}

bool OLED::drawTextGraphic(uint16_t x, uint16_t y, String text)
{
    return drawTextGraphic(x, y, text, 1, 1);
}


bool OLED::drawTextButton(uint16_t x, uint16_t y, String text, uint8_t width, uint8_t height,
    bool pressed, uint16_t fontColor, uint16_t buttonColor,
    uint8_t fontSize, uint8_t opacity, uint8_t proportional)
{
    if (!_checkDrawTextParameters(fontSize, opacity, proportional) ||
        width < 1 || height < 1)
        return false;
    
    if (fontSize == OLED_FONT_SIZE_NOT_SET)
        fontSize = _fontSize;

    if (proportional == OLED_FONT_PROPORTIONAL_NOT_SET)
        proportional = _fontProportional
            ? OLED_FONT_PROPORTIONAL : OLED_FONT_NONPROPORTIONAL;

    uint8_t oldOpacity = _buttonOpacity;
    if (opacity != OLED_FONT_OPACITY_NOT_SET)
        setButtonOpacity(opacity == OLED_FONT_OPAQUE);

    bool changedFontOpacity = false;
    uint8_t oldFontOpacity = _fontOpacity;
    if (_buttonOpacity != _fontOpacity)
    {
        changedFontOpacity = true;
        oldFontOpacity = _fontOpacity;
        setFontOpacity(_buttonOpacity == OLED_FONT_OPAQUE);
    }

    bool result;
    write(2, OLED_CMD_DRAW_STRING_BUTTON,
        pressed ? OLED_PRM_BUTTON_DOWN : OLED_PRM_BUTTON_UP);
    writeSpatial(2, x, y);
    writeShort(buttonColor);
    write(fontSize | proportional);
    writeShort(fontColor);
    write(2, width, height);
    writeString(text);
    write(0x00);
    result = getAck();

    if (changedFontOpacity)
        setFontOpacity(oldFontOpacity);
    
    if (opacity != OLED_FONT_OPACITY_NOT_SET)
        setButtonOpacity(oldOpacity == OLED_FONT_OPAQUE);

    return result;
}

bool OLED::drawTextButton(uint16_t x, uint16_t y, String text, uint8_t width, uint8_t height,
    bool pressed, Color fontColor, Color buttonColor,
    uint8_t fontSize, uint8_t opacity, uint8_t proportional)
{
    return drawTextButton(x, y, text, width, height,
        pressed, fontColor.to16BitRGB(), buttonColor.to16BitRGB(),
        fontSize, opacity, proportional);
}

bool OLED::drawTextButton(uint16_t x, uint16_t y, String text, uint8_t width, uint8_t height,
    bool pressed)
{
    return drawTextButton(x, y, text, width, height, pressed, _fontColor, _buttonColor,
        OLED_FONT_SIZE_NOT_SET, OLED_FONT_OPACITY_NOT_SET, OLED_FONT_PROPORTIONAL_NOT_SET);
}


bool OLED::_checkDrawTextParameters(uint8_t fontSize, uint8_t opacity, uint8_t proportional)
{
    // TODO: Do this better
    return (fontSize == OLED_FONT_SMALL ||
        fontSize == OLED_FONT_MEDIUM ||
        fontSize == OLED_FONT_LARGE ||
        fontSize == OLED_FONT_SIZE_NOT_SET)
        &&
        (proportional == OLED_FONT_PROPORTIONAL ||
        proportional == OLED_FONT_NONPROPORTIONAL ||
        proportional == OLED_FONT_PROPORTIONAL_NOT_SET)
        &&
        (opacity == OLED_FONT_OPAQUE ||
        opacity == OLED_FONT_TRANSPARENT ||
        opacity == OLED_FONT_OPACITY_NOT_SET);
}



//
// SD Card
//

bool OLED::SDInitialize()
{
    write(2, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_INITIALIZE_CARD);
    return getAck();
}

bool OLED::SDSetAddressPointer(uint32_t address)
{
    write(2, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_SET_ADDRESS_POINTER);
    writeLong(address);
    return getAck();
}


bool OLED::SDRead(uint8_t &data)
{
    write(2, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_READ_BYTE);
    return getResponse(data);
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
    write(3, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_WRITE_BYTE, data);
    return getAck();
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
        SDWrite(OLEDUtil::getByte(data,1)) &&
        SDWrite(OLEDUtil::getByte(data));
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
        SDWrite(OLEDUtil::getByte(data,3)) &&
        SDWrite(OLEDUtil::getByte(data,2)) &&
        SDWrite(OLEDUtil::getByte(data,1)) &&
        SDWrite(OLEDUtil::getByte(data));
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
    write(5, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_READ_SECTOR_BLOCK,
        OLEDUtil::getByte(sectorAddress, 2),
        OLEDUtil::getByte(sectorAddress, 1),
        OLEDUtil::getByte(sectorAddress));

    delay(OLED_SD_SECTOR_READ_DELAY_MS);

    for (uint16_t b = 0; b < OLED_SD_SECTOR_SIZE; b++)
    {
        bytesRead = b;
        uint8_t result;
        
        if (!getResponse(result))
            return false;
        data[b] = result;
    }
    return true;
}

bool OLED::SDWriteSector(uint32_t sectorAddress, uint8_t *data)
{
    write(5, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_WRITE_SECTOR_BLOCK,
        OLEDUtil::getByte(sectorAddress, 2),
        OLEDUtil::getByte(sectorAddress, 1),
        OLEDUtil::getByte(sectorAddress));

    for (uint16_t b = 0; b < OLED_SD_SECTOR_SIZE; b++)
        write(data[b]);

    return getAck();
}

bool OLED::SDWipeSector(uint32_t sectorAddress, uint8_t wipeData)
{
    write(5, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_WRITE_SECTOR_BLOCK,
        OLEDUtil::getByte(sectorAddress, 2),
        OLEDUtil::getByte(sectorAddress, 1),
        OLEDUtil::getByte(sectorAddress));

    for (uint16_t b = 0; b < OLED_SD_SECTOR_SIZE; b++)
        write(wipeData);

    return getAck();
}

bool OLED::SDWipeSectors(uint32_t sectorAddress, uint32_t numSectors,
    uint32_t &sectorsWiped, bool displayProgress, uint8_t wipeData)
{    
    sectorsWiped = 0;
    uint32_t percent = 0;

    if (numSectors == 0)
        return true;

    uint16_t height = getDeviceHeight();
    uint16_t width = getDeviceWidth();

    bool success = true;
    for (uint32_t sector = sectorAddress; sector < numSectors; sector++)
    {
        if (displayProgress)
        {

            percent = sector * 100 / numSectors;
            
            drawProgressBar(0, height-9, width, 9, percent,
                OLED_PROGRESSBAR_COLOR_FORE_DEFAULT,
                OLED_PROGRESSBAR_COLOR_BACK_DEFAULT);
            
            String output = "s:" + (String)sector + " (" + percent + "%)";
            drawTextGraphic(1, height-8, output, 1, 1,
                OLED_FONT_COLOR_DEFAULT,
                OLED_FONT_SMALL, OLED_FONT_TRANSPARENT, OLED_FONT_PROPORTIONAL);
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
        drawRectangleWH(0, height-9, width, 9, OLED_PROGRESSBAR_COLOR_FORE_DEFAULT);
        drawTextGraphic(1, height-8, "s:"+(String)sectorsWiped+" (100%)", 1, 1,
            OLED_FONT_COLOR_DEFAULT,
            OLED_FONT_SMALL, OLED_FONT_TRANSPARENT, OLED_FONT_PROPORTIONAL);
    }

    return success;
}

/* // This simply takes too long to complete, so there's no point in it being here.
uint32_t OLED::SDWipeCard(uint8_t wipeData)
{
    SDWriteScreen(0x00);

    uint32_t sector = 1;
    while (SDWipeSector(sector))
    {
        drawTextGraphic(0,getDeviceHeight()-8,(String)""+sector+" ", 1, 1,
            OLED_FONT_COLOR_DEFAULT,
            OLED_FONT_SMALL, OLED_FONT_OPAQUE, OLED_FONT_PROPORTIONAL);
        sector++;
    }

    SDDrawScreen(0x00);
    SDWipeSector(0x00);

    return sector;
}
*/


bool OLED::SDWriteScreen(uint32_t sectorAddress,
    uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    write(2, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_WRITE_SCREENSHOT);
    writeSpatial(4, x, y, width, height);
    write(3,
        OLEDUtil::getByte(sectorAddress, 2),
        OLEDUtil::getByte(sectorAddress, 1),
        OLEDUtil::getByte(sectorAddress));
    return getAck();
}

bool OLED::SDWriteScreen(uint32_t sectorAddress)
{
    return SDWriteScreen(sectorAddress, 0, 0, getDeviceWidth(), getDeviceHeight());
}

 bool OLED::SDDrawScreen(uint32_t sectorAddress)
 {
     return SDDrawImage(sectorAddress, 0, 0, getDeviceWidth(), getDeviceHeight());
 }

// The width MUST match the width of the original image to display correctly.
// Height can be anything <= original height, image will be truncated accordingly.
bool OLED::SDDrawImage(uint32_t sectorAddress,
    uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    write(2, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_DISPLAY_IMAGE);
    writeSpatial(4, x, y, width, height);
    write(4, OLED_PRM_DRAW_IMAGE_16BIT,
        OLEDUtil::getByte(sectorAddress, 2),
        OLEDUtil::getByte(sectorAddress, 1),
        OLEDUtil::getByte(sectorAddress));
    return getAck();
}

bool OLED::SDRunCommand(uint32_t address)
{
    write(2, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_DISPLAY_OBJECT);
    writeLong(address);
    return getAck();
}

// UNTESTED!!!
bool OLED::SDPlayVideo(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
    uint8_t delayMs, uint16_t frameCount, uint32_t sectorAddress)
{
    write(2, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_DISPLAY_VIDEO);
    writeSpatial(4, x, y, width, height);
    write(2, OLED_PRM_DRAW_IMAGE_16BIT, delayMs);
    writeShort(frameCount);
    write(3,
        OLEDUtil::getByte(sectorAddress, 2),
        OLEDUtil::getByte(sectorAddress, 1),
        OLEDUtil::getByte(sectorAddress));
    return getAck();
}

// UNTESTED!!!
bool OLED::SDRunScript(uint32_t address)
{
    write(2, OLED_CMD_EXTENDED_SD, OLED_CMD_SD_RUN_4DSL_SCRIPT);
    writeLong(address);

    // This command will not return a response if successful.
    // If unsuccessful (or no SD is installed), NAK is returned.
    uint8_t response = 0x00;
    return (!getResponse(response) || response != OLED_NAK);
}
