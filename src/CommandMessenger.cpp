//
// commandmessenger.cpp
//
// (C) MobiFlight Project 2022
//

#include "mobiflight.h"

#include "Button.h"
#include "Encoder.h"
#if MF_ANALOG_SUPPORT == 1
#include "Analog.h"
#endif
#if MF_INPUT_SHIFTER_SUPPORT == 1
#include "InputShifter.h"
#endif
#include "Output.h"
#if MF_SEGMENT_SUPPORT == 1
#include "LedSegment.h"
#endif
#if MF_STEPPER_SUPPORT == 1
#include "Stepper.h"
#endif
#if MF_SERVO_SUPPORT == 1
#include "Servos.h"
#endif
#if MF_LCD_SUPPORT == 1
#include "LCDDisplay.h"
#endif
#if MF_OUTPUT_SHIFTER_SUPPORT == 1
#include "OutputShifter.h"
#endif
#if MF_DIGIN_MUX_SUPPORT == 1
#include "DigInMux.h"
#endif

CmdMessenger  cmdMessenger = CmdMessenger(Serial);
unsigned long lastCommand;

void OnTrigger();
void OnUnknownCommand();

// Callbacks define on which received commands we take action
void attachCommandCallbacks()
{
    // Attach callback methods
    cmdMessenger.attach(OnUnknownCommand);

#if MF_SEGMENT_SUPPORT == 1
    cmdMessenger.attach(kInitModule, LedSegment::OnInitModule);
    cmdMessenger.attach(kSetModule, LedSegment::OnSetModule);
    cmdMessenger.attach(kSetModuleBrightness, LedSegment::OnSetModuleBrightness);
#endif

    cmdMessenger.attach(kSetPin, Output::OnSet);

#if MF_STEPPER_SUPPORT == 1
    cmdMessenger.attach(kSetStepper, Stepper::OnSet);
    cmdMessenger.attach(kResetStepper, Stepper::OnReset);
    cmdMessenger.attach(kSetZeroStepper, Stepper::OnSetZero);
    cmdMessenger.attach(kSetStepperSpeedAccel, Stepper::OnSetSpeedAccel);
#endif

#if MF_SERVO_SUPPORT == 1
    cmdMessenger.attach(kSetServo, Servos::OnSet);
#endif

    cmdMessenger.attach(kGetInfo, OnGetInfo);
    cmdMessenger.attach(kGetConfig, OnGetConfig);
    cmdMessenger.attach(kSetConfig, OnSetConfig);
    cmdMessenger.attach(kResetConfig, OnResetConfig);
    cmdMessenger.attach(kSaveConfig, OnSaveConfig);
    cmdMessenger.attach(kActivateConfig, OnActivateConfig);
    cmdMessenger.attach(kSetName, OnSetName);
    cmdMessenger.attach(kGenNewSerial, OnGenNewSerial);
    cmdMessenger.attach(kTrigger, OnTrigger);

#if MF_LCD_SUPPORT == 1
    cmdMessenger.attach(kSetLcdDisplayI2C, LCDDisplay::OnSet);
#endif

#if MF_OUTPUT_SHIFTER_SUPPORT == 1
    cmdMessenger.attach(kSetShiftRegisterPins, OutputShifter::OnSet);
#endif

#ifdef DEBUG2CMDMESSENGER
    cmdMessenger.sendCmd(kDebug, F("Attached callbacks"));
#endif
}

// Called when a received command has no attached function
void OnUnknownCommand()
{
    lastCommand = millis();
    cmdMessenger.sendCmd(kDebug, F("n/a"));
}

uint32_t getLastCommandMillis()
{
    return lastCommand;
}

void setLastCommandMillis()
{
    lastCommand = millis();
}

void OnTrigger()
{
    Button::OnTrigger();
#if MF_INPUT_SHIFTER_SUPPORT == 1
    InputShifter::OnTrigger();
#endif
#if MF_DIGIN_MUX_SUPPORT == 1
    DigInMux::OnTrigger();
#endif
#if MF_ANALOG_SUPPORT == 1
    Analog::OnTrigger();
#endif
}

// commandmessenger.cpp
