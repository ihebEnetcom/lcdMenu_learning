#include "HardwareSerial.h"
#include "Arduino.h"
#include <math.h>
#include "MotionControl.h"

MotionControl::MotionControl()
    : a1(AccelStepper::DRIVER, SETP_1_PIN, DIR_1_PIN),
      a2(AccelStepper::DRIVER, SETP_2_PIN, DIR_2_PIN) {}

bool MotionControl::init()
{
  if (limitSwitch.isPressed())
  {
    return false;
  }
  display.write(F("Initialising"));
  display.write(F("\nMotor 1"), 0, false);
  if (!homeMotor(&a1, A0))
    return false;
  display.write(F("\nMotor 2"), 0, false);
  if (!homeMotor(&a1, A1))
    return false;
  display.write(F("\nMotor 3"), 0, false);
  if (!homeMotor(&a2, A2))
    return false;
  display.write(F("\nMotor 4"), 0, false);
  if (!homeMotor(&a2, A3))
    return false;
  a1.setMaxSpeed(6000.0);
  a1.setAcceleration(400.0);
  a2.setMaxSpeed(6000.0);
  a2.setAcceleration(400.0);
  setAllMotor(HIGH);
  delay(1000);
  return true;
}

bool MotionControl::homeMotor(AccelStepper *a, uint8_t motor)
{
  a->setMaxSpeed(4000.0);
  a->setAcceleration(400.0);
  a->move(-715.3 * 230.0);
  digitalWrite(motor, HIGH); // enable m
  while (!limitSwitch.isPressed())
  {
    limitSwitch.update();
    if (!a->run())
    {
      digitalWrite(motor, LOW);
      display.write(F("\n\nSoft Limit"), 2, false);
      return false;
    }
  }
  a->setAcceleration(5000.0);
  a->stop();
  a->runToPosition();
  a->move(715.3 * 4);
  a->setMaxSpeed(40);
  a->setAcceleration(400);
  while (limitSwitch.isPressed())
  {
    limitSwitch.update();
    if (!a->run())
    {
      digitalWrite(motor, LOW);
      display.write(F("\n\n\tError"), 2, false);
      return false;
    }
  }
  digitalWrite(motor, LOW); // disable motor
  a->setCurrentPosition(0);
  return true;
}

void MotionControl::setPositionEvasage(float x)
{
  
  a1.moveTo(lroundf(x * 715.3f));
}

void MotionControl::setPositionRivetage(float x)
{
  a2.moveTo(lroundf(x * 715.3f));
}

float MotionControl::positionEvasage()
{
  return a1.currentPosition() / 715.3;
}

float MotionControl::positionRivetage()
{
  return a2.currentPosition() / 715.3;
}

bool MotionControl::runEvasage()
{
  if (limitSwitch.isPressed())
    return false;
  return a1.run();
}

bool MotionControl::runRivetage()
{
  if (limitSwitch.isPressed())
    return false;
  return a2.run();
}

/*void MotionControl::activate() {
  setAllMotor(HIGH);
}*/

void MotionControl::deactivate()
{
  setAllMotor(LOW);
}
