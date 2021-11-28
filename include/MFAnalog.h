// MFSegments.h
//
/// \mainpage MF MFAnalog module for MobiFlight Framework
/// \par Revision History
/// \version 1.0 Initial release
/// \author  Manfred Berry (manfred@nystedberry.info) DO NOT CONTACT THE AUTHOR DIRECTLY: USE THE LISTS
// Copyright (C) 2021 Manfred Berry

#ifndef MFAnalog_h
#define MFAnalog_h

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#define ADC_MAX_AVERAGE                 8           // must be 2^n
#define ADC_MAX_AVERAGE_LOG2            3           // please calculate LOG2(ADC_MAX_AVERAGE)

extern "C"
{
  // callback functions
  typedef void (*analogEvent) (int, uint8_t, const char *);
};


/////////////////////////////////////////////////////////////////////
/// \class MFAnalog MFAnalog.h <MFAnalog.h>
class MFAnalog
{
public:
    MFAnalog(uint8_t pin = 1, const char * name = "Analog Input", uint8_t sensitivity = 2);
    static void attachHandler(analogEvent handler);    
    void update();
    void readBuffer();   
    const char *  _name;
    uint8_t       _pin;
    
private:
    static analogEvent   _handler; 
    int          _lastValue;
    uint8_t       _sensitivity;

    uint16_t ADC_Buffer[ADC_MAX_AVERAGE] = {0};     // Buffer for all values from each channel  
    uint16_t ADC_Average_Total = 0;                 // sum of sampled values, must be divided by ADC_MAX_AVERAGE to get actual value
    volatile uint8_t ADC_Average_Pointer = 0;       // points to the actual position in ADC_BUFFER
    uint32_t      _lastReadBuffer;
};
#endif 