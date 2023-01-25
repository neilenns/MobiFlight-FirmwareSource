#include <Arduino.h>
#include "TFT.h"
#include "FreeFonts.h" // Include the header file attached to this sketch

static constexpr int WAIT          = 500;
static constexpr int SPRITE_WIDTH  = 320;
static constexpr int SPRITE_HEIGHT = 240;

// Library instance
TFT_eSPI      tft        = TFT_eSPI();
TFT_eSprite   img        = TFT_eSprite(&tft);
unsigned long targetTime = 0; // Used for testing draw times

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
    }

    void draw()
    {
        // Wait until we are signaled. This is a blocking call.
        rp2040.fifo.pop();

        img.fillSprite(TFT_BLACK);
        img.setTextColor(TFT_WHITE, TFT_BLACK);

        img.drawString("COM1 ", 0, 0, 4);
        img.pushSprite(0, 0);
    }
} // end of namespace TFT