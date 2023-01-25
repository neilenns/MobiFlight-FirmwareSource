#include <Arduino.h>
#include "TFT.h"
#include "IPC.h"
#include "FreeFonts.h" // Include the header file attached to this sketch

static constexpr int WAIT              = 500;
static constexpr int SPRITE_WIDTH      = 320;
static constexpr int SPRITE_HEIGHT     = 240;
static constexpr int MAX_STRING_LENGTH = 13;

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
    }

    void draw()
    {
        // Wait until we are signaled. This is a blocking call.
        ChangedValue property = (ChangedValue)rp2040.fifo.pop();

        switch (property) {
        case ChangedValue::Com1Active:
            // Arduino can't format floats in snprintf so use dtostrf to get the formatted value.
            dtostrf(values[(int)property], 7, 3, formattedValue);

            snprintf(displayString, MAX_STRING_LENGTH, "COM1 %s", formattedValue);
            img.drawString(displayString, 0, 0, 4);
            img.pushSprite(0, 0);
            break;
        }
    }
} // end of namespace TFT