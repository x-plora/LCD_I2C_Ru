/**
 * @file LCD_I2C_Ru.h
 * @brief Declares an I2C driver for HD44780-compatible LCD displays.
 * @details Supports PCF8574 backpacks, Russian UTF-8 conversion, and optional
 * buffered display updates.
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

#pragma once

#ifndef _LCD_I2C_Ru_h
#define _LCD_I2C_Ru_h

#include "Arduino.h"

/**
 * TwoWire class declaration is required for optionally using Wire1, Wire2.. objects
 * for I2C interface.
 * Forward declaration of TwoWire avoids include of Wire.h here and in user sketch.
 */
#ifdef ARDUINO_ARCH_MBED
/** @brief Uses the Mbed-specific I2C type on Arduino Mbed platforms. */
namespace arduino {class MbedI2C;}
typedef arduino::MbedI2C TwoWire;
#else
class TwoWire;
#endif

/**
 * @class LCD_I2C_Ru
 * @brief Controls an HD44780-compatible LCD through a PCF8574 I2C adapter.
 * @details Geometry is configured once in the constructor. The driver can use
 * the default Wire bus or an explicitly supplied TwoWire instance.
 */
class LCD_I2C_Ru : public Print
{
public:
    /**
     * This constructor just uses the default TwoWire object 'Wire'.
     */
    /** @brief Creates a display using the default Wire bus.
     * @param address Seven-bit I2C address of the PCF8574 adapter.
     * @param columns Number of display columns; zero is normalized to one.
     * @param rows Number of display rows; values are normalized to the range 1-4.
     */
    LCD_I2C_Ru(uint8_t address, uint8_t columns = 16, uint8_t rows = 2);

    /**
     * This constructor takes a TwoWire object so that the driver can
     * work on another interface.
     * In case that the hardware has a second TwoWire interface
     * which should be used for the LCD, just pass 'Wire1' as the
     * first parameter. For the 3rd interface use 'Wire2', and so on.
     *
     * There is no need for the sketch to include the Wire.h header.
     * Just declare the other TwoWire object to be used as 'extern'.
     *
     * Example:
     *  #include <LCD_I2C_Ru.h>
     *  extern TwoWire Wire1;
     *  LCD_I2C_Ru(Wire1, 0x27, 16, 2);
     *
     */
    /** @brief Creates a display using a specified I2C bus.
     * @param wire I2C bus instance, such as Wire1.
     * @param address Seven-bit I2C address of the PCF8574 adapter.
     * @param columns Number of display columns.
     * @param rows Number of display rows in the range 1-4.
     */
    LCD_I2C_Ru(TwoWire& wire, uint8_t address, uint8_t columns = 16, uint8_t rows = 2);
    /** @brief Releases the optional asynchronous screen buffers. */
    ~LCD_I2C_Ru();

    /**
     * Some microcontrollers (like ESP32) require to set sda pin and
     * scl pin for I2C.
     */
    /** @brief Initializes the display and optionally configures ESP32 I2C pins.
     * @param sdaPin SDA pin on ESP32; ignored on other platforms.
     * @param sclPin SCL pin on ESP32; ignored on other platforms.
     * @param beginWire Whether to call TwoWire::begin().
     */
    void begin(int sdaPin, int sclPin, bool beginWire = true);

    /** @brief Initializes the display controller.
     * @param beginWire Whether to call TwoWire::begin() before initialization.
     */
    void begin(bool beginWire = true);
    /** @brief Compatibility alias for begin(). */
    void init() { begin(); }
    /** @brief Restores an already powered and configured display controller.
     * @details Does not call TwoWire::begin(), allocate screen buffers, or run
     * the HD44780 power-on synchronization sequence. The display must already
     * be operating in 4-bit mode. Custom characters are lost and must be
     * created again by the caller.
     */
    void reinitialize();
    /** @brief Turns the PCF8574-controlled backlight on. */
    void backlight();
    /** @brief Turns the PCF8574-controlled backlight off. */
    void noBacklight();

    /** @brief Clears the display or asynchronous screen buffer. */
    void clear();
    /** @brief Moves the cursor to the origin. */
    void home();
    /** @brief Sets left-to-right character entry mode. */
    void leftToRight();
    /** @brief Sets right-to-left character entry mode. */
    void rightToLeft();
    /** @brief Enables display shifting while characters are written. */
    void autoscroll();
    /** @brief Disables display shifting while characters are written. */
    void noAutoscroll();
    /** @brief Makes display contents visible. */
    void display();
    /** @brief Hides display contents without clearing DDRAM. */
    void noDisplay();
    /** @brief Shows the underline cursor. */
    void cursor();
    /** @brief Hides the underline cursor. */
    void noCursor();
    /** @brief Enables cursor blinking. */
    void blink();
    /** @brief Disables cursor blinking. */
    void noBlink();
    /** @brief Shifts visible display contents one position left. */
    void scrollDisplayLeft();
    /** @brief Shifts visible display contents one position right. */
    void scrollDisplayRight();
    /** @brief Stores an 8-byte glyph in one CGRAM location.
     * @param location Custom-character location, normalized to 0-7.
     * @param charmap Eight HD44780 glyph rows.
     */
    void createChar(uint8_t location, const uint8_t charmap[8]);
    /** @brief Sets the cursor position, clamped to constructor geometry.
     * @param col Zero-based column.
     * @param row Zero-based row.
     */
    void setCursor(uint8_t col, uint8_t row);

    /** @brief Enables or disables buffered, non-blocking updates.
     * @param enabled True to buffer screen changes after begin().
     * @note Buffer allocation can fail on memory-constrained boards.
     */
    void setAsync(bool enabled);
    /** @brief Sends at most one changed character to the display. */
    void tick();
    /** @brief Sends a bounded number of changed characters.
     * @param maxChars Maximum characters to process; one I2C transaction has at most four.
     */
    void tick(uint8_t maxChars);
    /** @brief Reports whether buffered changes remain unsent.
     * @return True when tick() or flush() still has work to do.
     */
    bool isBusy() const;
    /** @brief Sends all pending changes, stopping when an I2C error occurs. */
    void flush();
    /** @brief Marks the complete screen buffer dirty for retransmission. */
    void invalidate();

    /** @brief Number of I2C transmissions for which endTransmission() returned an error. */
    uint32_t getTransmissionErrorCount() const { return _transmissionErrorCount; }

#ifdef DEBUG_LCD_I2C_Ru
    /** @brief Number of characters still waiting in the asynchronous screen buffer. */
    uint16_t getDirtyCount() const;
    /** @brief Number of successfully transmitted asynchronous I2C bursts. */
    uint32_t getSentBurstCount() const { return _sentBurstCount; }
    /** @brief Number of characters successfully transmitted by asynchronous bursts. */
    uint32_t getSentCharCount() const { return _sentCharCount; }
#endif

    /** @brief Writes one byte, converting Russian UTF-8 sequences when applicable.
     * @param character Byte supplied by Print::print() or a direct caller.
     * @return Always 1, matching the Print contract.
     */
    size_t write(uint8_t character) override;

    /** @name LiquidCrystal_I2C compatibility aliases */
    /** @{ */
    void blink_on() { blink(); }
    void blink_off() { noBlink(); }
    void cursor_on() { cursor(); }
    void cursor_off() { noCursor(); }
    void setBacklight(uint8_t value) { value ? backlight() : noBacklight(); }
    void load_custom_character(uint8_t location, uint8_t *charmap) { createChar(location, charmap); }
    void printstr(const char text[]) { print(text); }
    /** @} */

private:
    /** @brief Sends the HD44780 power-on command sequence. */
    void InitializeLCD();
    /** @brief Allocates two geometry-sized buffers for asynchronous mode.
     * @return True when both buffers are available.
     */
    bool allocateBuffers();
    /** @brief Checks whether asynchronous buffers are allocated. */
    bool hasBuffers() const;
    /** @brief Finds a contiguous changed run in the shadow buffer. */
    uint8_t findDirtyRun(uint16_t *startIndex, uint8_t maxChars);
    /** @brief Sends one changed run as an I2C burst. */
    bool writeRunBurst(uint16_t startIndex, uint8_t count);
    /** @brief Appends one HD44780 byte to the active I2C burst. */
    void writeByteToBurst(uint8_t output);
    /** @brief Writes one already-converted HD44780 character. */
    void writeCharacter(uint8_t character);
    /** @brief Sends one PCF8574 output byte and records transmission errors. */
    void I2C_Write(uint8_t output);
    /** @brief Sends one complete byte to the HD44780 in four-bit mode. */
    void LCD_WriteByte(uint8_t output);
    /** @brief Sends the high nibble of one HD44780 byte. */
    inline void LCD_WriteHighNibble(uint8_t output);

private:
    TwoWire& _wire; ///< I2C bus used for all PCF8574 transmissions.
    const uint8_t _address; ///< Seven-bit PCF8574 I2C address.
    /** @brief Geometry is fixed at construction time; HD44780 supports at most four rows. */
    const uint8_t _columns; ///< Immutable display width set by the constructor.
    const uint8_t _rows; ///< Immutable display height, normalized to one through four.
    uint8_t _displayState; ///< Cached HD44780 display, cursor, and blink flags.
    uint8_t _entryState; ///< Cached HD44780 text direction and autoscroll flags.
    bool _async = false; ///< True while writes target the shadow buffer.
    uint8_t *_shadow = nullptr; ///< Desired screen contents for asynchronous mode.
    uint8_t *_screen = nullptr; ///< Last successfully transmitted screen contents.
    uint16_t _bufferSize = 0; ///< Number of bytes in each screen buffer.
    uint8_t _cursorCol = 0; ///< Buffered cursor column.
    uint8_t _cursorRow = 0; ///< Buffered cursor row.
    uint16_t _scanIndex = 0; ///< Next buffer position to inspect for changes.
    uint32_t _transmissionErrorCount = 0; ///< Cumulative failed I2C transmissions.
#ifdef DEBUG_LCD_I2C_Ru
    uint32_t _sentBurstCount = 0; ///< Cumulative successful asynchronous I2C bursts.
    uint32_t _sentCharCount = 0; ///< Cumulative characters sent by asynchronous bursts.
#endif
    /** @brief First byte class of a pending Russian UTF-8 sequence; -1 if absent. */
    int8_t _utf8CyrillicLead = -1; ///< Pending UTF-8 lead-byte class or -1.

    /* This struct helps us constructing the I2C output based on data and control outputs.
       Because the LCD is set to 4-bit mode, 4 bits of the I2C output are for the control outputs
       while the other 4 bits are for the 8 bits of data which are send in parts using the enable output.*/
    /** @brief Stores PCF8574 control-pin states and builds output bytes. */
    struct OutputState
    {
        uint8_t rs = 0; ///< Register-select pin state.
        uint8_t rw = 0; ///< Read/write pin state.
        uint8_t Led = 0; ///< Backlight pin state.

        /** @brief Builds output for the low nibble of an HD44780 byte. */
        uint8_t getLowData(uint8_t data, uint8_t E) const
        {
            return getCommonData(E) | ((data & 0x0F) << 4);
        }

        /** @brief Builds output for the high nibble of an HD44780 byte. */
        uint8_t getHighData(uint8_t data, uint8_t E) const
        {
            return getCommonData(E) | (data & 0xF0);
        }

    private:
        /** @brief Combines the PCF8574 control-pin states. */
        inline uint8_t getCommonData(uint8_t E) const {
            return rs | (rw << 1) | (E << 2) | (Led << 3);
        }
    } _output;
};

#endif // #ifndef _LCD_I2C_Ru_h
