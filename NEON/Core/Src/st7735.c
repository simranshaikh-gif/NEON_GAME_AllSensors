#include "st7735.h"
#include "main.h"
#include <stdio.h>

// --- COMMAND DEFINITIONS (From Gemini) ---
#define ST7735_CMD_SWRESET 0x01
#define ST7735_CMD_SLPOUT 0x11
#define ST7735_CMD_FRMCTR1 0xB1
#define ST7735_CMD_FRMCTR2 0xB2
#define ST7735_CMD_FRMCTR3 0xB3
#define ST7735_CMD_INVCTR 0xB4
#define ST7735_CMD_PWCTR1 0xC0
#define ST7735_CMD_PWCTR2 0xC1
#define ST7735_CMD_PWCTR3 0xC2
#define ST7735_CMD_PWCTR4 0xC3
#define ST7735_CMD_PWCTR5 0xC4
#define ST7735_CMD_VMCTR1 0xC5
#define ST7735_CMD_INVOFF 0x20
#define ST7735_CMD_MADCTL 0x36
#define ST7735_CMD_COLMOD 0x3A
#define ST7735_CMD_CASET 0x2A
#define ST7735_CMD_RASET 0x2B
#define ST7735_CMD_RAMWR 0x2C
#define ST7735_CMD_GMCTRP1 0xE0
#define ST7735_CMD_GMCTRN1 0xE1
#define ST7735_CMD_NORON 0x13
#define ST7735_CMD_DISPON 0x29

// --- GPIO MACROS ---
#define CS_LOW() HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_RESET)
#define CS_HIGH() HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_SET)
#define DC_LOW() HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET)
#define DC_HIGH() HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET)
#define RST_LOW()                                                              \
  HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_RESET)
#define RST_HIGH()                                                             \
  HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_SET)

extern SPI_HandleTypeDef hspi1;

// --- GLOBALS ---
uint8_t colstart = 0;
uint8_t rowstart = 0;

// --- LOW LEVEL SPI (HAL Access) ---
static void SPI_Write(uint8_t data) { HAL_SPI_Transmit(&hspi1, &data, 1, 100); }

static void ST7735_WriteCommand(uint8_t cmd) {
  DC_LOW();
  CS_LOW();
  SPI_Write(cmd);
  CS_HIGH();
}

static void ST7735_WriteData(uint8_t *buff, size_t buff_size) {
  DC_HIGH();
  CS_LOW();
  HAL_SPI_Transmit(&hspi1, buff, buff_size, 100);
  CS_HIGH();
}

void ST7735_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1,
                             uint16_t y1) {
  // Add Offsets for Green Tab / Various Panel Types
  x0 += colstart;
  x1 += colstart;
  y0 += rowstart;
  y1 += rowstart;

  uint8_t data[4];
  ST7735_WriteCommand(ST7735_CMD_CASET);
  data[0] = x0 >> 8;
  data[1] = x0 & 0xFF;
  data[2] = x1 >> 8;
  data[3] = x1 & 0xFF;
  ST7735_WriteData(data, 4);

  ST7735_WriteCommand(ST7735_CMD_RASET);
  data[0] = y0 >> 8;
  data[1] = y0 & 0xFF;
  data[2] = y1 >> 8;
  data[3] = y1 & 0xFF;
  ST7735_WriteData(data, 4);

  ST7735_WriteCommand(ST7735_CMD_RAMWR);
}

// --- INITIALIZATION (Standard Black Tab) ---
void ST7735_Init(void) {
  // Config for "Green Tab" 1.8" (Often needs offsets)
  // 128x160 panel in Portrait
  colstart = 0;
  rowstart = 0;

  // Ensure we start in a known state
  CS_HIGH();

  printf("ST7735: RST LOW\r\n");
  RST_LOW();
  HAL_Delay(50); // Increased reset time
  RST_HIGH();
  HAL_Delay(50);

  printf("ST7735: Sending Commands (Standard)...\r\n");

  ST7735_WriteCommand(ST7735_CMD_SWRESET);
  HAL_Delay(150);
  ST7735_WriteCommand(ST7735_CMD_SLPOUT);
  HAL_Delay(255);

  // FRMCTR1 (Frame Rate Control normal mode) -> Rate = fosc/(1x2+40) *
  // (LINE+2C+2D)
  ST7735_WriteCommand(ST7735_CMD_FRMCTR1);
  uint8_t data1[] = {0x01, 0x2C, 0x2D};
  ST7735_WriteData(data1, sizeof(data1));

  // FRMCTR2 (Frame Rate Control idle mode)
  ST7735_WriteCommand(ST7735_CMD_FRMCTR2);
  ST7735_WriteData(data1, sizeof(data1));

  // FRMCTR3 (Frame Rate Control partial mode)
  ST7735_WriteCommand(ST7735_CMD_FRMCTR3);
  uint8_t data2[] = {0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D};
  ST7735_WriteData(data2, sizeof(data2));

  // INVCTR (Display Inversion Control)
  ST7735_WriteCommand(ST7735_CMD_INVCTR);
  uint8_t data3[] = {0x07};
  ST7735_WriteData(data3, sizeof(data3));

  // PWCTR1 (Power Control 1)
  ST7735_WriteCommand(ST7735_CMD_PWCTR1);
  uint8_t data4[] = {0xA2, 0x02, 0x84};
  ST7735_WriteData(data4, sizeof(data4));

  // PWCTR2 (Power Control 2)
  ST7735_WriteCommand(ST7735_CMD_PWCTR2);
  uint8_t data5[] = {0xC5};
  ST7735_WriteData(data5, sizeof(data5));

  // PWCTR3 (Power Control 3)
  ST7735_WriteCommand(ST7735_CMD_PWCTR3);
  uint8_t data6[] = {0x0A, 0x00};
  ST7735_WriteData(data6, sizeof(data6));

  // PWCTR4 (Power Control 4)
  ST7735_WriteCommand(ST7735_CMD_PWCTR4);
  uint8_t data7[] = {0x8A, 0x2A};
  ST7735_WriteData(data7, sizeof(data7));

  // PWCTR5 (Power Control 5)
  ST7735_WriteCommand(ST7735_CMD_PWCTR5);
  uint8_t data8[] = {0x8A, 0xEE};
  ST7735_WriteData(data8, sizeof(data8));

  // VMCTR1 (VCOM Control 1)
  ST7735_WriteCommand(ST7735_CMD_VMCTR1);
  uint8_t data9[] = {0x0E};
  ST7735_WriteData(data9, sizeof(data9));

  // INVOFF (Display Inversion Off)
  ST7735_WriteCommand(ST7735_CMD_INVOFF);

  // MADCTL (Memory Data Access Control)
  ST7735_WriteCommand(ST7735_CMD_MADCTL);
  // 0xA8 = 10101000 = MY=1, MX=0, MV=1(Landscape), BGR=1
  uint8_t data10[] = {0xA8};
  ST7735_WriteData(data10, sizeof(data10));

  // COLMOD (Interface Pixel Format)
  ST7735_WriteCommand(ST7735_CMD_COLMOD);
  uint8_t data11[] = {0x05}; // 16-bit/pixel
  ST7735_WriteData(data11, sizeof(data11));

  // GMCTRP1 (Gamma '+' polarity Correction)
  ST7735_WriteCommand(ST7735_CMD_GMCTRP1);
  uint8_t data12[] = {0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d,
                      0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10};
  ST7735_WriteData(data12, sizeof(data12));

  // GMCTRN1 (Gamma '-' polarity Correction)
  ST7735_WriteCommand(ST7735_CMD_GMCTRN1);
  uint8_t data13[] = {0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
                      0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10};
  ST7735_WriteData(data13, sizeof(data13));

  // NORON (Normal Display Mode On)
  ST7735_WriteCommand(ST7735_CMD_NORON);
  HAL_Delay(10);

  // DISPON (Display On)
  ST7735_WriteCommand(ST7735_CMD_DISPON);
  HAL_Delay(100);

  printf("ST7735: Init Done (Standard).\\r\\n");
}

/*
   Note: Gemini uses HAL_SPI_Transmit with a buffer for filling rectangles.
   We stick to our optimized Register Access loop for simplicity and speed.
*/
void ST7735_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                          uint16_t color) {
  if ((x >= ST7735_WIDTH) || (y >= ST7735_HEIGHT))
    return;
  if ((x + w - 1) >= ST7735_WIDTH)
    w = ST7735_WIDTH - x;
  if ((y + h - 1) >= ST7735_HEIGHT)
    h = ST7735_HEIGHT - y;

  ST7735_SetAddressWindow(x, y, x + w - 1, y + h - 1);

  uint8_t data[2] = {color >> 8, color & 0xFF};
  DC_HIGH();
  CS_LOW();
  for (uint32_t i = 0; i < w * h; i++) {
    SPI_Write(data[0]);
    SPI_Write(data[1]);
  }
  CS_HIGH();
}

void ST7735_FillScreen(uint16_t color) {
  ST7735_FillRectangle(0, 0, ST7735_WIDTH, ST7735_HEIGHT, color);
}

void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
  ST7735_FillRectangle(x, y, 1, 1, color);
}

// --- 7-SEGMENT ENGINE (Preserved from original) ---
static const uint8_t SEGMENT_MAP[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66,
                                      0x6D, 0x7D, 0x07, 0x7F, 0x6F};

void ST7735_DrawSevenSegDigit(uint16_t x, uint16_t y, uint8_t digit,
                              uint16_t color, uint8_t size) {
  if (digit > 9)
    return;
  uint8_t segments = SEGMENT_MAP[digit];
  uint16_t h_th = 2 * size;
  uint16_t v_th = 2 * size;
  uint16_t len = 6 * size;

  if (segments & 0x01)
    ST7735_FillRectangle(x + v_th, y, len, h_th, color);
  if (segments & 0x02)
    ST7735_FillRectangle(x + v_th + len, y + h_th, v_th, len, color);
  if (segments & 0x04)
    ST7735_FillRectangle(x + v_th + len, y + 2 * h_th + len, v_th, len, color);
  if (segments & 0x08)
    ST7735_FillRectangle(x + v_th, y + 2 * len + 2 * h_th, len, h_th, color);
  if (segments & 0x10)
    ST7735_FillRectangle(x, y + 2 * h_th + len, v_th, len, color);
  if (segments & 0x20)
    ST7735_FillRectangle(x, y + h_th, v_th, len, color);
  if (segments & 0x40)
    ST7735_FillRectangle(x + v_th, y + len + h_th, len, h_th, color);
}

void ST7735_DrawSevenSegNumber(uint16_t x, uint16_t y, int number,
                               uint8_t digits, uint16_t color, uint8_t size) {
  int divider = 1;
  for (int i = 0; i < digits - 1; i++)
    divider *= 10;
  for (int i = 0; i < digits; i++) {
    int d = (number / divider) % 10;
    ST7735_DrawSevenSegDigit(x + i * (10 * size), y, d, color, size);
    divider /= 10;
  }
}

// --- FONT ENGINE IMPLEMENTATION ---
void ST7735_DrawChar(uint16_t x, uint16_t y, char ch, FontDef font,
                     uint16_t color, uint16_t bgcolor) {
  uint32_t i, b, j;

  ST7735_SetAddressWindow(x, y, x + font.width - 1, y + font.height - 1);

  uint8_t color_data[2] = {color >> 8, color & 0xFF};
  uint8_t bg_data[2] = {bgcolor >> 8, bgcolor & 0xFF};

  DC_HIGH();
  CS_LOW();
  for (i = 0; i < font.height; i++) {
    b = font.data[(ch - 32) * font.height + i];
    for (j = 0; j < font.width; j++) {
      if ((b << j) & 0x8000) {
        HAL_SPI_Transmit(&hspi1, color_data, 2, 10);
      } else {
        HAL_SPI_Transmit(&hspi1, bg_data, 2, 10);
      }
    }
  }
  CS_HIGH();
}

void ST7735_WriteString(uint16_t x, uint16_t y, const char *str, FontDef font,
                        uint16_t color, uint16_t bgcolor) {
  while (*str) {
    if (x + font.width >= ST7735_WIDTH) {
      x = 0;
      y += font.height;
      if (y + font.height >= ST7735_HEIGHT) {
        break;
      }
      if (*str == ' ') {
        // skip spaces in the beginning of the new line
        str++;
        continue;
      }
    }

    ST7735_DrawChar(x, y, *str, font, color, bgcolor);
    x += font.width;
    str++;
  }
}
