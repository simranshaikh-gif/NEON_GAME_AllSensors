#include "tm1637.h"
#include "input.h"
#include "main.h"

// TM1637 Commands
#define TM1637_CMD_SET_DATA 0x40
#define TM1637_CMD_SET_ADDR 0xC0
#define TM1637_CMD_SET_DSPLY 0x80

// Delay for bit-banging
// Increased delay for stability
static void TM1637_Delay(void) {
  for (int i = 0; i < 500; i++)
    __NOP();
}

static void TM1637_ClkHigh(void) {
  HAL_GPIO_WritePin(SEG_CLK_GPIO_Port, SEG_CLK_Pin, GPIO_PIN_SET);
}

static void TM1637_ClkLow(void) {
  HAL_GPIO_WritePin(SEG_CLK_GPIO_Port, SEG_CLK_Pin, GPIO_PIN_RESET);
}

static void TM1637_DioHigh(void) {
  HAL_GPIO_WritePin(SEG_DIO_GPIO_Port, SEG_DIO_Pin, GPIO_PIN_SET);
}

static void TM1637_DioLow(void) {
  HAL_GPIO_WritePin(SEG_DIO_GPIO_Port, SEG_DIO_Pin, GPIO_PIN_RESET);
}

static void TM1637_Start(void) {
  TM1637_ClkHigh();
  TM1637_DioHigh();
  TM1637_Delay();
  TM1637_DioLow();
  TM1637_Delay();
  TM1637_ClkLow();
  TM1637_Delay();
}

static void TM1637_Stop(void) {
  TM1637_ClkLow();
  TM1637_Delay();
  TM1637_DioLow();
  TM1637_Delay();
  TM1637_ClkHigh();
  TM1637_Delay();
  TM1637_DioHigh();
  TM1637_Delay();
}

static void TM1637_WriteByte(uint8_t b) {
  for (int i = 0; i < 8; i++) {
    TM1637_ClkLow();
    if (b & 0x01)
      TM1637_DioHigh();
    else
      TM1637_DioLow();
    TM1637_Delay();
    TM1637_ClkHigh();
    TM1637_Delay();
    b >>= 1;
  }

  // ACK
  TM1637_ClkLow();
  TM1637_DioHigh(); // Release SDA (ideally set input, but pulling high is OK
                    // for now)
  TM1637_ClkHigh();
  TM1637_Delay();
  // We don't check ACK for simplicity here
  TM1637_ClkLow();
}

const uint8_t digitToSegment[] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
    0x77, // A
    0x7C, // b
    0x39, // C
    0x5E, // d
    0x79, // E
    0x71  // F
};

void TM1637_Init(void) {
  // Pins are init in main.c MX_GPIO_Init
  TM1637_SetBrightness(7);
}

void TM1637_SetBrightness(uint8_t brightness) {
  TM1637_Start();
  TM1637_WriteByte(TM1637_CMD_SET_DSPLY | (brightness & 0x07) | 0x08);
  TM1637_Stop();
}

void TM1637_DisplayDecimal(int v, int displaySeparator) {
  uint8_t data[4];
  // Clamp
  if (v > 9999)
    v = 9999;

  data[3] = digitToSegment[v % 10];
  v /= 10;
  data[2] = digitToSegment[v % 10];
  v /= 10;
  data[1] = digitToSegment[v % 10];
  v /= 10;
  data[0] = digitToSegment[v % 10];

  // Handle Separator (Colon)
  if (displaySeparator) {
    data[1] |= 0x80; // Assuming dot/colon is MSB
  }

  // Write Data
  TM1637_Start();
  TM1637_WriteByte(TM1637_CMD_SET_DATA);
  TM1637_Stop();

  TM1637_Start();
  TM1637_WriteByte(TM1637_CMD_SET_ADDR);
  for (int i = 0; i < 4; i++) {
    TM1637_WriteByte(data[i]);
  }
  TM1637_Stop();

  TM1637_SetBrightness(7); // Reactivate
}
