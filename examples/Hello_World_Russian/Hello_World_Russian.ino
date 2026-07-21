#include <LCD_I2C_Ru.h>

LCD_I2C_Ru lcd(0x27, 16, 2);

void setup()
{
    lcd.begin();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Привет, мир!");
    lcd.setCursor(0, 1);
    lcd.print("Ёжик и ёлка");
}

void loop()
{
}
