#include "tcs3200.h"
#include "main.h"

volatile uint32_t pulse_count = 0;

void TCS3200_Init(void) {
  // S0, S1 freq scaling (20%)
  HAL_GPIO_WritePin(TCS_S0_GPIO_Port, TCS_S0_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(TCS_S1_GPIO_Port, TCS_S1_Pin, GPIO_PIN_RESET);

  // Enable (OE Low)
  HAL_GPIO_WritePin(TCS_OE_GPIO_Port, TCS_OE_Pin, GPIO_PIN_RESET);
}

void TCS3200_SelectColor(uint8_t s2, uint8_t s3) {
  HAL_GPIO_WritePin(TCS_S2_GPIO_Port, TCS_S2_Pin,
                    s2 ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(TCS_S3_GPIO_Port, TCS_S3_Pin,
                    s3 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void TCS3200_PulseCallback(void) { pulse_count++; }

uint32_t TCS3200_ReadFrequency(void) {
  pulse_count = 0;
  HAL_Delay(40);           // Increased measurement window for stability
  return pulse_count * 25; // Scale to Hz (approx)
}

uint8_t map_val(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min,
                uint32_t out_max) {
  if (x < in_min)
    return out_min;
  if (x > in_max)
    return out_max;
  return (uint8_t)((x - in_min) * (out_max - out_min) / (in_max - in_min) +
                   out_min);
}

void TCS3200_GetRGB(uint8_t *r, uint8_t *g, uint8_t *b) {
  // 1. Read Raw Values (mapped 0-255)
  TCS3200_SelectColor(0, 0); // Red
  uint8_t rawR = map_val(TCS3200_ReadFrequency(), 200, 20000, 0, 255);

  TCS3200_SelectColor(1, 1); // Green
  uint8_t rawG = map_val(TCS3200_ReadFrequency(), 200, 20000, 0, 255);

  TCS3200_SelectColor(0, 1); // Blue
  uint8_t rawB = map_val(TCS3200_ReadFrequency(), 200, 20000, 0, 255);

  // 1.5 Low Level Filter (Ambient Light Rejection)
  // User says Red=55 when nothing is there. We set threshold to 60.
  if (rawR < 60 && rawG < 60 && rawB < 60) {
    *r = 0;
    *g = 0;
    *b = 0;
    return;
  }

  // 2. Winner-Takes-All Logic (User Request)
  // Only the dominant color keeps its value; others become 0.
  if (rawR >= rawG && rawR >= rawB) {
    *r = rawR;
    *g = 0;
    *b = 0;
  } else if (rawG >= rawR && rawG >= rawB) {
    *r = 0;
    *g = rawG;
    *b = 0;
  } else {
    *r = 0;
    *g = 0;
    *b = rawB;
  }
}

const char *TCS3200_GetColorName(void) {
  uint8_t r, g, b;
  TCS3200_GetRGB(&r, &g, &b);

  if (r > 150 && g < 100 && b < 100)
    return "RED";
  if (g > 150 && r < 100 && b < 100)
    return "GREEN";
  if (b > 150 && r < 100 && g < 100)
    return "BLUE";

  return "UNKNOWN";
}
