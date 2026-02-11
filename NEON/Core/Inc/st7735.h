#ifndef __ST7735_H
#define __ST7735_H

#include <stdbool.h>
#include <stdint.h>

// #include "fonts.h" // Removed as content is inlined
#include "stm32f4xx_hal.h"

// Inlined content from fonts.h
#ifndef __FONTS_H__
#define __FONTS_H__

#include <stdint.h>

typedef struct {
  const uint8_t width;
  uint8_t height;
  const uint16_t *data;
} FontDef;

extern const FontDef Font_7x10;
extern const FontDef Font_11x18;
extern const FontDef Font_16x26;

#endif // __FONTS_H__

// Screen resolution (Landscape)
#define ST7735_WIDTH 160
#define ST7735_HEIGHT 128

// Colors
#define ST7735_BLACK 0x0000
#define ST7735_BLUE 0x001F
#define ST7735_RED 0xF800
#define ST7735_GREEN 0x07E0
#define ST7735_CYAN 0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW 0xFFE0
#define ST7735_WHITE 0xFFFF
#define ST7735_ORANGE 0xFD20

// User Functions
void ST7735_Init(void);
void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7735_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                          uint16_t color);
void ST7735_FillScreen(uint16_t color);
void ST7735_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                      const uint16_t *data);
void ST7735_InvertColors(uint8_t invert);

// Text Functions
void ST7735_DrawChar(uint16_t x, uint16_t y, char ch, FontDef font,
                     uint16_t color, uint16_t bgcolor);
void ST7735_WriteString(uint16_t x, uint16_t y, const char *str, FontDef font,
                        uint16_t color, uint16_t bgcolor);

// Retro UI Functions
void ST7735_DrawSevenSegDigit(uint16_t x, uint16_t y, uint8_t digit,
                              uint16_t color, uint8_t size);
void ST7735_DrawSevenSegNumber(uint16_t x, uint16_t y, int number,
                               uint8_t digits, uint16_t color, uint8_t size);

#endif // __ST7735_H
