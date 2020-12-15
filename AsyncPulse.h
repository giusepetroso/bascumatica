/*
  AsyncPulse.h - Library for getting a pulse with custom period.
  Created by Giuse Petroso, 2020.
  Released into the public domain.
*/
#ifndef AsyncPulse_h
#define AsyncPulse_h

#include "Arduino.h"

class AsyncPulse
{
public:
  AsyncPulse(unsigned long timeOn, unsigned long timeOff);
  int get();

private:
  int _state;
  unsigned long _timeOn, _timeOff, _countOn, _countOff;
};

#endif