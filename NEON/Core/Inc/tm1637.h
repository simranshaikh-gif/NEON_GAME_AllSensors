#ifndef __TM1637_H
#define __TM1637_H

#include "stm32f4xx_hal.h"

void TM1637_Init(void);
void TM1637_DisplayDecimal(int v, int displaySeparator);
void TM1637_SetBrightness(uint8_t brightness);

#endif // __TM1637_H
