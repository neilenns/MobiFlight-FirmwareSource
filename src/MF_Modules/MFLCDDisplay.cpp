// MFSegments.cpp
//
// Copyright (C) 2013-2014

#include "MFLCDDisplay.h"

MFLCDDisplay::MFLCDDisplay()
{
  _initialized = false;
}

void MFLCDDisplay::display(const char *string)
{
  if (!_initialized)
    return;
  for (uint8_t line = 0; line != _lines; line++)
  {
    _lcdDisplay->setCursor(0, line);
    for (uint8_t col = 0; col < _cols; col++)
    {
      uint8_t char2print = string[(_cols * line) + col];
      if (!char2print)                                     // just to be sure not to print after NULL termination
        return;
      _lcdDisplay->write(char2print);
    }
  }
}

void MFLCDDisplay::attach(byte address, byte cols, byte lines)
{
  _address = address;
  _cols = cols;
  _lines = lines;
  _lcdDisplay = new LiquidCrystal_I2C((uint8_t)address, (uint8_t)cols, (uint8_t)lines);
  _initialized = true;
  _lcdDisplay->init();
  _lcdDisplay->backlight();
  test();
}

void MFLCDDisplay::detach()
{
  if (!_initialized)
    return;
  delete _lcdDisplay;
  _initialized = false;
}

void MFLCDDisplay::powerSavingMode(bool state)
{
  if (state)
    _lcdDisplay->noBacklight();
  else
    _lcdDisplay->backlight();
}

void MFLCDDisplay::test()
{
  if (!_initialized)
    return;
  uint8_t preLines = 0;
  _lcdDisplay->clear();

  if (_lines > 2)
  {
    preLines = floor(_lines / 2) - 1;
  }

  _printCentered("MobiFlight", preLines++);
  if (_lines > 1)
  {
    _printCentered("Rocks!", preLines++);
  }

  _lcdDisplay->setCursor(0, 0);
}

void MFLCDDisplay::_printCentered(const char *str, uint8_t line)
{
  _lcdDisplay->setCursor(0, line);
  for (byte c = 0; c != ((_cols - strlen(str)) / 2); c++)
  {
    _lcdDisplay->print(F(" "));
  }
  _lcdDisplay->print(str);
  for (byte c = 0; c != ((_cols - strlen(str)) / 2); c++)
  {
    _lcdDisplay->print(F(" "));
  }
}