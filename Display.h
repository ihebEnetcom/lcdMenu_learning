#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LcdMenu.h>
#include <LiquidCrystal_I2C.h>
#include <display/LiquidCrystal_I2CAdapter.h>
#include <renderer/CharacterDisplayRenderer.h>

class Display
{
public:
    Display();

    void begin();
    void render();

    void write(const __FlashStringHelper *text,
               uint8_t level = 0,
               bool clear = true);

    void write(const char *text,
               uint8_t level = 0,
               bool clear = true);
  

    LcdMenu &menu();

private:
    LiquidCrystal_I2C lcd;
    LiquidCrystal_I2CAdapter lcdAdapter;
    CharacterDisplayRenderer renderer;
    LcdMenu menuInstance;
};

extern Display display;

#endif
