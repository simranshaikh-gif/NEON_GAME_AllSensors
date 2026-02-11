#ifndef __TCS3200_H
#define __TCS3200_H

#include "stm32f4xx_hal.h"

void TCS3200_Init(void);
void TCS3200_SelectColor(uint8_t s2, uint8_t s3);
void TCS3200_PulseCallback(void);
uint32_t TCS3200_ReadFrequency(void);
const char *TCS3200_GetColorName(void);
void TCS3200_GetRGB(uint8_t *r, uint8_t *g, uint8_t *b);

#endif // __TCS3200_H
