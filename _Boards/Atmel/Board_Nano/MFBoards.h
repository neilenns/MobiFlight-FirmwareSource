//
// MFBoards.h (Arduino Uno/Nano)
//
// (C) MobiFlight Project 2022
//

#ifndef MFBoardUno_h
#define MFBoardUno_h

#ifndef MF_SEGMENT_SUPPORT
#define MF_SEGMENT_SUPPORT 1
#endif
#ifndef MF_LCD_SUPPORT
#define MF_LCD_SUPPORT 1
#endif
#ifndef MF_STEPPER_SUPPORT
#define MF_STEPPER_SUPPORT 1
#endif
#ifndef MF_SERVO_SUPPORT
#define MF_SERVO_SUPPORT 1
#endif
#ifndef MF_ANALOG_SUPPORT
#define MF_ANALOG_SUPPORT 1
#endif
#ifndef MF_OUTPUT_SHIFTER_SUPPORT
#define MF_OUTPUT_SHIFTER_SUPPORT 1
#endif
#ifndef MF_INPUT_SHIFTER_SUPPORT
#define MF_INPUT_SHIFTER_SUPPORT 1
#endif
#ifndef MF_MUX_SUPPORT
#define MF_MUX_SUPPORT 1
#endif
#ifndef MF_DIGIN_MUX_SUPPORT
#define MF_MUX_SUPPORT       1
#define MF_DIGIN_MUX_SUPPORT 1
#endif

#define MAX_OUTPUTS         21
#define MAX_BUTTONS         21
#define MAX_LEDSEGMENTS     1
#define MAX_ENCODERS        9
#define MAX_STEPPERS        2
#define MAX_MFSERVOS        2
#define MAX_MFLCD_I2C       2
#define MAX_ANALOG_INPUTS   6
#define MAX_OUTPUT_SHIFTERS 2
#define MAX_INPUT_SHIFTERS  2
#define MAX_DIGIN_MUX       3

#define STEPS         64
#define STEPPER_SPEED 400 // 300 already worked, 467, too?
#define STEPPER_ACCEL 800

#define MOBIFLIGHT_TYPE     "MobiFlight Nano"
#define MOBIFLIGHT_SERIAL   "0987654321"
#define MOBIFLIGHT_NAME     "MobiFlight Nano"
#define EEPROM_SIZE         1024 // EEPROMSizeUno
#define MEMLEN_CONFIG       286  // max. size for config which wil be stored in EEPROM
#define MEMLEN_NAMES_BUFFER 220  // max. size for configBuffer, contains only names from inputs
#define MF_MAX_DEVICEMEM    300  // max. memory size for devices

#define RANDOM_SEED_INPUT A0

#endif

// MFBoards.h