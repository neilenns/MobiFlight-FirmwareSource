#pragma once

#include <TFT_eSPI.h>
#include <SD.h>

#define MAX_CLIPPING_RADIUS 300

extern TFT_eSPI    tft;
extern TFT_eSprite spr[];
extern uint16_t   *sprPtr[];

template <typename T>
static inline void
swap_coord(T &a, T &b)
{
    T t = a;
    a   = b;
    b   = t;
}

namespace TFT
{
    void init();
    void setClippingArea(int32_t ClippingX0, int32_t ClippingY0, int32_t ClippingXwidth, int32_t ClippingYwidth, int32_t clippingRadiusOuter, int32_t clippingRadiusInner);
    void drawLineClipped(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color, bool sel);
    void fillHalfCircleSprite(int32_t x0, int32_t y0, int32_t r, uint32_t colorUpper, uint32_t colorLower, bool sel);
    void fillHalfCircleTFT(int32_t x0, int32_t y0, int32_t r, uint32_t colorUpper, uint32_t colorLower);
    void drawPixelClipped(int32_t x, int32_t y, uint32_t color, bool sel);
    void drawFastVLineClipped(int32_t x, int32_t y, int32_t h, uint32_t color, bool sel);
    void drawFastHLineClipped(int32_t x, int32_t y, int32_t w, uint32_t color, bool sel);
}