/*
    LCD_I2C_Ru - Arduino library to control a 16x2 LCD via an I2C adapter based on PCF8574
    * 2021-11-18 Brewmanz: make changes to also work for 20x4 LCD2004

    Copyright(C) 2020 Blackhack <davidaristi.0504@gmail.com>

    This program is free software : you can redistribute it and /or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.If not, see < https://www.gnu.org/licenses/>.
*/

#include "LCD_I2C_Ru.h"
#include "Wire.h"

// Russian UTF-8 conversion derived from LiquidCrystal_I2C_Ru.
// See THIRD_PARTY_NOTICES.md for its MIT license notice.
// Indexes are the low six bits of a byte following UTF-8 lead byte 0xD0 or 0xD1.
// The values target the common HD44780U-compatible Cyrillic character ROM.
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
    , _address(address), _columnMax(columns-1), _rowMax(rows-1), _displayState(0x00), _entryState(0x00)
{
}

LCD_I2C_Ru::LCD_I2C_Ru(uint8_t address, uint8_t columns, uint8_t rows)
    : _wire(Wire) // Use the default object 'Wire'.
    , _address(address), _columnMax(columns-1), _rowMax(rows-1), _displayState(0x00), _entryState(0x00)
{
}

void LCD_I2C_Ru::begin(int sdaPin, int sclPin, bool beginWire)
{
#if defined (ESP32)
    // ESP32 requires setting sda and scl pins.
    _wire.setPins(sdaPin, sclPin);
#endif
    begin(beginWire);
}

void LCD_I2C_Ru::begin(bool beginWire)
{

    if (beginWire)
        _wire.begin();

    I2C_Write(0b00000000); // Clear i2c adapter
    delay(50); //Wait more than 40ms after powerOn.

    InitializeLCD();
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
    _output.rs = 0;
    _output.rw = 0;

    LCD_WriteByte(0b00000001);
    delayMicroseconds(1600);
}

void LCD_I2C_Ru::home()
{
    _output.rs = 0;
    _output.rw = 0;

    LCD_WriteByte(0b00000010);
    delayMicroseconds(1600);
}

// Part of Entry mode set
void LCD_I2C_Ru::leftToRight()
{
    _output.rs = 0;
    _output.rw = 0;

    _entryState |= 1 << 1;

    LCD_WriteByte(0b00000100 | _entryState);
    delayMicroseconds(37);
}

// Part of Entry mode set
void LCD_I2C_Ru::rightToLeft()
{
    _output.rs = 0;
    _output.rw = 0;

    _entryState &= ~(1 << 1);

    LCD_WriteByte(0b00000100 | _entryState);
    delayMicroseconds(37);
}

// Part of Entry mode set
void LCD_I2C_Ru::autoscroll()
{
    _output.rs = 0;
    _output.rw = 0;

    _entryState |= 1;

    LCD_WriteByte(0b00000100 | _entryState);
    delayMicroseconds(37);
}

// Part of Entry mode set
void LCD_I2C_Ru::noAutoscroll()
{
    _output.rs = 0;
    _output.rw = 0;

    _entryState &= ~1;

    LCD_WriteByte(0b00000100 | _entryState);
    delayMicroseconds(37);
}

// Part of Display control
void LCD_I2C_Ru::display()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState |= 1 << 2;

    LCD_WriteByte(0b00001000 | _displayState);
    delayMicroseconds(37);
}

// Part of Display control
void LCD_I2C_Ru::noDisplay()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState &= ~(1 << 2);

    LCD_WriteByte(0b00001000 | _displayState);
    delayMicroseconds(37);
}

// Part of Display control
void LCD_I2C_Ru::cursor()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState |= 1 << 1;

    LCD_WriteByte(0b00001000 | _displayState);
    delayMicroseconds(37);
}

// Part of Display control
void LCD_I2C_Ru::noCursor()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState &= ~(1 << 1);

    LCD_WriteByte(0b00001000 | _displayState);
    delayMicroseconds(37);
}

// Part of Display control
void LCD_I2C_Ru::blink()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState |= 1;

    LCD_WriteByte(0b00001000 | _displayState);
    delayMicroseconds(37);
}

// Part of Display control
void LCD_I2C_Ru::noBlink()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState &= ~1;

    LCD_WriteByte(0b00001000 | _displayState);
    delayMicroseconds(37);
}

// Part of Cursor or display shift
void LCD_I2C_Ru::scrollDisplayLeft()
{
    _output.rs = 0;
    _output.rw = 0;

    LCD_WriteByte(0b00011000);
    delayMicroseconds(37);
}

// Part of Cursor or display shift
void LCD_I2C_Ru::scrollDisplayRight()
{
    _output.rs = 0;
    _output.rw = 0;

    LCD_WriteByte(0b00011100);
    delayMicroseconds(37);
}

// Set CGRAM address
void LCD_I2C_Ru::createChar(uint8_t location, const uint8_t charmap[8])
{
    _output.rs = 0;
    _output.rw = 0;

    location %= 8;

    LCD_WriteByte(0b01000000 | (location << 3));
    delayMicroseconds(37);

    for (int i = 0; i < 8; i++)
        write(charmap[i]);

    setCursor(0, 0); // Set the address pointer back to the DDRAM
}

// Set DDRAM address
void LCD_I2C_Ru::setCursor(uint8_t col, uint8_t row)
{
    static const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    _output.rs = 0;
    _output.rw = 0;

    if(col > _columnMax) { col = _columnMax; } // sanity limits
    if(row > _rowMax) { row = _rowMax; } // sanity limits

    const uint8_t newAddress = row_offsets[row] + col;

    LCD_WriteByte(0b10000000 | newAddress);
    delayMicroseconds(37);
}

size_t LCD_I2C_Ru::write(uint8_t character)
{
    _output.rs = 1;
    _output.rw = 0;

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
            LCD_WriteByte(hd44780Character);
            delayMicroseconds(41);
            return 1;
        }

        // Preserve an incomplete sequence as its original lead byte.
        LCD_WriteByte(0xD0 + _utf8CyrillicLead);
        delayMicroseconds(41);
        _utf8CyrillicLead = -1;
    }

    if (character == 0xD0 || character == 0xD1)
    {
        _utf8CyrillicLead = character - 0xD0;
        return 1;
    }

    LCD_WriteByte(character);
    delayMicroseconds(41);

    return 1;
}

void LCD_I2C_Ru::InitializeLCD()
{
    // See HD44780U datasheet "Initializing by Instruction" Figure 24 (4-Bit Interface)
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
    LCD_WriteByte(0b00101000); // Function Set - 4 bits(Still), 2 lines, 5x8 font
    delayMicroseconds(37);

    display();
    clear();
    leftToRight();
}

void LCD_I2C_Ru::I2C_Write(uint8_t output)
{
    _wire.beginTransmission(_address);
    _wire.write(output);
    _wire.endTransmission();
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

    //delayMicroseconds(37); // Some commands have different timing requirement,
                             // so every command should handle its own delay after execution
}
