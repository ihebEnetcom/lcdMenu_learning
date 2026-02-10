#include <LcdMenu.h>
#include <MenuScreen.h>
#include <ItemSubMenu.h>
#include <ItemWidget.h>
#include <ItemCommand.h>
#include <widget/WidgetRange.h>
#include <OneButton.h>
#include "Display.h"
#include "MotionControl.h"
#define UP_PIN 9
#define DOWN_PIN 8
#define ENTER_PIN 7

#define RED_LED_PIN 11
#define GREEN_LED_PIN 13

#define RED_BUTTON_PIN 10
#define GREEN_BUTTON_PIN 12

uint8_t state;
MotionControl motionControl;
Bounce2::Button limitSwitch;

OneButton upButton;
OneButton downButton;
OneButton enterButton;

OneButton greenButton;
OneButton redButton;

MENU_SCREEN(modeDroitScreen, modeDroitItems,
            ITEM_WIDGET(
                "eva", [](float position)
                {
                //positionEvasage = (position - 34.0) / 2;
                motionControl.setPositionEvasage((position - 34.0) * 5.0); },
                WIDGET_RANGE(50.0f, 0.05f, 34.0f, 80.0f, "%.2f", 0, false)),
            // ITEM_WIDGET(
            //     "rev", [](float position)
            //     {
            //     //positionRivetage = (position - 34.0) / 2;
            //     motionControl.setPositionRivetage((position - 33.6) * 5.0); },
            //     WIDGET_RANGE(50.0f, 0.05f, 33.6f, 80.0f, "%.2f", 0, false))

);
template <typename T, typename V = uint8_t>
class WidgetListArray : public BaseWidgetValue<V>
{
protected:
    const bool cycle;
    const T *values;
    const size_t count;
    V originalValue;

public:
    WidgetListArray(
        const T *values,
        const size_t count,
        const V activePosition,
        const char *format,
        const uint8_t cursorOffset,
        const bool cycle,
        void (*callback)(const V &))
        : BaseWidgetValue<V>(activePosition, format, cursorOffset, callback),
          cycle(cycle),
          values(values),
          count(count),
          originalValue(static_cast<V>(activePosition)) {}

protected:
    bool process(LcdMenu *menu, const unsigned char command) override
    {
        switch (command)
        {
        case UP:
            if (nextValue())
            {
                BaseWidgetValue<V>::handleChange();
            }
            return true;
        case DOWN:
            if (previousValue())
            {
                BaseWidgetValue<V>::handleChange();
            }
            return true;
        default:
            return false;
        }
    }

    uint8_t draw(char *buffer, const uint8_t start) override
    {
        if (start >= ITEM_DRAW_BUFFER_SIZE)
            return 0;
        return snprintf(buffer + start, ITEM_DRAW_BUFFER_SIZE - start, this->format, values[(uint8_t)this->value]);
    }

    bool nextValue()
    {
        if (static_cast<size_t>(this->value) + 1 < count)
        {
            this->value++;
            return true;
        }
        if (cycle && count > 0)
        {
            this->value = 0;
            return true;
        }
        return false;
    }

    bool previousValue()
    {
        if (static_cast<size_t>(this->value) > 0)
        {
            this->value--;
            return true;
        }
        if (cycle && count > 0)
        {
            this->value = static_cast<V>(count - 1);
            return true;
        }
        return false;
    }

    void startEdit() override { originalValue = static_cast<V>(this->value); }
    void cancelEdit() override { this->value = originalValue; }
};

template <typename T, size_t N>
inline BaseWidgetValue<uint8_t> *WIDGET_LIST_ARRAY(
    const T (&values)[N],
    const uint8_t activePosition = 0,
    const char *format = "%s",
    const uint8_t cursorOffset = 0,
    const bool cycle = false,
    void (*callback)(const uint8_t &) = nullptr)
{
    return new WidgetListArray<T, uint8_t>(values, N, activePosition, format, cursorOffset, cycle, callback);
}

static const uint8_t listItems[] = {34, 39, 42, 47, 50, 55};
static const size_t listItemsCount = sizeof(listItems) / sizeof(listItems[0]);
static void onExpanding(uint8_t index)
{
    if (index >= listItemsCount)
        return;
    const float position = static_cast<float>(listItems[index]);
    motionControl.setPositionEvasage((position - 34.0f) * 5.0f);
    motionControl.setPositionRivetage((position - 33.6f) * 5.0f);
}
MENU_SCREEN(fastScreen, fastScreenItems,
            ITEM_WIDGET(
                "", onExpanding,
                WIDGET_LIST_ARRAY(listItems, 0, "%d", 0, false))

);

MENU_SCREEN(mainScreen, mainScreenItems,
            ITEM_SUBMENU("d", modeDroitScreen),
            ITEM_SUBMENU("r", fastScreen), 
            );

void updateState(uint8_t s = 0);

void setup()
{

    pinMode(A0, OUTPUT);
    pinMode(A1, OUTPUT);
    pinMode(A2, OUTPUT);
    pinMode(A3, OUTPUT);
    motionControl.deactivate();
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(UP_PIN, INPUT_PULLUP);
    pinMode(DOWN_PIN, INPUT_PULLUP);
    pinMode(ENTER_PIN, INPUT_PULLUP);
    pinMode(GREEN_BUTTON_PIN, INPUT_PULLUP);
    pinMode(RED_BUTTON_PIN, INPUT_PULLUP);
    pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);

    display.begin();
    //----
    limitSwitch.attach(LIMIT_SWITCH_PIN, INPUT_PULLUP);
    limitSwitch.interval(5);
    limitSwitch.setPressedState(HIGH);

    display.menu().setScreen(mainScreen);
    upButton.setup(UP_PIN, INPUT_PULLUP, true);

    upButton.attachClick([](LcdMenu *m)
                         { m->process(UP); }, &display.menu());

    upButton.attachLongPressStart([](LcdMenu *m)
                                  { m->process(UP); }, &display.menu());
    //----
    downButton.setup(DOWN_PIN, INPUT_PULLUP, true);
    downButton.attachClick([](LcdMenu *m)
                           { m->process(DOWN); }, &display.menu());

    downButton.attachLongPressStart([](LcdMenu *m)
                                    { m->process(DOWN); }, &display.menu());
    //---
    enterButton.setup(ENTER_PIN, INPUT_PULLUP, true);
    enterButton.attachClick([](LcdMenu *m)
                            { m->process(ENTER); }, &display.menu());

    enterButton.attachLongPressStart([](LcdMenu *m)
                                     { m->process(BACK); }, &display.menu());
    //--
    greenButton.setup(GREEN_BUTTON_PIN);
    greenButton.attachClick(updateState, 1);
    greenButton.attachLongPressStart([]()
                                     {
            if (state == 1) {
            state = 9;
            } });
    greenButton.attachLongPressStop([]()
                                    {
        if (state == 9)
        state = 1; });
    redButton.setup(RED_BUTTON_PIN);
    redButton.attachPress([]()
                          { motionControl.deactivate(); });
    redButton.attachLongPressStart([]()
                                   {
        state =0;
        digitalWrite(RED_LED_PIN, LOW);
        motionControl.deactivate(); });
}

void loop()
{

    upButton.tick();
    downButton.tick();
    enterButton.tick();

    if (!state)
    {

        display.write(F("start"), 1);
        state = 8;
    }
    switch (state % 8)
    {
    case 0: // manual mode
        greenButton.tick();
        break;
    case 1:
        delay(100);
        while (state == 9)
        {
            // blink the led if the moteur dosen't work in this state
            if (!(motionControl.runEvasage() || motionControl.runRivetage()))
            {
                digitalWrite(GREEN_LED_PIN, LOW);
                delay(1000);
                digitalWrite(GREEN_LED_PIN, HIGH);
                break;
            }
            limitSwitch.update();
            greenButton.tick();
        }
        greenButton.tick();

    case 7: // error limit switch
        redButton.tick();
        break;
    }
    if (state != 8 && limitSwitch.isPressed())
    {
        delay(500);
        digitalWrite(RED_LED_PIN, HIGH);
        digitalWrite(GREEN_LED_PIN, LOW);
        motionControl.deactivate();
        display.write(F("!!\t ERROR\t!!"));
        display.write(F("\nLimit Switch"), 2, false);
        // display.write(F("or Disconnected"), 2, false);
        state = 7;
        redButton.tick();
        limitSwitch.update();
    }
    delay(10);
}

void updateState(uint8_t s = 0)
{
    switch (state % 8)
    {
    case 0: // manual mode
        if (s == 1)
        { // go to ready

            if (motionControl.init())
            {
                digitalWrite(GREEN_LED_PIN, HIGH);
                state = 1;
                display.render();
                return;
            }
            digitalWrite(RED_LED_PIN, HIGH);
            state = 7;
        }
    
}}
