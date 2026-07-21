#include <LCD_I2C_Ru.h>

LCD_I2C_Ru lcd(0x27, 16, 2);

unsigned long nextUpdate = 0;
unsigned long counter = 0;

void setup()
{
    lcd.begin();
    lcd.backlight();
    lcd.setAsync(true);
}

void loop()
{
    if (millis() >= nextUpdate)
    {
        nextUpdate = millis() + 1000;
        lcd.setCursor(0, 0);
        lcd.print("Счётчик: ");
        lcd.print(counter++);
    }

    // Sends one changed character per iteration. Other program work can run here.
    lcd.tick();
}
