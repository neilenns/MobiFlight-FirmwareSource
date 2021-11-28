/**
 * Includes Core Arduino functionality 
 **/
char foo;
#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include "mobiflight.h"
#include <MFBoards.h>

// The build version comes from an environment variable
#define STRINGIZER(arg) #arg
#define STR_VALUE(arg) STRINGIZER(arg)
#define VERSION STR_VALUE(BUILD_VERSION)

//#define DEBUG 1

#include "MFEEPROM.h"
#include <CmdMessenger.h>

#if MF_SEGMENT_SUPPORT == 1
#include <MFSegments.h>
#endif

#include <MFButton.h>
#include <MFEncoder.h>

#if MF_STEPPER_SUPPORT == 1
#include <AccelStepper.h>
#include <MFStepper.h>
#endif

#if MF_SERVO_SUPPORT == 1
#include <Servo.h>
#include <MFServo.h>
#endif

#include <MFOutput.h>

#if MF_LCD_SUPPORT == 1
#include <LiquidCrystal_I2C.h>
#include <MFLCDDisplay.h>
#endif

#if MF_ANALOG_SUPPORT == 1
#include <MFAnalog.h>
#endif

#if MF_SHIFTER_SUPPORT == 1
#include <MFShifter.h>
#endif

const uint8_t MEM_OFFSET_NAME = 0;
const uint8_t MEM_LEN_NAME = 48;
const uint8_t MEM_OFFSET_SERIAL = MEM_OFFSET_NAME + MEM_LEN_NAME;
const uint8_t MEM_LEN_SERIAL = 11;
const uint8_t MEM_OFFSET_CONFIG = MEM_OFFSET_NAME + MEM_LEN_NAME + MEM_LEN_SERIAL;

uint32_t lastAnalogAverage = 0;
uint32_t lastAnalogRead = 0;
uint32_t lastButtonUpdate= 0;
uint32_t lastEncoderUpdate = 0;

const char type[sizeof(MOBIFLIGHT_TYPE)] = MOBIFLIGHT_TYPE;
char serial[MEM_LEN_SERIAL] = MOBIFLIGHT_SERIAL;
char name[MEM_LEN_NAME] = MOBIFLIGHT_NAME;
const int MEM_LEN_CONFIG = MEMLEN_CONFIG;

char configBuffer[MEM_LEN_CONFIG] = "";

uint16_t configLength = 0;
boolean configActivated = false;

bool powerSavingMode = false;
uint8_t pinsRegistered[MODULE_MAX_PINS + 1];
const unsigned long POWER_SAVING_TIME = 60 * 15; // in seconds

CmdMessenger cmdMessenger = CmdMessenger(Serial);
unsigned long lastCommand;

MFEEPROM MFeeprom;

MFOutput outputs[MAX_OUTPUTS];
uint8_t outputsRegistered = 0;

MFButton buttons[MAX_BUTTONS];
uint8_t buttonsRegistered = 0;

#if MF_SEGMENT_SUPPORT == 1
MFSegments ledSegments[MAX_LEDSEGMENTS];
uint8_t ledSegmentsRegistered = 0;
#endif

MFEncoder encoders[MAX_ENCODERS];
uint8_t encodersRegistered = 0;

#if MF_STEPPER_SUPPORT == 1
MFStepper *steppers[MAX_STEPPERS]; //
uint8_t steppersRegistered = 0;
#endif

#if MF_SERVO_SUPPORT == 1
MFServo servos[MAX_MFSERVOS];
uint8_t servosRegistered = 0;
#endif

#if MF_LCD_SUPPORT == 1
MFLCDDisplay lcd_I2C[MAX_MFLCD_I2C];
uint8_t lcd_12cRegistered = 0;
#endif

#if MF_ANALOG_SUPPORT == 1
MFAnalog analog[MAX_ANALOG_INPUTS];
uint8_t analogRegistered = 0;
#endif

#if MF_SHIFTER_SUPPORT == 1
MFShifter shiftregisters[MAX_SHIFTERS];
uint8_t shiftregisterRegistered = 0;
#endif

// Callbacks define on which received commands we take action
void attachCommandCallbacks()
{
  // Attach callback methods
  cmdMessenger.attach(OnUnknownCommand);

#if MF_SEGMENT_SUPPORT == 1
  cmdMessenger.attach(kInitModule, OnInitModule);
  cmdMessenger.attach(kSetModule, OnSetModule);
  cmdMessenger.attach(kSetModuleBrightness, OnSetModuleBrightness);
#endif

  cmdMessenger.attach(kSetPin, OnSetPin);

#if MF_STEPPER_SUPPORT == 1
  cmdMessenger.attach(kSetStepper, OnSetStepper);
#endif

#if MF_SERVO_SUPPORT == 1
  cmdMessenger.attach(kSetServo, OnSetServo);
#endif

  cmdMessenger.attach(kGetInfo, OnGetInfo);
  cmdMessenger.attach(kGetConfig, OnGetConfig);
  cmdMessenger.attach(kSetConfig, OnSetConfig);
  cmdMessenger.attach(kResetConfig, OnResetConfig);
  cmdMessenger.attach(kSaveConfig, OnSaveConfig);
  cmdMessenger.attach(kActivateConfig, OnActivateConfig);
  cmdMessenger.attach(kSetName, OnSetName);
  cmdMessenger.attach(kGenNewSerial, OnGenNewSerial);

#if MF_STEPPER_SUPPORT == 1
  cmdMessenger.attach(kResetStepper, OnResetStepper);
  cmdMessenger.attach(kSetZeroStepper, OnSetZeroStepper);
#endif

  cmdMessenger.attach(kTrigger, OnTrigger);
  cmdMessenger.attach(kResetBoard, OnResetBoard);

#if MF_LCD_SUPPORT == 1
  cmdMessenger.attach(kSetLcdDisplayI2C, OnSetLcdDisplayI2C);
#endif

#if MF_SHIFTER_SUPPORT
  cmdMessenger.attach(kSetShiftRegisterPins, OnSetShiftRegisterPins);
#endif

#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Attached callbacks"));
#endif
}

// Callbacks that define what commands we issue upon internal events
void attachEventCallbacks()
{
  MFButton::attachHandler(handlerOnButton);
  MFEncoder::attachHandler(handlerOnEncoder);
#if MF_ANALOG_SUPPORT == 1
  MFAnalog::attachHandler(handlerOnAnalogChange);
#endif

}


void OnResetBoard()
{
  MFeeprom.init();
  configBuffer[0] = '\0';
  generateSerial(false);
  clearRegisteredPins();
  lastCommand = millis();
  loadConfig();
  _restoreName();
}

// Setup function
void setup()
{
  Serial.begin(115200);
  attachCommandCallbacks();
  attachEventCallbacks();
  cmdMessenger.printLfCr();
  OnResetBoard();
  // Time Gap between Inputs, do not read at the same loop
  lastAnalogAverage = millis() + 4;
  lastAnalogRead = millis() + 4;
  lastButtonUpdate= millis();
  lastEncoderUpdate = millis() +2;
}

void generateSerial(bool force)
{
  MFeeprom.read_block(MEM_OFFSET_SERIAL, serial, MEM_LEN_SERIAL);
  if (!force && serial[0] == 'S' && serial[1] == 'N')
    return;
  randomSeed(analogRead(0));
  sprintf(serial, "SN-%03x-", (unsigned int)random(4095));
  sprintf(&serial[7], "%03x", (unsigned int)random(4095));
  MFeeprom.write_block(MEM_OFFSET_SERIAL, serial, MEM_LEN_SERIAL);
  if (!force) MFeeprom.write_byte(MEM_OFFSET_CONFIG, 0x00);           // First byte of config to 0x00 to ensure to start 1st time with empty config, but not if forced from the connector to generate a new one
}

void loadConfig()
{
  resetConfig();
  MFeeprom.read_block(MEM_OFFSET_CONFIG, configBuffer, MEM_LEN_CONFIG);
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Restored config"));
  cmdMessenger.sendCmd(kStatus, configBuffer);
#endif
  configLength = strlen(configBuffer);
  readConfig();
  _activateConfig();
}

void _storeConfig()
{
  MFeeprom.write_block(MEM_OFFSET_CONFIG, configBuffer, MEM_LEN_CONFIG);
}

void SetPowerSavingMode(bool state)
{
  // disable the lights ;)
  powerSavingMode = state;

#if MF_SEGMENT_SUPPORT == 1
  PowerSaveLedSegment(state);
#endif

#ifdef DEBUG
  if (state)
    cmdMessenger.sendCmd(kStatus, F("On"));
  else
    cmdMessenger.sendCmd(kStatus, F("Off"));
#endif
  //PowerSaveOutputs(state);
}

void updatePowerSaving()
{
  if (!powerSavingMode && ((millis() - lastCommand) > (POWER_SAVING_TIME * 1000)))
  {
    // enable power saving
    SetPowerSavingMode(true);
  }
  else if (powerSavingMode && ((millis() - lastCommand) < (POWER_SAVING_TIME * 1000)))
  {
    // disable power saving
    SetPowerSavingMode(false);
  }
}

// Loop function
void loop()
{
  // Process incoming serial data, and perform callbacks
  cmdMessenger.feedinSerialData();
  updatePowerSaving();

  // if config has been reset
  // and still is not activated
  // do not perform updates
  // to prevent mangling input for config (shared buffers)
  if (!configActivated)
    return;

  readButtons();
  readEncoder();
#if MF_ANALOG_SUPPORT == 1
  readAnalog();
#endif

  // segments do not need update
#if MF_STEPPER_SUPPORT == 1
  updateSteppers();
#endif

#if MF_SERVO_SUPPORT == 1
  updateServos();
#endif
}

bool isPinRegistered(uint8_t pin)
{
  return pinsRegistered[pin] != kTypeNotSet;
}

bool isPinRegisteredForType(uint8_t pin, uint8_t type)
{
  return pinsRegistered[pin] == type;
}

void registerPin(uint8_t pin, uint8_t type)
{
  pinsRegistered[pin] = type;
}

void clearRegisteredPins(uint8_t type)
{
  for (int i = 0; i != MODULE_MAX_PINS + 1; ++i)
    if (pinsRegistered[i] == type)
      pinsRegistered[i] = kTypeNotSet;
}

void clearRegisteredPins()
{
  for (int i = 0; i != MODULE_MAX_PINS + 1; ++i)
    pinsRegistered[i] = kTypeNotSet;
}

//// OUTPUT /////
void AddOutput(uint8_t pin = 1, char const *name = "Output")
{
  if (outputsRegistered == MAX_OUTPUTS)
    return;
  if (isPinRegistered(pin))
    return;

  outputs[outputsRegistered] = MFOutput(pin);
  registerPin(pin, kTypeOutput);
  outputsRegistered++;
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Added output"));
#endif
}

void ClearOutputs()
{
  clearRegisteredPins(kTypeOutput);
  outputsRegistered = 0;
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Cleared outputs"));
#endif
}

//// BUTTONS /////
void AddButton(uint8_t pin = 1, char const *name = "Button")
{
  if (buttonsRegistered == MAX_BUTTONS)
    return;

  if (isPinRegistered(pin))
    return;

  buttons[buttonsRegistered] = MFButton(pin, name);
  
  registerPin(pin, kTypeButton);
  buttonsRegistered++;
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Added button ") /* + name */);
#endif
}

void ClearButtons()
{
  clearRegisteredPins(kTypeButton);
  buttonsRegistered = 0;
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Cleared buttons"));
#endif
}

//// ENCODERS /////
void AddEncoder(uint8_t pin1 = 1, uint8_t pin2 = 2, uint8_t encoder_type = 0, char const *name = "Encoder")
{
  if (encodersRegistered == MAX_ENCODERS)
    return;
  if (isPinRegistered(pin1) || isPinRegistered(pin2))
    return;

  encoders[encodersRegistered] = MFEncoder();
  encoders[encodersRegistered].attach(pin1, pin2, encoder_type, name);
  
  registerPin(pin1, kTypeEncoder);
  registerPin(pin2, kTypeEncoder);
  encodersRegistered++;
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Added encoder"));
#endif
}

void ClearEncoders()
{
  clearRegisteredPins(kTypeEncoder);
  encodersRegistered = 0;
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Cleared encoders"));
#endif
}

//// OUTPUTS /////

#if MF_SEGMENT_SUPPORT == 1
//// SEGMENTS /////
void AddLedSegment(int dataPin, int csPin, int clkPin, int numDevices, int brightness)
{
  if (ledSegmentsRegistered == MAX_LEDSEGMENTS)
    return;

  if (isPinRegistered(dataPin) || isPinRegistered(clkPin) || isPinRegistered(csPin))
    return;

  ledSegments[ledSegmentsRegistered].attach(dataPin, csPin, clkPin, numDevices, brightness); // lc is our object

  registerPin(dataPin, kTypeLedSegment);
  registerPin(csPin, kTypeLedSegment);
  registerPin(clkPin, kTypeLedSegment);
  ledSegmentsRegistered++;
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Added Led Segment"));
#endif
}

void ClearLedSegments()
{
  clearRegisteredPins(kTypeLedSegment);
  for (int i = 0; i != ledSegmentsRegistered; i++)
  {
    ledSegments[ledSegmentsRegistered].detach();
  }
  ledSegmentsRegistered = 0;
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Cleared segments"));
#endif
}

void PowerSaveLedSegment(bool state)
{
  for (int i = 0; i != ledSegmentsRegistered; ++i)
  {
    ledSegments[i].powerSavingMode(state);
  }

  for (int i = 0; i != outputsRegistered; ++i)
  {
    outputs[i].powerSavingMode(state);
  }
}
#endif

#if MF_STEPPER_SUPPORT == 1
//// STEPPER ////
void AddStepper(int pin1, int pin2, int pin3, int pin4, int btnPin1)
{
  if (steppersRegistered == MAX_STEPPERS)
    return;
  if (isPinRegistered(pin1) || isPinRegistered(pin2) || isPinRegistered(pin3) || isPinRegistered(pin4) || (btnPin1 > 0 && isPinRegistered(btnPin1)))
  {
#ifdef DEBUG
    cmdMessenger.sendCmd(kStatus, F("Conflict with stepper"));
#endif
    return;
  }

  steppers[steppersRegistered] = new MFStepper(pin1, pin2, pin3, pin4, btnPin1); // is our object
  steppers[steppersRegistered]->setMaxSpeed(STEPPER_SPEED);
  steppers[steppersRegistered]->setAcceleration(STEPPER_ACCEL);

  registerPin(pin1, kTypeStepper);
  registerPin(pin2, kTypeStepper);
  registerPin(pin3, kTypeStepper);
  registerPin(pin4, kTypeStepper);
  // autoreset is not released yet
  if (btnPin1 > 0)
  {
    registerPin(btnPin1, kTypeStepper);
    // this triggers the auto reset if we need to reset
    steppers[steppersRegistered]->reset();
  }

  // all set
  steppersRegistered++;

#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Added stepper"));
#endif
}

void ClearSteppers()
{
  for (int i = 0; i != steppersRegistered; i++)
  {
    delete steppers[steppersRegistered];
  }
  clearRegisteredPins(kTypeStepper);
  steppersRegistered = 0;
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Cleared steppers"));
#endif
}
#endif

#if MF_SERVO_SUPPORT == 1
//// SERVOS /////
void AddServo(int pin)
{
  if (servosRegistered == MAX_MFSERVOS)
    return;
  if (isPinRegistered(pin))
    return;

  servos[servosRegistered].attach(pin, true);
  registerPin(pin, kTypeServo);
  servosRegistered++;
}

void ClearServos()
{
  for (int i = 0; i != servosRegistered; i++)
  {
    servos[servosRegistered].detach();
  }
  clearRegisteredPins(kTypeServo);
  servosRegistered = 0;
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Cleared servos"));
#endif
}
#endif

#if MF_LCD_SUPPORT == 1
//// LCD Display /////
void AddLcdDisplay(uint8_t address = 0x24, uint8_t cols = 16, uint8_t lines = 2, char const *name = "LCD")
{
  if (lcd_12cRegistered == MAX_MFLCD_I2C)
    return;
  registerPin(SDA, kTypeLcdDisplayI2C);
  registerPin(SCL, kTypeLcdDisplayI2C);

  lcd_I2C[lcd_12cRegistered].attach(address, cols, lines);
  lcd_12cRegistered++;
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Added lcdDisplay"));
#endif
}

void ClearLcdDisplays()
{
  for (int i = 0; i != lcd_12cRegistered; i++)
  {
    lcd_I2C[lcd_12cRegistered].detach();
  }
  clearRegisteredPins(kTypeLcdDisplayI2C);
  lcd_12cRegistered = 0;
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Cleared lcdDisplays"));
#endif
}
#endif

#if MF_ANALOG_SUPPORT == 1

void AddAnalog(uint8_t pin = 1, char const *name = "AnalogInput", uint8_t sensitivity = 3)
{
  if (analogRegistered == MAX_ANALOG_INPUTS)
    return;

  if (isPinRegistered(pin))
    return;

  analog[analogRegistered] = MFAnalog(pin, name, sensitivity);
  registerPin(pin, kTypeAnalogInput);
  analogRegistered++;
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Added analog device "));
#endif
}

void ClearAnalog()
{
  clearRegisteredPins(kTypeAnalogInput);
  analogRegistered = 0;
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Cleared analog devices"));
#endif
}

#endif

#if MF_SHIFTER_SUPPORT == 1
//// SHIFT REGISTER /////
void AddShifter(uint8_t latchPin, uint8_t clockPin, uint8_t dataPin, uint8_t modules, char const *name = "Shifter")
{
  if (shiftregisterRegistered == MAX_SHIFTERS)
    return;
  shiftregisters[shiftregisterRegistered].attach(latchPin, clockPin, dataPin, modules);
  shiftregisters[shiftregisterRegistered].clear();
  shiftregisterRegistered++;

#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Added Shifter"));
#endif
}

void ClearShifters()
{
  for (int i = 0; i != shiftregisterRegistered; i++)
  {
    shiftregisters[shiftregisterRegistered].detach();
  }

  shiftregisterRegistered = 0;
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Cleared Shifter"));
#endif
}
#endif

//// EVENT HANDLER /////
void handlerOnButton(uint8_t eventId, uint8_t pin, const char *name)
{
  cmdMessenger.sendCmdStart(kButtonChange);
  cmdMessenger.sendCmdArg(name);
  cmdMessenger.sendCmdArg(eventId);
  cmdMessenger.sendCmdEnd();
};

//// EVENT HANDLER /////
void handlerOnEncoder(uint8_t eventId, uint8_t pin, const char *name)
{
  cmdMessenger.sendCmdStart(kEncoderChange);
  cmdMessenger.sendCmdArg(name);
  cmdMessenger.sendCmdArg(eventId);
  cmdMessenger.sendCmdEnd();
};

//// EVENT HANDLER /////
void handlerOnAnalogChange(int value, uint8_t pin, const char *name)
{
  cmdMessenger.sendCmdStart(kAnalogChange);
  cmdMessenger.sendCmdArg(name);
  cmdMessenger.sendCmdArg(value);
  cmdMessenger.sendCmdEnd();
};

/**
 ** config stuff
 **/
void OnSetConfig()
{
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Setting config start"));
#endif
  lastCommand = millis();
  char *cfg = cmdMessenger.readStringArg();
  uint8_t cfgLen = strlen(cfg);
  uint16_t bufferSize = MEM_LEN_CONFIG - (configLength + cfgLen);

  if (bufferSize > 1)
  {
    memcpy(&configBuffer[configLength], cfg, bufferSize);
    configLength += cfgLen;
    cmdMessenger.sendCmd(kStatus, configLength);
  }
  else
    cmdMessenger.sendCmd(kStatus, -1);
#ifdef DEBUG
  cmdMessenger.sendCmd(kStatus, F("Setting config end"));
#endif
}

void resetConfig()
{
  ClearButtons();
  ClearEncoders();
  ClearOutputs();

#if MF_SEGMENT_SUPPORT == 1
  ClearLedSegments();
#endif

#if MF_SERVO_SUPPORT == 1
  ClearServos();
#endif

#if MF_STEPPER_SUPPORT == 1
  ClearSteppers();
#endif

#if MF_LCD_SUPPORT == 1
  ClearLcdDisplays();
#endif

#if MF_ANALOG_SUPPORT == 1
  ClearAnalog();
#endif

#if MF_SHIFTER_SUPPORT == 1
  ClearShifters();
#endif

  configLength = 0;
  configActivated = false;
}

void OnResetConfig()
{
  resetConfig();
  cmdMessenger.sendCmd(kStatus, F("OK"));
}

void OnSaveConfig()
{
  _storeConfig();
  cmdMessenger.sendCmd(kConfigSaved, F("OK"));
}

void OnActivateConfig()
{
  readConfig();
  _activateConfig();
}

void _activateConfig()
{
  configActivated = true;
  cmdMessenger.sendCmd(kConfigActivated, F("OK"));
}

void readConfig()
{
  if (configLength == 0) return;
  char *p = NULL;

  char *command = strtok_r(configBuffer, ".", &p);
  char *params[6];
  if (*command == 0)
    return;

  do
  {
    switch (atoi(command))
    {
    case kTypeButton:
      params[0] = strtok_r(NULL, ".", &p); // pin
      params[1] = strtok_r(NULL, ":", &p); // name
      AddButton(atoi(params[0]), params[1]);
      break;

    case kTypeOutput:
      params[0] = strtok_r(NULL, ".", &p); // pin
      params[1] = strtok_r(NULL, ":", &p); // Name
      AddOutput(atoi(params[0]), params[1]);
      break;

    case kTypeLedSegment:
      params[0] = strtok_r(NULL, ".", &p); // pin Data
      params[1] = strtok_r(NULL, ".", &p); // pin Cs
      params[2] = strtok_r(NULL, ".", &p); // pin Clk
      params[3] = strtok_r(NULL, ".", &p); // brightness
      params[4] = strtok_r(NULL, ".", &p); // numModules
      params[5] = strtok_r(NULL, ":", &p); // Name
                                           // int dataPin, int clkPin, int csPin, int numDevices, int brightness
#if MF_SEGMENT_SUPPORT == 1
      AddLedSegment(atoi(params[0]), atoi(params[1]), atoi(params[2]), atoi(params[4]), atoi(params[3]));
#endif
      break;

    case kTypeStepperDeprecated:
      // this is for backwards compatibility
      params[0] = strtok_r(NULL, ".", &p); // pin1
      params[1] = strtok_r(NULL, ".", &p); // pin2
      params[2] = strtok_r(NULL, ".", &p); // pin3
      params[3] = strtok_r(NULL, ".", &p); // pin4
      params[4] = strtok_r(NULL, ".", &p); // btnPin1
      params[5] = strtok_r(NULL, ":", &p); // Name
#if MF_STEPPER_SUPPORT == 1
      AddStepper(atoi(params[0]), atoi(params[1]), atoi(params[2]), atoi(params[3]), 0);
#endif
      break;

    case kTypeStepper:
      // AddStepper(int pin1, int pin2, int pin3, int pin4)
      params[0] = strtok_r(NULL, ".", &p); // pin1
      params[1] = strtok_r(NULL, ".", &p); // pin2
      params[2] = strtok_r(NULL, ".", &p); // pin3
      params[3] = strtok_r(NULL, ".", &p); // pin4
      params[4] = strtok_r(NULL, ".", &p); // btnPin1
      params[5] = strtok_r(NULL, ":", &p); // Name
#if MF_STEPPER_SUPPORT == 1
      AddStepper(atoi(params[0]), atoi(params[1]), atoi(params[2]), atoi(params[3]), atoi(params[4]));
#endif
      break;

    case kTypeServo:
      // AddServo(int pin)
      params[0] = strtok_r(NULL, ".", &p); // pin1
      params[1] = strtok_r(NULL, ":", &p); // Name
#if MF_SERVO_SUPPORT == 1
      AddServo(atoi(params[0]));
#endif
      break;

    case kTypeEncoderSingleDetent:
      // AddEncoder(uint8_t pin1 = 1, uint8_t pin2 = 2, uint8_t encoder_type = 0, String name = "Encoder")
      params[0] = strtok_r(NULL, ".", &p); // pin1
      params[1] = strtok_r(NULL, ".", &p); // pin2
      params[2] = strtok_r(NULL, ":", &p); // Name
      AddEncoder(atoi(params[0]), atoi(params[1]), 0, params[2]);
      break;

    case kTypeEncoder:
      // AddEncoder(uint8_t pin1 = 1, uint8_t pin2 = 2, uint8_t encoder_type = 0, String name = "Encoder")
      params[0] = strtok_r(NULL, ".", &p); // pin1
      params[1] = strtok_r(NULL, ".", &p); // pin2
      params[2] = strtok_r(NULL, ".", &p); // encoder_type
      params[3] = strtok_r(NULL, ":", &p); // Name
      AddEncoder(atoi(params[0]), atoi(params[1]), atoi(params[2]), params[3]);
      break;

    case kTypeLcdDisplayI2C:
      // AddEncoder(uint8_t address = 0x24, uint8_t cols = 16, lines = 2, String name = "Lcd")
      params[0] = strtok_r(NULL, ".", &p); // address
      params[1] = strtok_r(NULL, ".", &p); // cols
      params[2] = strtok_r(NULL, ".", &p); // lines
      params[3] = strtok_r(NULL, ":", &p); // Name
#if MF_LCD_SUPPORT == 1
      AddLcdDisplay(atoi(params[0]), atoi(params[1]), atoi(params[2]), params[3]);
#endif
      break;

    case kTypeAnalogInput:
      params[0] = strtok_r(NULL, ".", &p); // pin
      params[1] = strtok_r(NULL, ".", &p); // sensitivity
      params[2] = strtok_r(NULL, ":", &p); // name
#if MF_ANALOG_SUPPORT == 1
      AddAnalog(atoi(params[0]), params[2], atoi(params[1]));
#endif
      break;

    case kShiftRegister:
      params[0] = strtok_r(NULL, ".", &p); // pin latch
      params[1] = strtok_r(NULL, ".", &p); // pin clock
      params[2] = strtok_r(NULL, ".", &p); // pin data
      params[3] = strtok_r(NULL, ".", &p); // number of daisy chained modules
      params[4] = strtok_r(NULL, ":", &p); // name
#if MF_SHIFTER_SUPPORT == 1
      AddShifter(atoi(params[0]), atoi(params[1]), atoi(params[2]), atoi(params[3]), params[4]);
#endif
      break;

    default:
      // read to the end of the current command which is
      // apparently not understood
      params[0] = strtok_r(NULL, ":", &p); // read to end of unknown command
    }
    command = strtok_r(NULL, ".", &p);
  } while (command != NULL);
}

// Called when a received command has no attached function
void OnUnknownCommand()
{
  lastCommand = millis();
  cmdMessenger.sendCmd(kStatus, F("n/a"));
}

void OnGetInfo()
{
  lastCommand = millis();
  cmdMessenger.sendCmdStart(kInfo);
  cmdMessenger.sendCmdArg(type);
  cmdMessenger.sendCmdArg(name);
  cmdMessenger.sendCmdArg(serial);
  cmdMessenger.sendCmdArg(VERSION);
  cmdMessenger.sendCmdEnd();
}

void OnGetConfig()
{
  lastCommand = millis();
  cmdMessenger.sendCmdStart(kInfo);
  cmdMessenger.sendCmdArg(MFeeprom.read_char(MEM_OFFSET_CONFIG));
  for (uint16_t i=1; i<configLength; i++) {
    cmdMessenger.sendArg(MFeeprom.read_char(MEM_OFFSET_CONFIG+i));
  }
  cmdMessenger.sendCmdEnd();
}

// Callback function that sets led on or off
void OnSetPin()
{
  // Read led state argument, interpret string as boolean
  int pin = cmdMessenger.readInt16Arg();
  int state = cmdMessenger.readInt16Arg();
  // Set led
  analogWrite(pin, state);
  lastCommand = millis();
}

#if MF_SEGMENT_SUPPORT == 1
void OnInitModule()
{
  int module = cmdMessenger.readInt16Arg();
  int subModule = cmdMessenger.readInt16Arg();
  int brightness = cmdMessenger.readInt16Arg();
  ledSegments[module].setBrightness(subModule, brightness);
  lastCommand = millis();
}

void OnSetModule()
{
  int module = cmdMessenger.readInt16Arg();
  int subModule = cmdMessenger.readInt16Arg();
  char *value = cmdMessenger.readStringArg();
  uint8_t points = (uint8_t)cmdMessenger.readInt16Arg();
  uint8_t mask = (uint8_t)cmdMessenger.readInt16Arg();
  ledSegments[module].display(subModule, value, points, mask);
  lastCommand = millis();
}

void OnSetModuleBrightness()
{
  int module = cmdMessenger.readInt16Arg();
  int subModule = cmdMessenger.readInt16Arg();
  int brightness = cmdMessenger.readInt16Arg();
  ledSegments[module].setBrightness(subModule, brightness);
  lastCommand = millis();
}

#endif

#if MF_SHIFTER_SUPPORT == 1

void OnInitShiftRegister()
{
  int module = cmdMessenger.readInt16Arg();
  shiftregisters[module].clear();
  lastCommand = millis();
}

void OnSetShiftRegisterPins()
{

  int module = cmdMessenger.readInt16Arg();
  char *pins = cmdMessenger.readStringArg();
  int value = cmdMessenger.readInt16Arg();
  shiftregisters[module].setPins(pins, value);
  lastCommand = millis();
}

#endif

#if MF_STEPPER_SUPPORT == 1
void OnSetStepper()
{
  int stepper = cmdMessenger.readInt16Arg();
  long newPos = cmdMessenger.readInt32Arg();

  if (stepper >= steppersRegistered)
    return;
  steppers[stepper]->moveTo(newPos);
  lastCommand = millis();
}

void OnResetStepper()
{
  int stepper = cmdMessenger.readInt16Arg();

  if (stepper >= steppersRegistered)
    return;
  steppers[stepper]->reset();
  lastCommand = millis();
}

void OnSetZeroStepper()
{
  int stepper = cmdMessenger.readInt16Arg();

  if (stepper >= steppersRegistered)
    return;
  steppers[stepper]->setZero();
  lastCommand = millis();
}

void updateSteppers()
{
  for (int i = 0; i != steppersRegistered; i++)
  {
    steppers[i]->update();
  }
}
#endif

#if MF_SERVO_SUPPORT == 1
void OnSetServo()
{
  int servo = cmdMessenger.readInt16Arg();
  int newValue = cmdMessenger.readInt16Arg();
  if (servo >= servosRegistered)
    return;
  servos[servo].moveTo(newValue);
  lastCommand = millis();
}

void updateServos()
{
  for (int i = 0; i != servosRegistered; i++)
  {
    servos[i].update();
  }
}
#endif

#if MF_LCD_SUPPORT == 1
void OnSetLcdDisplayI2C()
{
  int address = cmdMessenger.readInt16Arg();
  char *output = cmdMessenger.readStringArg();
  lcd_I2C[address].display(output);
  lastCommand = millis();
}
#endif

void readButtons()
{
  if (millis()-lastButtonUpdate <= MF_BUTTON_DEBOUNCE_MS) return;
  lastButtonUpdate= millis();
  for (int i = 0; i != buttonsRegistered; i++)
  {
    buttons[i].update();
  }
}

void readEncoder()
{
  if (millis()-lastEncoderUpdate < 1) return;
  lastEncoderUpdate = millis();
  for (int i = 0; i != encodersRegistered; i++)
  {
    encoders[i].update();
  }
}

#if MF_ANALOG_SUPPORT == 1
void readAnalog()
{
  if (millis()-lastAnalogAverage > 10) {
    for (int i = 0; i != analogRegistered; i++)
    {
      analog[i].readBuffer();
    }
    lastAnalogAverage = millis();
  }
  if (millis()-lastAnalogRead < 50) return;
  lastAnalogRead = millis();
  for (int i = 0; i != analogRegistered; i++)
  {
    analog[i].update();
  }
}
#endif

void OnGenNewSerial()
{
  generateSerial(true);
  cmdMessenger.sendCmdStart(kInfo);
  cmdMessenger.sendCmdArg(serial);
  cmdMessenger.sendCmdEnd();
}

void OnSetName()
{
  char *cfg = cmdMessenger.readStringArg();
  memcpy(name, cfg, MEM_LEN_NAME);
  _storeName();
  cmdMessenger.sendCmdStart(kStatus);
  cmdMessenger.sendCmdArg(name);
  cmdMessenger.sendCmdEnd();
}

void _storeName()
{
  char prefix[] = "#";
  MFeeprom.write_block(MEM_OFFSET_NAME, prefix, 1);
  MFeeprom.write_block(MEM_OFFSET_NAME + 1, name, MEM_LEN_NAME - 1);
}

void _restoreName()
{
  char testHasName[1] = "";
  MFeeprom.read_block(MEM_OFFSET_NAME, testHasName, 1);
  if (testHasName[0] != '#')
    return;

  MFeeprom.read_block(MEM_OFFSET_NAME + 1, name, MEM_LEN_NAME - 1);
}

void OnTrigger()
{
  // Trigger all button release events first...
  for (int i = 0; i != buttonsRegistered; i++)
  {
    buttons[i].triggerOnRelease();
  }
  // ... then trigger all the press events
  for (int i = 0; i != buttonsRegistered; i++)
  {
    buttons[i].triggerOnPress();
  }
}
