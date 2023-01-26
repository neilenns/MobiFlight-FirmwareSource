//
// LCDDisplay.cpp
//
// (C) MobiFlight Project 2022
//

#include "mobiflight.h"
#include "MFLCDDisplay.h"
#include "LCDDisplay.h"
#include "IPC.h"

volatile double values[8];

namespace LCDDisplay
{
    MFLCDDisplay *lcd_I2C[MAX_MFLCD_I2C];
    uint8_t       lcd_12cRegistered = 0;

    void Add(uint8_t address, uint8_t cols, uint8_t lines)
    {
        if (lcd_12cRegistered == MAX_MFLCD_I2C)
            return;

        if (!FitInMemory(sizeof(MFLCDDisplay))) {
            // Error Message to Connector
            cmdMessenger.sendCmd(kStatus, F("LCD does not fit in Memory!"));
            return;
        }
        lcd_I2C[lcd_12cRegistered] = new (allocateMemory(sizeof(MFLCDDisplay))) MFLCDDisplay;
        lcd_I2C[lcd_12cRegistered]->attach(address, cols, lines);
        lcd_12cRegistered++;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Added lcdDisplay"));
#endif
    }

    void Clear()
    {
        for (uint8_t i = 0; i < lcd_12cRegistered; i++) {
            lcd_I2C[i]->detach();
        }
        lcd_12cRegistered = 0;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Cleared lcdDisplays"));
#endif
    }

    void OnSet()
    {
        int   address = cmdMessenger.readInt16Arg();
        char *output  = cmdMessenger.readStringArg();
        cmdMessenger.unescape(output);

        // 25,0,124.850/130.855/124.850/124.850/110.50/113.15/110.50/113.90                                                                                                                                                                                     ;
        // There are 8 frequencies to check
        char  *frequencyValue = strtok(output, "|");
        double convertedValue;

        for (int i = 0; i < 8 && frequencyValue != 0; i++) {
            convertedValue = atof(frequencyValue);

            if (values[i] != convertedValue) {
                values[i] = convertedValue;
                rp2040.fifo.push((ChangedValue)i);
            }
            frequencyValue = strtok(0, "|");
        }
    }
} // namespace

// LCDDisplay.cpp
