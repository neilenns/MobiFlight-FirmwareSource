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
#ifndef MF_SHIFTER_SUPPORT
#define MF_SHIFTER_SUPPORT 1
#endif

#define MODULE_MAX_PINS 13
#define MAX_OUTPUTS 8
#define MAX_BUTTONS 8
#define MAX_LEDSEGMENTS 1
#define MAX_ENCODERS 2
#define MAX_STEPPERS 2
#define MAX_MFSERVOS 2
#define MAX_MFLCD_I2C 2
#define MAX_ANALOG_INPUTS 2
#define MAX_SHIFTERS 4

#define STEPS 64
#define STEPPER_SPEED 400 // 300 already worked, 467, too?
#define STEPPER_ACCEL 800

#define MOBIFLIGHT_TYPE "MobiFlight Nano"
#define MOBIFLIGHT_SERIAL "0987654321"
#define MOBIFLIGHT_NAME "MobiFlight Nano"
#define EEPROM_SIZE 1024 // EEPROMSizeUno
#define MEMLEN_CONFIG 256

#endif