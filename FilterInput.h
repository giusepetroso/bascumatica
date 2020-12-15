/*
  FilterInput.h - Library for filtering and delaying digital inputs.
  Created by Giuse Petroso, 2020.
  Released into the public domain.
*/
#ifndef FilterInput_h
#define FilterInput_h

class FilterInput {
    public: 
      FilterInput(int pin, int filterOn, int filterOff, int delayOn, int delayOff);
      bool watch();
    private:
      int _pin, _filterOn, _filterOff, _delayOn, _delayOff, _lastState, _filterOnTime, _filterOffTime, _delayOnTime, _delayOffTime;
};

#endif
