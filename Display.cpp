#include "Display.h"

Display display;

Display::Display()
    : lcd(0x20, 16, 2), // 0x20
      lcdAdapter(&lcd),
      renderer(&lcdAdapter, 16, 2),
      menuInstance(renderer)
{
}

void Display::begin()
{
    renderer.begin();
}

void Display::write(const __FlashStringHelper *text,
                    uint8_t level,
                    bool clear)
{
    lcd.noBlink();
    if (clear)
        lcd.clear();

    if (level == 4)
        lcd.blink();
    else if (level)
        for (uint8_t i = 0; i < 4; i++)
        {
            renderer.moveCursor(0, i);
            renderer.draw((level == 1) ? '#' : '!');
            renderer.moveCursor(15, i);
            renderer.draw((level == 1) ? '#' : '!');
        }

    uint8_t c = 1, r = 0;
    const uint8_t maxRows = 4;
    PGM_P p = reinterpret_cast<PGM_P>(text);
    while (true)
    {
        char ch = pgm_read_byte(p++);
        if (ch == '\0')
            break;
        if (c == 15 || ch == '\n')
        {
            c = 1;
            r++;
        }
        if (r == maxRows)
            break;
        if (ch == '\n')
            continue;
        if (ch == '\t')
        {
            c += 2;
            continue;
        }
        renderer.moveCursor(c++, r);
        renderer.draw(ch);
    }
}

void Display::write(const char *text,
                    uint8_t level,
                    bool clear)
{
    lcd.noBlink();
    if (clear)
        lcd.clear();

    if (level == 4)
        lcd.blink();
    else if (level)
        for (uint8_t i = 0; i < 4; i++)
        {
            renderer.moveCursor(0, i);
            renderer.draw((level == 1) ? '#' : '!');
            renderer.moveCursor(15, i);
            renderer.draw((level == 1) ? '#' : '!');
        }

    uint8_t c = 1, r = 0;
    uint8_t len = strlen(text);

    for (uint8_t i = 0; i < len; i++)
    {
        if (c == 15 || text[i] == '\n')
        {
            c = 1;
            r++;
        }
        if (r == 4)
            break;
        if (text[i] == '\n')
            continue;
        if (text[i] == '\t')
        {
            c += 2;
            continue;
        }
        renderer.moveCursor(c++, r);
        renderer.draw(text[i]);
    }
}

void Display::render()
{
    menuInstance.show();
    lcd.clear();
    menuInstance.refresh();
}
LcdMenu &Display::menu()
{
    return menuInstance;
}
