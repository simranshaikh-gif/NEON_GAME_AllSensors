#include "dwin.h"
#include <string.h>

extern UART_HandleTypeDef huart2;

void DWIN_Init(void) {
  // UART is initialized in main.c
}

void DWIN_WriteVP(uint16_t addr, uint16_t value) {
  uint8_t frame[8];
  frame[0] = 0x5A; // Header
  frame[1] = 0xA5; // Header
  frame[2] = 0x05; // Length
  frame[3] = 0x82; // Write Command
  frame[4] = (addr >> 8) & 0xFF;
  frame[5] = addr & 0xFF;
  frame[6] = (value >> 8) & 0xFF;
  frame[7] = value & 0xFF;

  HAL_UART_Transmit(&huart2, frame, 8, 100);
}

void DWIN_Update(int temp, int light, int r, int g, int b, int colorId,
                 int highScore) {
	  DWIN_WriteVP(VP_TEMP, (uint16_t)temp);
	  DWIN_WriteVP(VP_LIGHT, (uint16_t)light);
	  DWIN_WriteVP(VP_COLOR_ID, (uint16_t)colorId);
	  DWIN_WriteVP(VP_COLOR_R, (uint16_t)r);
	  DWIN_WriteVP(VP_COLOR_G, (uint16_t)g);
	  DWIN_WriteVP(VP_COLOR_B, (uint16_t)b);
	  DWIN_WriteVP(VP_HIGH_SCORE, (uint16_t)highScore);
}
