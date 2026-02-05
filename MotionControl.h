#include "pins_arduino.h"
#include <Bounce2.h>
#include <stdint.h>
#include "Arduino.h"
#include <AccelStepper.h>
#include "Display.h"

extern uint8_t state;
extern Bounce2::Button limitSwitch;

#define LIMIT_SWITCH_PIN 6

#define SETP_1_PIN 3
#define DIR_1_PIN 2

#define SETP_2_PIN 5
#define DIR_2_PIN 4

class MotionControl
{
private:
  AccelStepper a1;
  AccelStepper a2;
  void setAllMotor(uint8_t state)
  {
    digitalWrite(A0, state);
    digitalWrite(A1, state);
    digitalWrite(A2, state);
    digitalWrite(A3, state);
  }
  bool homeMotor(AccelStepper *a, uint8_t motor);

public:
  MotionControl();
  void setPositionEvasage(float x);
  void setPositionRivetage(float x);
  float positionEvasage();
  float positionRivetage();
  bool runEvasage();
  bool runRivetage();
  bool init();
  // void activate();
  void deactivate();
};
