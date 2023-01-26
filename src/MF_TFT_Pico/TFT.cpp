#include <Arduino.h>
#include "TFT.h"
#include "IPC.h"
#include "FreeFonts.h" // Include the header file attached to this sketch

static constexpr int WAIT              = 500;
static constexpr int SPRITE_WIDTH      = 320;
static constexpr int SPRITE_HEIGHT     = 240;
static constexpr int MAX_STRING_LENGTH = 30;

// Library instance
TFT_eSPI      tft        = TFT_eSPI();
TFT_eSprite   img        = TFT_eSprite(&tft);
unsigned long targetTime = 0; // Used for testing draw times
char          displayString[MAX_STRING_LENGTH];
char          formattedValue[8];

namespace TFT
{
    int xpos;
    int ypos;
    int fontHeight;

    void init()
    {
        tft.init();
        tft.initDMA();
        tft.setRotation(3);
        tft.fillScreen(TFT_BLACK);

        img.createSprite(SPRITE_WIDTH, SPRITE_HEIGHT);
        img.fillSprite(TFT_BLACK);
        img.setTextSize(1);
        img.setTextColor(TFT_WHITE, TFT_BLACK);
        img.setFreeFont(&FreeMono12pt7b);

        fontHeight = img.fontHeight(GFXFF);
    }

    void writeString(const ChangedValue property, const char *prefix, const char *formattedValue)
    {
        snprintf(displayString, MAX_STRING_LENGTH, prefix, formattedValue);
        img.drawString(displayString, 0, (int)property * fontHeight + 2, 4);
        img.pushSprite(0, 0);
    }

    void draw()
    {
        // Wait until we are signaled. This is a blocking call.
        ChangedValue property = (ChangedValue)rp2040.fifo.pop();

        // Arduino can't format floats in snprintf so use dtostrf to get the formatted value.
        switch (property) {
        case ChangedValue::Com1Active:
            dtostrf(values[(int)property], 7, 3, formattedValue);
            writeString(property, "COM1 %s", formattedValue);
            break;
        case ChangedValue::Com1Standby:
            dtostrf(values[(int)property], 7, 3, formattedValue);
            writeString(property, "COM1 STBY %s", formattedValue);
            break;
        case ChangedValue::Com2Active:
            dtostrf(values[(int)property], 7, 3, formattedValue);
            writeString(property, "COM2 %s", formattedValue);
            break;
        case ChangedValue::Com2Standby:
            dtostrf(values[(int)property], 7, 3, formattedValue);
            writeString(property, "COM2 STBY %s", formattedValue);
            break;
        case ChangedValue::Nav1Active:
            dtostrf(values[(int)property], 6, 2, formattedValue);
            writeString(property, "NAV1 %s", formattedValue);
            break;
        case ChangedValue::Nav1Standby:
            dtostrf(values[(int)property], 6, 2, formattedValue);
            writeString(property, "NAV1 STBY %s", formattedValue);
            break;
        case ChangedValue::Nav2Active:
            dtostrf(values[(int)property], 6, 2, formattedValue);
            writeString(property, "NAV2 %s", formattedValue);
            break;
        case ChangedValue::Nav2Standby:
            dtostrf(values[(int)property], 6, 2, formattedValue);
            writeString(property, "NAV2 STBY %s", formattedValue);
            break;
        }
    }
} // end of namespace TFT