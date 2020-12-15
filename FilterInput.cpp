/*
  FilterInput.cpp - Library for filtering inputs.
  Created by Giuse Petroso, 2020.
  Released into the public domain.
*/
#include "Arduino.h"
#include "FilterInput.h"

FilterInput::FilterInput(unsigned long filterOn, unsigned long filterOff, bool debug)
{
  _filterOn = filterOn;   //milliseconds should pass for setting filter
  _filterOff = filterOff; //milliseconds should pass for unsetting filter
  _inputState = 0;
  _lastInput = 0;
  _filterState = 0;
  _lastFilter = 0;
  _filterOnTime = 0;
  _filterOffTime = 0;
  _debug = debug;
}

int FilterInput::watch(int input)
{
  // read the digital input
  _inputState = input;

  // watch the input and manage the filter state
  _watchInput();

  // print debug stuff
  if (_debug)
    _debugLogs();

  // return the filter state
  return _filterState;
}

void FilterInput::_debugLogs()
{
  Serial.print("INPUT: ");
  Serial.print(_inputState);
  Serial.print(" | FON: ");
  Serial.print(millis() - _filterOnTime);
  Serial.print(" / ");
  Serial.print(_filterOn);
  Serial.print(" | FOFF: ");
  Serial.print(millis() - _filterOffTime);
  Serial.print(" / ");
  Serial.print(_filterOff);
  Serial.print(" | F: ");
  Serial.print(_filterState);
  // Serial.print(" | TON: ");
  // Serial.print(millis() - _delayOnTime);
  // Serial.print(" / ");
  // Serial.print(_delayOn);
  // Serial.print(" | TOFF: ");
  // Serial.print(millis() - _delayOffTime);
  // Serial.print(" / ");
  // Serial.print(_delayOff);
  // Serial.print(" | T: ");
  // Serial.print(_delayState);
  Serial.println("");
}

void FilterInput::_watchInput()
{
  // input rising edge
  bool risingEdge = _inputState == 1 && _lastInput == 0;
  if (risingEdge)
    _filterOnTime = millis();

  // input falling edge
  bool fallingEdge = _inputState == 0 && _lastInput == 1;
  if (fallingEdge)
    _filterOffTime = millis();

  // set input state
  _lastInput = _inputState;

  // filter state management
  if (_inputState == 1 && !risingEdge)
  {
    if (millis() - _filterOnTime >= _filterOn)
      _filterState = 1;
  }

  if (_inputState == 0 && !fallingEdge)
  {
    if (millis() - _filterOffTime >= _filterOff)
      _filterState = 0;
  }
}