/**
 * @file Hello_World_Wire1.ino
 * @brief Demonstrates an LCD connected to a secondary I2C bus.
 * @details Uses Wire1 and is suitable for boards exposing multiple TwoWire buses.
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

/** @brief Uses the secondary Wire1 bus available on boards such as Arduino Due or ESP32. */
extern TwoWire Wire1;
LCD_I2C_Ru lcd(Wire1, 0x27, 16, 2); ///< Display connected to the Wire1 bus.

/** @brief Initializes Wire1 through the LCD driver and enables the backlight. */
void setup()
{
    /** @note Use lcd.begin(false) when another component already initialized Wire1.
     * On ESP32, lcd.begin(4, 5) maps Wire1 SDA to pin 4 and SCL to pin 5.
     */
    lcd.begin();

    lcd.backlight();
}

/** @brief Repeatedly renders and clears a greeting on Wire1. */
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
