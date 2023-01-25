//
// mobiflight.cpp
//
// (C) MobiFlight Project 2022
//

#include <Arduino.h>
#include "mobiflight.h"
#include "Button.h"
#include "Encoder.h"
#include "MFEEPROM.h"
#include "ArduinoUniqueID.h"
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
#if MF_OUTPUT_SHIFTER_SUPPORT == 1
#include "OutputShifter.h"
#endif
#if MF_DIGIN_MUX_SUPPORT == 1
#include "DigInMux.h"
#endif

#include "TFT.h"
#include "BouncingCircles.h"

#define MF_BUTTON_DEBOUNCE_MS     10 // time between updating the buttons
#define MF_ENCODER_DEBOUNCE_MS    1  // time between encoder updates
#define MF_INSHIFTER_POLL_MS      10 // time between input shift reg updates
#define MF_INMUX_POLL_MS          10 // time between dig input mux updates
#define MF_SERVO_DELAY_MS         5  // time between servo updates
#define MF_ANALOGAVERAGE_DELAY_MS 10 // time between updating the analog average calculation
#define MF_ANALOGREAD_DELAY_MS    50 // time between sending analog values

bool                powerSavingMode   = false;
const unsigned long POWER_SAVING_TIME = 60 * 15; // in seconds
uint32_t            demoMillis;

#if MF_MUX_SUPPORT == 1
MFMuxDriver MUX;
#endif
// ==================================================
//   Polling interval counters
// ==================================================

typedef struct {
    uint32_t Buttons  = 0;
    uint32_t Encoders = 0;
#if MF_SERVO_SUPPORT == 1
    uint32_t Servos = 0;
#endif
#if MF_ANALOG_SUPPORT == 1
    uint32_t AnalogAverage = 0;
    uint32_t Analog        = 0;
#endif
#if MF_INPUT_SHIFTER_SUPPORT == 1
    uint32_t InputShifters = 0;
#endif
#if MF_DIGIN_MUX_SUPPORT == 1
    uint32_t DigInMux = 0;
#endif
} lastUpdate_t;

lastUpdate_t lastUpdate;

extern MFEEPROM MFeeprom;

void initPollIntervals(void)
{
    // Init Time Gap between Inputs, do not read at the same loop
    lastUpdate.Buttons  = millis();
    lastUpdate.Encoders = millis();
#if MF_SERVO_SUPPORT == 1
    lastUpdate.Servos = millis() + 2;
#endif
#if MF_ANALOG_SUPPORT == 1
    lastUpdate.AnalogAverage = millis() + 4;
    lastUpdate.Analog        = millis() + 4;
#endif
#if MF_INPUT_SHIFTER_SUPPORT == 1
    lastUpdate.InputShifters = millis() + 6;
#endif
#if MF_DIGIN_MUX_SUPPORT == 1
    lastUpdate.DigInMux = millis() + 8;
#endif
}

void timedUpdate(void (*fun)(), uint32_t *last, uint8_t intv)
{
    if (millis() - *last >= intv) {
        fun();
        *last = millis();
    }
}

// ************************************************************
// Power saving
// ************************************************************
void SetPowerSavingMode(bool state)
{
    // disable the lights ;)
    powerSavingMode = state;
    Output::PowerSave(state);
#if MF_SEGMENT_SUPPORT == 1
    LedSegment::PowerSave(state);
#endif
#if MF_STEPPER_SUPPORT == 1
    Stepper::PowerSave(state);
#endif

#ifdef DEBUG2CMDMESSENGER
    if (state)
        cmdMessenger.sendCmd(kDebug, F("On"));
    else
        cmdMessenger.sendCmd(kDebug, F("Off"));
#endif
}

void updatePowerSaving()
{
    if (!powerSavingMode && ((millis() - getLastCommandMillis()) > (POWER_SAVING_TIME * 1000))) {
        // enable power saving
        SetPowerSavingMode(true);
    } else if (powerSavingMode && ((millis() - getLastCommandMillis()) < (POWER_SAVING_TIME * 1000))) {
        // disable power saving
        SetPowerSavingMode(false);
    }
}

// ************************************************************
// Reset Board
// ************************************************************
void ResetBoard()
{
    generateSerial(false);
    setLastCommandMillis();
    restoreName();
    loadConfig();
}

// ************************************************************
// Setup
// ************************************************************
void setup()
{
    Serial.println("Core 0 ready to go");
    MFeeprom.init();
    attachCommandCallbacks();
    cmdMessenger.printLfCr();
    ResetBoard();
    initPollIntervals();
}

// ************************************************************
// Loop function
// ************************************************************
void loop()
{
    // Process incoming serial data, and perform callbacks
    cmdMessenger.feedinSerialData();
    updatePowerSaving();

    // if config has been reset and still is not activated
    // do not perform updates
    // to prevent mangling input for config (shared buffers)
    if (getStatusConfig()) {

        timedUpdate(Button::read, &lastUpdate.Buttons, MF_BUTTON_DEBOUNCE_MS);

        timedUpdate(Encoder::read, &lastUpdate.Encoders, MF_ENCODER_DEBOUNCE_MS);

#if MF_STEPPER_SUPPORT == 1
        Stepper::update();
#endif

#if MF_SERVO_SUPPORT == 1
        timedUpdate(Servos::update, &lastUpdate.Servos, MF_SERVO_DELAY_MS);
#endif

#if MF_ANALOG_SUPPORT == 1
        timedUpdate(Analog::read, &lastUpdate.Analog, MF_ANALOGREAD_DELAY_MS);
        timedUpdate(Analog::readAverage, &lastUpdate.AnalogAverage, MF_ANALOGAVERAGE_DELAY_MS);
#endif

#if MF_INPUT_SHIFTER_SUPPORT == 1
        timedUpdate(InputShifter::read, &lastUpdate.InputShifters, MF_INSHIFTER_POLL_MS);
#endif

#if MF_DIGIN_MUX_SUPPORT == 1
        timedUpdate(DigInMux::read, &lastUpdate.DigInMux, MF_INMUX_POLL_MS);
#endif
        // lcds, outputs, outputshifters, segments do not need update
    }
}

// Initialize the graphics on the second core.
void setup1()
{
    TFT::init();
    BouncingCircles::initRandom(); // random seems not to work on core1
}

// Loop on the second core.
void loop1()
{
    demoMillis = millis();
    BouncingCircles::init();
    do {
        BouncingCircles::loop();
    } while (millis() - demoMillis < 5000);
    BouncingCircles::stop();
}
