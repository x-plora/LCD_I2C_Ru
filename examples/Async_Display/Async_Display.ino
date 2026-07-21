/**
 * @file Async_Display.ino
 * @brief Demonstrates non-blocking buffered LCD updates.
 * @details tick() sends one changed character per loop iteration.
 * @version 2.5.0
 * @author Async mode by Kirill X-plora Chugreev
 * @copyright Copyright (C) 2026 Kirill X-plora Chugreev, GPL-3.0-or-later.
 */
#include <LCD_I2C_Ru.h>

LCD_I2C_Ru lcd(0x27, 16, 2); ///< Display updated through the asynchronous buffer.

unsigned long nextUpdate = 0; ///< Scheduled time for the next counter update.
unsigned long counter = 0; ///< Value rendered on the first LCD row.

/** @brief Initializes the display and enables buffered output. */
void setup()
{
    lcd.begin();
    lcd.backlight();
    lcd.setAsync(true);
}

/** @brief Updates the counter once per second and services one pending glyph. */
void loop()
{
    if (millis() >= nextUpdate)
    {
        nextUpdate = millis() + 1000;
        lcd.setCursor(0, 0);
        lcd.print("Счётчик: ");
        lcd.print(counter++);
    }

    /** @brief Sends one changed character; other application work can run here. */
    lcd.tick();
}
