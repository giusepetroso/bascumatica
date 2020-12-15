/*
  AsyncPulse.h - Library for getting a pulse with custom period.
  Created by Giuse Petroso, 2020.
  Released into the public domain.
*/
#include "Arduino.h"
#include "AsyncPulse.h"

AsyncPulse::AsyncPulse(unsigned long timeOn, unsigned long timeOff)
{
  _state = 1;
  _timeOn = timeOn;
  _timeOff = timeOff;
  _countOn = 0;
  _countOff = 0;
}

int AsyncPulse::get()
{
  unsigned long millisecs = millis();

  if (_state == 1)
  {
    if (millisecs - _countOn >= _timeOn)
    {
      _state = 0;
      _countOff = millisecs;
    }
  }

  if (_state == 0)
  {
    if (millisecs - _countOff >= _timeOff)
    {
      _state = 1;
      _countOn = millisecs;
    }
  }

  // Serial.print("STATE: ");
  // Serial.print(_state);
  // Serial.print(" | TON: ");
  // Serial.print(millisecs - _countOn);
  // Serial.print(" / ");
  // Serial.print(_timeOn);
  // Serial.print(" | TOFF: ");
  // Serial.print(millisecs - _countOff);
  // Serial.print(" / ");
  // Serial.print(_timeOff);
  // Serial.println("");

  // return the state
  return _state;
}