// MFButton.cpp
//
// Copyright (C) 2013-2014

#include "MFButton.h"

buttonEvent   MFButton::_handler = NULL;

MFButton::MFButton(uint8_t pin, const char * name)
{   
  _pin  = pin;
  _name = name;
  _state = 1;
  pinMode(_pin, INPUT_PULLUP);     // set pin to input
}

void MFButton::update()
{
    uint8_t newState = (uint8_t) digitalRead(_pin);
    if (newState!=_state) {     
      _state = newState;
      trigger(_state);      
    }
}

void MFButton::trigger(uint8_t state)
{
    (state==LOW) ? triggerOnPress() : triggerOnRelease();
}

void MFButton::triggerOnPress()
{
    if (_handler && _state==LOW) {
        (*_handler)(btnOnPress, _pin, _name);
    }
}

void MFButton::triggerOnRelease()
{
    if (_handler && _state==HIGH) {
        (*_handler)(btnOnRelease, _pin, _name);
    }
}

void MFButton::attachHandler(buttonEvent newHandler)
{
  _handler = newHandler;
}