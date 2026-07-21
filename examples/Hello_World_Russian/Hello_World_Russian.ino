/**
 * @file Hello_World_Russian.ino
 * @brief Demonstrates Russian UTF-8 output, including Ё and ё.
 * @version 2.5.0
 */
#include <LCD_I2C_Ru.h>

LCD_I2C_Ru lcd(0x27, 16, 2); ///< Display receiving UTF-8 text.

/** @brief Initializes the display and writes two Russian lines. */
void setup()
{
    lcd.begin();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Привет, мир!");
    lcd.setCursor(0, 1);
    lcd.print("Ёжик и ёлка");
}

/** @brief Leaves the greeting on screen. */
void loop()
{
}
