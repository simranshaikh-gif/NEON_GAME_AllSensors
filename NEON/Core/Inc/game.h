#ifndef __GAME_H
#define __GAME_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>


typedef enum {
  STATE_MENU,
  STATE_RUNNING,
  STATE_PAUSED,
  STATE_GAME_OVER,
  STATE_HIGH_SCORES
} GameState_t;

typedef enum {
  DIFF_EASY,   // 1.5x speed
  DIFF_NORMAL, // 2.5x speed
  DIFF_HARD    // 4.0x speed
} Difficulty_t;

void Game_Init(void);
void Game_Update(void);
void Game_Render(void);

#endif // __GAME_H
