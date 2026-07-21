/**
 * @file Functions.ino
 * @brief Demonstrates scrolling, cursor, blinking, and text-direction APIs.
 * @version 2.5.0
 * @copyright Copyright (C) 2020 Blackhack, GPL-3.0-or-later.
 *
 * LCD_I2C_Ru - Arduino library to control a 16x2 LCD via an I2C adapter based on PCF8574
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

#include <LCD_I2C_Ru.h>

LCD_I2C_Ru lcd(0x27, 16, 2); ///< Display used by the API demonstrations.

/**
 * @note Use F() with string literals passed to lcd.print() where supported.
 * It stores the literal in flash rather than RAM. See
 * https://www.arduino.cc/reference/en/language/variables/utilities/progmem/.
 */

/** @brief Initializes the display for the demonstrations. */
void setup()
{
    lcd.begin();
    lcd.backlight();
}

/** @brief Runs each display-function demonstration in sequence. */
void loop()
{
    /** @brief Demonstrates autoscroll mode. */
    lcd.setCursor(5, 0);
    lcd.print(F("Autoscrolling"));
    lcd.setCursor(10, 1);
    lcd.autoscroll();

    for (int i = 0; i < 10; i++)
    {
        lcd.print(i);
        delay(200);
    }

    lcd.noAutoscroll();
    lcd.clear();

    /** @brief Demonstrates display shifting in both directions. */
    lcd.setCursor(10, 0);
    lcd.print(F("To the left!"));
    for (int i = 0; i < 10; i++)
    {
        lcd.scrollDisplayLeft();
        delay(200);
    }
    lcd.clear();
    lcd.print(F("To the right!"));
    for (int i = 0; i < 10; i++)
    {
        lcd.scrollDisplayRight();
        delay(200);
    }
    lcd.clear();

    /** @brief Demonstrates the underline cursor. */
    lcd.setCursor(0, 0);
    lcd.cursor();
    lcd.print(F("Cursor"));
    delay(3000);
    lcd.clear();

    /** @brief Demonstrates cursor blinking. */
    lcd.setCursor(0, 0);
    lcd.blink();
    lcd.print(F("Cursor blink"));
    delay(3000);
    lcd.clear();

    /** @brief Demonstrates blinking without an underline cursor. */
    lcd.setCursor(0, 0);
    lcd.noCursor();
    lcd.print(F("Just blink"));
    delay(3000);
    lcd.noBlink();
    lcd.clear();
}
