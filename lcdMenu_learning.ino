#include <LcdMenu.h>
#include <MenuScreen.h>
#include <ItemSubMenu.h>
#include <ItemWidget.h>
#include <ItemCommand.h>
#include <widget/WidgetRange.h>
#include <input/ButtonAdapter.h>
#include <OneButton.h>
#include "Display.h"
#include "MotionControl.h"
#include <widget/WidgetList.h>
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

// MENU_SCREEN(modeDroitScreen, modeDroitItems,
//             ITEM_WIDGET(
//                 "set", [](float position)
//                 {
//                 //positionEvasage = (position - 34.0) / 2;
//                 motionControl.setPositionEvasage((position - 34.0) * 5.0);
//                 motionControl.setPositionRivetage((position - 33.6) * 5.0); },
//                 WIDGET_RANGE(50.0f, 0.05f, 34.0f, 80.0f, "%.2f", 0, false)),
//             // ITEM_WIDGET(
//             //     "Riveting", [](float position)
//             //     {
//             //     //positionRivetage = (position - 34.0) / 2;
//             //      },
//             //     WIDGET_RANGE(50.0f, 0.05f, 33.6f, 80.0f, "%.2f", 0, false))

// );
static const std::vector<uint8_t> listItems = {34, 39, 42, 47, 50, 55};
static void onExpanding(uint8_t index)
{
    if (index >= listItems.size())
        return;
    const float position = static_cast<float>(listItems[index]);
    motionControl.setPositionEvasage((position - 34.0f) * 5.0f);
    motionControl.setPositionRivetage((position - 33.6f) * 5.0f);
}
MENU_SCREEN(fastScreen, fastScreenItems,
            ITEM_WIDGET(
                "Expanding", onExpanding,
                WIDGET_LIST(listItems, 0, "%d", 0, false))

);

MENU_SCREEN(mainScreen, mainScreenItems,
            // ITEM_SUBMENU("M D", modeDroitScreen),
            ITEM_SUBMENU("M R", fastScreen), );

void updateState(uint8_t s = 0);

void setup()
{
    Serial.begin(9600);

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
    greenButton.attachLongPressStart([](){
            Serial.println(F("green long press start"));
            if (state == 1) {
            state = 9;
            } 
          });
    greenButton.attachLongPressStop([]()
                                    {
        if (state == 9)
        state = 1; });
    redButton.setup(RED_BUTTON_PIN);
    redButton.attachPress([](){ motionControl.deactivate(); });
    redButton.attachLongPressStart([](){
        state =0;
        digitalWrite(RED_LED_PIN, LOW);
        motionControl.deactivate();
      });
            
}
unsigned long lastPrint = 0;
void loop()
{

    upButton.tick();
    downButton.tick();
    enterButton.tick();

    if (millis() - lastPrint > 1000) {
        Serial.print(F("Current State: "));
        Serial.println(state);
        lastPrint = millis();
    }
    if (!state)
    {

        display.write(F("start"), 1);
        state = 8;
    }
      switch (state % 8) {
    case 0:  // manual mode
      greenButton.tick();
      break;
    case 1:
      delay(100);
      while (state == 9) {
        // blink the led if the moteur dosen't work in this state
        if (!(motionControl.runEvasage() || motionControl.runRivetage())) {
          digitalWrite(GREEN_LED_PIN, LOW);
          delay(1000);
          digitalWrite(GREEN_LED_PIN, HIGH);
          break;
        }
        limitSwitch.update();
        greenButton.tick();
      }
        greenButton.tick();

    case 7:  //error limit switch
      redButton.tick();
      break;
  }
  if (state != 8 && limitSwitch.isPressed()) {
    delay(500);
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
    motionControl.deactivate();
    display.write(F("!!\t ERROR\t!!"));
    display.write(F("\nLimit Switch"), 2, false);
    //display.write(F("or Disconnected"), 2, false);
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
        return;
    case 1: // ready
    case 2: // mode Ecarte
        // if (s == 3)
        // { // go to manual mode
        //     motionControl.deactivate();
        //     display.write(F("\n  Manual Mode"), 1);
        //     digitalWrite(GREEN_LED_PIN, LOW);
        //     state = 8;
        // }
        // return;
    case 3:
    //   if (s == 1) {  // go to deplacement rivetage
    //     motionControl.setPositionEvasage(ladder.evasage[state / 8] / 100.0);
    //     if (state < 8)
    //       motionControl.setPositionRivetage(ladder.rivetage[state / 8] / 100.0);
    //     state++;
    //   }
    //   if (s == 2) {
    //     if (state > 8) {
    //       state -= 6;
    //       showConfig();
    //       //motionControl.setPositionRivetage(ladder.rivetage[state / 8] / 100.0);
    //       motionControl.setPositionEvasage(ladder.evasage[state / 8] / 100.0);
    //     } else {
    //       state = 1;
    //       display.render();
    //     }
    //   }
    //   return;
    case 5:
    //   if (s == 1) {  // go to deplacement evasage
    //     motionControl.setPositionRivetage(ladder.rivetage[state / 8] / 100.0);
    //     //motionControl.setPositionEvasage(ladder.evasage[state / 8] / 100.0);
    //     state++;
    //   }
    //   if (s == 2) {
    //     state -= 2;
    //     showConfig();
    //     //motionControl.setPositionRivetage(ladder.rivetage[state / 8] / 100.0);
    //     motionControl.setPositionEvasage(ladder.evasage[state / 8] / 100.0);
    //   }
    //   return;
    case 4:
    case 6:
        // if (s > 1)
        // {
        //     state--;
        // }
        // return;
    case 7:
        // if (s == 3)
        // {
        //     if (motionControl.init())
        //     {
        //         digitalWrite(RED_LED_PIN, LOW);
        //         digitalWrite(GREEN_LED_PIN, HIGH);
        //         state = 1;
        //         display.render();
        //     }
        // }
         return;
    }
}
