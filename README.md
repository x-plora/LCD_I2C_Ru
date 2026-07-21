# LCD_I2C_Ru

Arduino library to control HD44780-compatible LCD displays through PCF8574 I2C adapters.
It supports the default `Wire` interface as well as `Wire1`, and Russian UTF-8 text.

```cpp
#include <LCD_I2C_Ru.h>

LCD_I2C_Ru lcd(0x27, 16, 2);

void setup() {
    lcd.begin();
    lcd.backlight();
    lcd.print("Привет, мир!");
}
```

Russian characters, including `Ё` and `ё`, are converted to the common
HD44780U-compatible Cyrillic character ROM. LCD modules with a different ROM
may display different glyphs.

For migration from `LiquidCrystal_I2C`, the aliases `init()`, `setBacklight()`,
`cursor_on/off()`, `blink_on/off()`, `printstr()`, and
`load_custom_character()` are also available.

## Buffered updates

The display geometry is fixed by the constructor; valid row counts are `1` to
`4`. After `begin()`, call `setAsync(true)` to keep a screen buffer. `print()`,
`setCursor()` and `clear()` then update the buffer, while `tick()` sends only
changed characters without blocking the main loop. `flush()` sends all pending
changes immediately. `getTransmissionErrorCount()` returns the number of I2C
transmissions rejected by the bus.

## Русский

Библиотека Arduino для управления LCD-дисплеями, совместимыми с HD44780,
через I2C-адаптеры PCF8574. Поддерживает стандартную шину `Wire`,
дополнительные шины вроде `Wire1` и вывод русского текста в UTF-8.

```cpp
#include <LCD_I2C_Ru.h>

LCD_I2C_Ru lcd(0x27, 16, 2);

void setup() {
    lcd.begin();
    lcd.backlight();
    lcd.print("Привет, мир!");
}
```

Русские символы, включая `Ё` и `ё`, преобразуются в кодировку
распространённой кириллической ROM HD44780U. На модулях с другой ROM
изображение отдельных символов может отличаться.

Для переноса скетчей с `LiquidCrystal_I2C` доступны алиасы `init()`,
`setBacklight()`, `cursor_on/off()`, `blink_on/off()`, `printstr()` и
`load_custom_character()`.

## Буферизированное обновление

Размер дисплея задаётся конструктором и после этого не меняется; допустимо от
`1` до `4` строк. После `begin()` вызовите `setAsync(true)`, чтобы включить
экранный буфер. Тогда `print()`, `setCursor()` и `clear()` изменяют буфер, а
`tick()` передаёт только изменившиеся символы и не блокирует основной цикл.
`flush()` отправляет все ожидающие изменения сразу.
`getTransmissionErrorCount()` возвращает число I2C-транзакций, завершившихся
ошибкой.
