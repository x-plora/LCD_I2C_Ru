/**
 * @file LCD_I2C_Ru.cpp
 * @brief Implements the PCF8574 and HD44780 communication logic.
 * @details Contains synchronous and buffered I2C output, UTF-8 Cyrillic
 * conversion, cursor management, and error accounting.
 * @version 2.5.0
 * @author Blackhack; Russian UTF-8 conversion by Ilya V. Danilov (mk90/LiquidCrystalRus); Async mode by Kirill X-plora Chugreev
 * @copyright Copyright (C) 2020 Blackhack, GPL-3.0-or-later.
 * @copyright Copyright (C) 2026 Kirill X-plora Chugreev, GPL-3.0-or-later.
 * @license GPL-3.0-or-later
 * @note Modified 2026-07-22: added asynchronous buffered mode.
 * @date 2026-07-22
 *
 * LCD_I2C_Ru - Arduino library to control a 16x2 LCD via an I2C adapter based on PCF8574
 * 2021-11-18 Brewmanz: make changes to also work for 20x4 LCD2004
 *
 * Copyright(C) 2020 Blackhack <davidaristi.0504@gmail.com>
 *
 * This program is free software : you can redistribute it and /or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see < https://www.gnu.org/licenses/>.
*/

#include "LCD_I2C_Ru.h"
#include "Wire.h"
#include <stdlib.h>
#include <string.h>

/** @brief Maximum row count addressable by the HD44780 row-offset table. */
static const uint8_t lcdMaxRows = 4;
/** @brief Maximum data characters grouped in one Wire transmission. */
static const uint8_t lcdMaxCharsPerI2cBurst = 4;

/**
 * @brief Maps Russian UTF-8 suffix bytes to common HD44780 Cyrillic ROM codes.
 * @details Derived from Ilya V. Danilov's mk90/LiquidCrystalRus project:
 * https://github.com/mk90/LiquidCrystalRus. Indexes are low six bits after
 * UTF-8 lead bytes 0xD0 or 0xD1.
 */
static const uint8_t utf8CyrillicToHd44780[64] = {
    0x70, 0x63, 0xbf, 0x79, 0xe4, 0x78, 0xe5, 0xc0,
    0xc1, 0xe6, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
    0x41, 0xa0, 0x42, 0xa1, 0xe0, 0x45, 0xa3, 0xa4,
    0xa5, 0xa6, 0x4b, 0xa7, 0x4d, 0x48, 0x4f, 0xa8,
    0x50, 0x43, 0x54, 0xa9, 0xaa, 0x58, 0xe1, 0xab,
    0xac, 0xe2, 0xad, 0xae, 0x62, 0xaf, 0xb0, 0xb1,
    0x61, 0xb2, 0xb3, 0xb4, 0xe3, 0x65, 0xb6, 0xb7,
    0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0x6f, 0xbe
};

LCD_I2C_Ru::LCD_I2C_Ru(TwoWire& wire, uint8_t address, uint8_t columns, uint8_t rows)
    : _wire(wire) // Use the TwoWire object passed as parameter.
    , _address(address), _columns(columns == 0 ? 1 : columns)
    , _rows(rows == 0 ? 1 : (rows > lcdMaxRows ? lcdMaxRows : rows))
    , _displayState(0x00), _entryState(0x00)
{
}

LCD_I2C_Ru::LCD_I2C_Ru(uint8_t address, uint8_t columns, uint8_t rows)
    : _wire(Wire) // Use the default object 'Wire'.
    , _address(address), _columns(columns == 0 ? 1 : columns)
    , _rows(rows == 0 ? 1 : (rows > lcdMaxRows ? lcdMaxRows : rows))
    , _displayState(0x00), _entryState(0x00)
{
}

LCD_I2C_Ru::~LCD_I2C_Ru()
{
    free(_shadow);
    free(_screen);
}

void LCD_I2C_Ru::begin(int sdaPin, int sclPin, bool beginWire)
{
#if defined (ESP32)
    /** @brief ESP32 requires explicit SDA and SCL pin configuration. */
    _wire.setPins(sdaPin, sclPin);
#else
    (void)sdaPin;
    (void)sclPin;
#endif
    begin(beginWire);
}

void LCD_I2C_Ru::begin(bool beginWire)
{
    if (beginWire)
        _wire.begin();

    const bool asyncWasEnabled = _async;
    allocateBuffers();
    _async = false;
    I2C_Write(0b00000000); // Clear i2c adapter
    delay(50); //Wait more than 40ms after powerOn.

    InitializeLCD();
    _async = asyncWasEnabled && hasBuffers();
    if (_async)
        invalidate();
}

void LCD_I2C_Ru::reinitialize()
{
    const bool asyncWasEnabled = _async;
    _async = false;
    _output.rs = 0;
    _output.rw = 0;

    const uint8_t functionSet = 0b00100000 | (_rows > 1 ? 0b00001000 : 0);
    LCD_WriteByte(functionSet);
    delayMicroseconds(37);
    display();
    clear();
    leftToRight();

    _utf8CyrillicLead = -1;
    _async = asyncWasEnabled && hasBuffers();
    if (_async)
        invalidate();
}

void LCD_I2C_Ru::backlight()
{
    _output.Led = 1;
    I2C_Write(0b00000000 | _output.Led << 3); // Led pin is independent from LCD data and control lines.
}

void LCD_I2C_Ru::noBacklight()
{
    _output.Led = 0;
    I2C_Write(0b00000000 | _output.Led << 3); // Led pin is independent from LCD data and control lines.
}

void LCD_I2C_Ru::clear()
{
    if (_async && hasBuffers())
    {
        memset(_shadow, ' ', _bufferSize);
        _cursorCol = 0;
        _cursorRow = 0;
        return;
    }

    _output.rs = 0;
    _output.rw = 0;

    LCD_WriteByte(0b00000001);
    delayMicroseconds(1600);
    if (hasBuffers())
    {
        memset(_shadow, ' ', _bufferSize);
        invalidate();
    }
    _cursorCol = 0;
    _cursorRow = 0;
}

void LCD_I2C_Ru::home()
{
    if (_async && hasBuffers())
    {
        _cursorCol = 0;
        _cursorRow = 0;
        return;
    }

    _output.rs = 0;
    _output.rw = 0;

    LCD_WriteByte(0b00000010);
    delayMicroseconds(1600);
    _cursorCol = 0;
    _cursorRow = 0;
}

/** @brief Updates the HD44780 entry-mode command for left-to-right text. */
void LCD_I2C_Ru::leftToRight()
{
    _output.rs = 0;
    _output.rw = 0;

    _entryState |= 1 << 1;

    LCD_WriteByte(0b00000100 | _entryState);
    delayMicroseconds(37);
}

/** @brief Updates the HD44780 entry-mode command for right-to-left text. */
void LCD_I2C_Ru::rightToLeft()
{
    _output.rs = 0;
    _output.rw = 0;

    _entryState &= ~(1 << 1);

    LCD_WriteByte(0b00000100 | _entryState);
    delayMicroseconds(37);
}

/** @brief Updates the HD44780 entry-mode command to enable autoscroll. */
void LCD_I2C_Ru::autoscroll()
{
    _output.rs = 0;
    _output.rw = 0;

    _entryState |= 1;

    LCD_WriteByte(0b00000100 | _entryState);
    delayMicroseconds(37);
}

/** @brief Updates the HD44780 entry-mode command to disable autoscroll. */
void LCD_I2C_Ru::noAutoscroll()
{
    _output.rs = 0;
    _output.rw = 0;

    _entryState &= ~1;

    LCD_WriteByte(0b00000100 | _entryState);
    delayMicroseconds(37);
}

/** @brief Updates the HD44780 display-control command to show the display. */
void LCD_I2C_Ru::display()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState |= 1 << 2;

    LCD_WriteByte(0b00001000 | _displayState);
    delayMicroseconds(37);
}

/** @brief Updates the HD44780 display-control command to hide the display. */
void LCD_I2C_Ru::noDisplay()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState &= ~(1 << 2);

    LCD_WriteByte(0b00001000 | _displayState);
    delayMicroseconds(37);
}

/** @brief Updates the HD44780 display-control command to show the cursor. */
void LCD_I2C_Ru::cursor()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState |= 1 << 1;

    LCD_WriteByte(0b00001000 | _displayState);
    delayMicroseconds(37);
}

/** @brief Updates the HD44780 display-control command to hide the cursor. */
void LCD_I2C_Ru::noCursor()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState &= ~(1 << 1);

    LCD_WriteByte(0b00001000 | _displayState);
    delayMicroseconds(37);
}

/** @brief Updates the HD44780 display-control command to enable blinking. */
void LCD_I2C_Ru::blink()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState |= 1;

    LCD_WriteByte(0b00001000 | _displayState);
    delayMicroseconds(37);
}

/** @brief Updates the HD44780 display-control command to disable blinking. */
void LCD_I2C_Ru::noBlink()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState &= ~1;

    LCD_WriteByte(0b00001000 | _displayState);
    delayMicroseconds(37);
}

/** @brief Issues the HD44780 display-shift-left command. */
void LCD_I2C_Ru::scrollDisplayLeft()
{
    _output.rs = 0;
    _output.rw = 0;

    LCD_WriteByte(0b00011000);
    delayMicroseconds(37);
}

/** @brief Issues the HD44780 display-shift-right command. */
void LCD_I2C_Ru::scrollDisplayRight()
{
    _output.rs = 0;
    _output.rw = 0;

    LCD_WriteByte(0b00011100);
    delayMicroseconds(37);
}

/** @brief Selects a CGRAM address and writes a custom glyph. */
void LCD_I2C_Ru::createChar(uint8_t location, const uint8_t charmap[8])
{
    const bool asyncWasEnabled = _async;
    _async = false;
    _output.rs = 0;
    _output.rw = 0;

    location %= 8;

    LCD_WriteByte(0b01000000 | (location << 3));
    delayMicroseconds(37);

    _output.rs = 1;
    for (uint8_t i = 0; i < 8; ++i)
    {
        LCD_WriteByte(charmap[i]);
        delayMicroseconds(41);
    }

    setCursor(0, 0); // Set the address pointer back to the DDRAM
    _cursorCol = 0;
    _cursorRow = 0;
    _async = asyncWasEnabled;
}

/** @brief Selects a clamped DDRAM cursor address. */
void LCD_I2C_Ru::setCursor(uint8_t col, uint8_t row)
{
    if (col >= _columns) { col = _columns - 1; }
    if (row >= _rows) { row = _rows - 1; }

    _cursorCol = col;
    _cursorRow = row;
    if (_async && hasBuffers())
        return;

    static const uint8_t row_offsets[lcdMaxRows] = {0x00, 0x40, 0x14, 0x54};
    _output.rs = 0;
    _output.rw = 0;

    const uint8_t newAddress = row_offsets[row] + col;

    LCD_WriteByte(0b10000000 | newAddress);
    delayMicroseconds(37);
}

size_t LCD_I2C_Ru::write(uint8_t character)
{
    if (_utf8CyrillicLead >= 0)
    {
        const uint8_t suffix = character & 0x3F;
        const bool validCyrillic =
            (_utf8CyrillicLead == 0 && suffix >= 0x10) || // U+0410..U+043F
            (_utf8CyrillicLead == 1 && suffix <= 0x0F) || // U+0440..U+044F
            (_utf8CyrillicLead == 0 && suffix == 0x01) || // U+0401: Ё
            (_utf8CyrillicLead == 1 && suffix == 0x11);   // U+0451: ё

        if (character >= 0x80 && character <= 0xBF && validCyrillic)
        {
            const uint8_t hd44780Character =
                (_utf8CyrillicLead == 0 && suffix == 0x01) ? 0xA2 :
                (_utf8CyrillicLead == 1 && suffix == 0x11) ? 0xB5 :
                utf8CyrillicToHd44780[suffix];
            _utf8CyrillicLead = -1;
            writeCharacter(hd44780Character);
            return 1;
        }

        /** @note Preserve an incomplete UTF-8 sequence as its original lead byte. */
        writeCharacter(0xD0 + _utf8CyrillicLead);
        _utf8CyrillicLead = -1;
    }

    if (character == 0xD0 || character == 0xD1)
    {
        _utf8CyrillicLead = character - 0xD0;
        return 1;
    }

    writeCharacter(character);

    return 1;
}

void LCD_I2C_Ru::setAsync(bool enabled)
{
    _async = enabled && hasBuffers();
    if (_async)
        invalidate();
}

#ifdef DEBUG_LCD_I2C_Ru
uint16_t LCD_I2C_Ru::getDirtyCount() const
{
    if (!_async || !hasBuffers())
        return 0;

    uint16_t dirty = 0;
    for (uint16_t i = 0; i < _bufferSize; ++i)
    {
        if (_shadow[i] != _screen[i])
            ++dirty;
    }
    return dirty;
}
#endif

bool LCD_I2C_Ru::isBusy() const
{
    if (!_async || !hasBuffers())
        return false;

    for (uint16_t i = 0; i < _bufferSize; ++i)
    {
        if (_shadow[i] != _screen[i])
            return true;
    }
    return false;
}

void LCD_I2C_Ru::tick()
{
    tick(4);
}

void LCD_I2C_Ru::tick(uint8_t maxChars)
{
    if (!_async || !hasBuffers() || maxChars == 0)
        return;

    uint8_t written = 0;
    while (written < maxChars)
    {
        uint8_t burstLimit = maxChars - written;
        if (burstLimit > lcdMaxCharsPerI2cBurst)
            burstLimit = lcdMaxCharsPerI2cBurst;

        uint16_t startIndex = 0;
        const uint8_t count = findDirtyRun(&startIndex, burstLimit);
        if (count == 0)
            break;

        if (!writeRunBurst(startIndex, count))
            break;
        for (uint8_t i = 0; i < count; ++i)
            _screen[startIndex + i] = _shadow[startIndex + i];

        _scanIndex = startIndex + count;
        if (_scanIndex >= _bufferSize)
            _scanIndex = 0;
        written += count;
    }
}

void LCD_I2C_Ru::flush()
{
    while (isBusy())
    {
        const uint32_t errorCount = _transmissionErrorCount;
        tick(lcdMaxCharsPerI2cBurst);
        if (_transmissionErrorCount != errorCount)
            break;
    }
}

void LCD_I2C_Ru::invalidate()
{
    if (hasBuffers())
    {
        memset(_screen, 0xFF, _bufferSize);
        _scanIndex = 0;
    }
}

bool LCD_I2C_Ru::allocateBuffers()
{
    const uint16_t requestedSize = static_cast<uint16_t>(_columns) * _rows;
    if (_bufferSize == requestedSize && hasBuffers())
    {
        memset(_shadow, ' ', _bufferSize);
        invalidate();
        return true;
    }

    free(_shadow);
    free(_screen);
    _shadow = static_cast<uint8_t *>(malloc(requestedSize));
    _screen = static_cast<uint8_t *>(malloc(requestedSize));
    if (_shadow == nullptr || _screen == nullptr)
    {
        free(_shadow);
        free(_screen);
        _shadow = nullptr;
        _screen = nullptr;
        _bufferSize = 0;
        return false;
    }

    _bufferSize = requestedSize;
    memset(_shadow, ' ', _bufferSize);
    invalidate();
    return true;
}

bool LCD_I2C_Ru::hasBuffers() const
{
    return _shadow != nullptr && _screen != nullptr && _bufferSize != 0;
}

uint8_t LCD_I2C_Ru::findDirtyRun(uint16_t *startIndex, uint8_t maxChars)
{
    if (maxChars == 0)
        return 0;

    for (uint16_t scanned = 0; scanned < _bufferSize; ++scanned)
    {
        const uint16_t index = _scanIndex;
        ++_scanIndex;
        if (_scanIndex >= _bufferSize)
            _scanIndex = 0;

        if (_shadow[index] == _screen[index])
            continue;

        const uint8_t column = index % _columns;
        uint8_t count = 1;
        while (count < maxChars && (column + count) < _columns)
        {
            const uint16_t nextIndex = index + count;
            if (_shadow[nextIndex] != _screen[nextIndex])
                ++count;
            else
                break;
        }

        *startIndex = index;
        return count;
    }
    return 0;
}

bool LCD_I2C_Ru::writeRunBurst(uint16_t startIndex, uint8_t count)
{
    static const uint8_t rowOffsets[lcdMaxRows] = {0x00, 0x40, 0x14, 0x54};
    const uint8_t row = startIndex / _columns;
    const uint8_t column = startIndex % _columns;

    _output.rs = 0;
    _output.rw = 0;
    _wire.beginTransmission(_address);
    writeByteToBurst(0x80 | (rowOffsets[row] + column));

    _output.rs = 1;
    for (uint8_t i = 0; i < count; ++i)
        writeByteToBurst(_shadow[startIndex + i]);

    const uint8_t status = _wire.endTransmission();
    if (status != 0)
    {
        ++_transmissionErrorCount;
        return false;
    }
#ifdef DEBUG_LCD_I2C_Ru
    ++_sentBurstCount;
    _sentCharCount += count;
#endif
    delayMicroseconds(50);
    return true;
}

void LCD_I2C_Ru::writeByteToBurst(uint8_t output)
{
    _wire.write(_output.getHighData(output, true));
    _wire.write(_output.getHighData(output, false));
    _wire.write(_output.getLowData(output, true));
    _wire.write(_output.getLowData(output, false));
}

void LCD_I2C_Ru::writeCharacter(uint8_t character)
{
    const uint16_t characterIndex = _cursorRow * _columns + _cursorCol;
    if (_async && hasBuffers())
    {
        _shadow[characterIndex] = character;
    }
    else
    {
        _output.rs = 1;
        _output.rw = 0;
        LCD_WriteByte(character);
        delayMicroseconds(41);
        if (hasBuffers())
            _shadow[characterIndex] = character;
    }

    ++_cursorCol;
    if (_cursorCol >= _columns)
    {
        _cursorCol = 0;
        if (_cursorRow + 1 < _rows)
            ++_cursorRow;
    }

}

void LCD_I2C_Ru::InitializeLCD()
{
    /** @details Follows HD44780U Figure 24, "Initializing by Instruction" in 4-bit mode. */
    _output.rs = 0;
    _output.rw = 0;

    LCD_WriteHighNibble(0b00110000);
    delayMicroseconds(4200);
    LCD_WriteHighNibble(0b00110000);
    delayMicroseconds(150);
    LCD_WriteHighNibble(0b00110000);
    delayMicroseconds(37);
    LCD_WriteHighNibble(0b00100000); // Function Set - 4 bits mode
    delayMicroseconds(37);
    const uint8_t functionSet = 0b00100000 | (_rows > 1 ? 0b00001000 : 0);
    LCD_WriteByte(functionSet); // Function Set - 4-bit interface, configured row count, 5x8 font
    delayMicroseconds(37);

    display();
    clear();
    leftToRight();
}

void LCD_I2C_Ru::I2C_Write(uint8_t output)
{
    _wire.beginTransmission(_address);
    _wire.write(output);
    if (_wire.endTransmission() != 0)
        ++_transmissionErrorCount;
}

void LCD_I2C_Ru::LCD_WriteHighNibble(uint8_t output)
{
    I2C_Write(_output.getHighData(output, true));
    delayMicroseconds(1); // High part of enable should be >450 nS
    I2C_Write(_output.getHighData(output, false));
}

void LCD_I2C_Ru::LCD_WriteByte(uint8_t output)
{
    LCD_WriteHighNibble(output);
    delayMicroseconds(37); // I think we need a delay between half byte writes, but no sure how long it needs to be.

    I2C_Write(_output.getLowData(output, true));
    delayMicroseconds(1); // High part of enable should be >450 nS
    I2C_Write(_output.getLowData(output, false));

    /** @note Commands apply their own post-execution delay because timings differ. */
}
