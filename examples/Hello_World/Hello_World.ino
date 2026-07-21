/**
 * @file Hello_World.ino
 * @brief Demonstrates basic text output and backlight control.
 * @details Creates a 16x2 display on the default I2C bus.
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

LCD_I2C_Ru lcd(0x27, 16, 2); ///< Display at the common PCF8574 address.

/** @brief Initializes the LCD and enables its backlight. */
void setup()
{
    /** @note Use lcd.begin(false) when another component already initialized Wire.
     * On ESP32, lcd.begin(4, 5) maps Wire SDA to pin 4 and SCL to pin 5.
     */
    lcd.begin();

    lcd.backlight();
}

/** @brief Repeatedly renders and clears a greeting. */
void loop()
{
    lcd.print("     Hello"); // You can make spaces using well... spaces
    lcd.setCursor(5, 1); // Or setting the cursor in the desired position.
    lcd.print("World!");
    delay(500);

    /** @brief Flashes the backlight five times. */
    for (int i = 0; i < 5; ++i)
    {
        lcd.backlight();
        delay(50);
        lcd.noBacklight();
        delay(50);
    }

    lcd.backlight();
    lcd.clear();
    delay(500);
}
