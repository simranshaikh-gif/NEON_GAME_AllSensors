#include "game.h"
#include "audio.h"
#include "fatfs.h"
#include "fonts.h"
#include "input.h"
#include "main.h"
#include "st7735.h"
#include "tcs3200.h"
#include "tm1637.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Constants ---
#define GROUND_Y 110
#define SKY_Y 20
#define PLAYER_X 15
#define PLAYER_W 10
#define PLAYER_H 16
// LED3 Removed (Conflicts with TEMP_Pin on PB0)
#define LANE_L 0
#define LANE_R 1

// --- Typedefs ---
typedef enum { RUNNING, JUMPING, SLIDING, CRASHED } PlayerState;

typedef struct {
  float y;
  float vy;
  int lane; // 0 or 1
  PlayerState state;
  // Previous State for Rendering
  float old_y;
  PlayerState old_state;
  int old_lane;
  bool shooting;
  bool old_shooting;
  // Double Jump Logic
  int jumps;        // Current jump count
  bool prevJumpCmd; // Edge detection for jump button
} Player;

typedef struct {
  float x;
  int type; // 0=High, 1=Low, 2=LaneBlock
  int lane;
  bool active;
  // Previous State
  float old_x;
  bool was_active;
} Obstacle;

#define MAX_OBSTACLES 5
Obstacle obstacles[MAX_OBSTACLES];

// --- Globals ---
// Game State
GameState_t currentState = STATE_MENU;
Difficulty_t currentDifficulty = DIFF_NORMAL;
Player player;
uint32_t score = 0;
uint32_t highScore = 0;
float speed = 2.0f;
bool firstFrame = true;

// New Globals for Time Dilation
float energy = 50.0f; // 0-100
int multiplier = 1;
bool isShooting = false; // Visual flag
int shootCooldown = 0;   // Shooting Latch to prevent missed hits

void RenderRunningGame(void);
void UpdateRunningLogic(void);

// --- SD Card Persistence ---
void SaveHighScore(void) {
  printf("SD: Saving High Score: %lu\r\n", highScore);
  FRESULT res = f_mount(&USERFatFS, "", 1);
  if (res == FR_OK) {
    res = f_open(&USERFile, "score.txt", FA_WRITE | FA_CREATE_ALWAYS);
    if (res == FR_OK) {
      char buf[16];
      sprintf(buf, "%lu\n", highScore); // Added newline for PC visibility
      UINT bw = 0;
      FRESULT res_write = f_write(&USERFile, buf, strlen(buf), &bw);
      FRESULT res_close = f_close(&USERFile);
      
      if (res_write == FR_OK && res_close == FR_OK && bw > 0) {
        printf("SD: Permanent Save OK! Score: %lu\r\n", highScore);
      } else {
        printf("SD: Write FAILED! (Write: %u, Close: %u)\r\n", (unsigned int)res_write, (unsigned int)res_close);
      }
    } else {
      printf("SD: Open ERROR (code: %u)\r\n", (unsigned int)res);
    }
  } else {
    printf("SD: Mount ERROR (code: %u)\r\n", (unsigned int)res);
  }
}

void LoadHighScore(void) {
  printf("SD: Loading High Score...\r\n");
  FRESULT res = f_mount(&USERFatFS, "", 1);
  if (res == FR_OK) {
    res = f_open(&USERFile, "score.txt", FA_READ);
    if (res == FR_OK) {
      char buf[16];
      UINT br;
      if (f_read(&USERFile, buf, sizeof(buf) - 1, &br) == FR_OK) {
        buf[br] = '\0';
        highScore = strtoul(buf, NULL, 10);
        printf("SD: Score loaded: %lu\r\n", highScore);
      }
      f_close(&USERFile);
    } else {
      printf("SD: score.txt not found (code: %u), using 0\r\n",
             (unsigned int)res);
      highScore = 0;
    }
  } else {
    printf("SD: Mount ERROR (code: %u)\r\n", (unsigned int)res);
  }
}

// Consolidated LED Control for multi-port assignments
void SetGameLEDs(GPIO_PinState state) {
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, state);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, state);
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, state);
  HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, state);

  // RGB LED (Combined)
  HAL_GPIO_WritePin(RGB_R_GPIO_Port, RGB_R_Pin, state);
  HAL_GPIO_WritePin(RGB_G_GPIO_Port, RGB_G_Pin, state);
  HAL_GPIO_WritePin(RGB_B_GPIO_Port, RGB_B_Pin, state);

  // On-board LD2
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, state);
}

void SetGameRelays(GPIO_PinState state) {
  HAL_GPIO_WritePin(RL1_GPIO_Port, RL1_Pin, state);
  HAL_GPIO_WritePin(RL2_GPIO_Port, RL2_Pin, state);
}

// --- Helper Functions ---
void SpawnObstacle() {
  // Check Safe Gap first to prevent overlapping "Glitches"
  bool safeToSpawn = true;
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (obstacles[i].active) {
      if (obstacles[i].x >
          ST7735_WIDTH - 80) { // Gap of 80px (wider for 160 width)
        safeToSpawn = false;
        break;
      }
    }
  }

  if (safeToSpawn) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
      if (!obstacles[i].active) {
        obstacles[i].active = true;
        obstacles[i].x = ST7735_WIDTH + 10;
        obstacles[i].old_x = obstacles[i].x;
        obstacles[i].was_active = false;

        // Types: 0=High, 1=Low, 2=Destructible
        int r = rand() % 100;
        if (r < 60)
          obstacles[i].type = 2; // Target
        else if (r < 80)
          obstacles[i].type = 1; // Low
        else
          obstacles[i].type = 0; // High

        obstacles[i].lane = rand() % 2;
        break;
      }
    }
  }
}

void Game_SetState(GameState_t newState) {
  currentState = newState;
  ST7735_FillScreen(ST7735_BLACK);

  if (newState == STATE_RUNNING) {
    // Reset Game Variables
    player.y = GROUND_Y;
    player.vy = 0;
    player.lane = LANE_L;
    player.state = RUNNING;
    player.jumps = 0;
    score = 0;
    energy = 50.0f;
    isShooting = false;

    // Speed based on difficulty
    if (currentDifficulty == DIFF_EASY)
      speed = 1.8f;
    else if (currentDifficulty == DIFF_NORMAL)
      speed = 2.5f;
    else
      speed = 3.5f;

    for (int i = 0; i < MAX_OBSTACLES; i++)
      obstacles[i].active = false;

    ST7735_FillRectangle(0, GROUND_Y, ST7735_WIDTH, 2, ST7735_WHITE);
    SetGameLEDs(GPIO_PIN_RESET);
    SetGameRelays(GPIO_PIN_RESET);
  } else if (newState == STATE_GAME_OVER) {
    // Update High Score
    if (score > highScore) {
      highScore = score;
      SaveHighScore(); // Persistent Record!
    }
    SetGameLEDs(GPIO_PIN_SET);
  } else if (newState == STATE_MENU) {
    SetGameLEDs(GPIO_PIN_RESET);
    SetGameRelays(GPIO_PIN_RESET);
  }
}

// --- Main Game Functions ---
void Game_Init(void) {
  printf("Game: Init Neon Survivor (SD Persistence Integration)\r\n");
  ST7735_Init();
  LoadHighScore(); // Load record from SD
  Game_SetState(STATE_MENU);
}

void Game_Update(void) {
  Input_Poll();

  switch (currentState) {
  case STATE_MENU:
    // Difficulty
    if (Inputs.KeyJustPressed >= '1' && Inputs.KeyJustPressed <= '3') {
      currentDifficulty = (Difficulty_t)(Inputs.KeyJustPressed - '1');
    }

    if (Inputs.KeyJustPressed == '*') {
      Game_SetState(STATE_RUNNING);
    }
    break;

  case STATE_PAUSED:
    if (Inputs.KeyJustPressed == '*') {
      currentState = STATE_RUNNING;
      ST7735_FillScreen(ST7735_BLACK); // Clean wipe
      ST7735_FillRectangle(0, GROUND_Y, ST7735_WIDTH, 2, ST7735_WHITE);
    }
    break;

  case STATE_GAME_OVER:
    if (Inputs.KeyJustPressed == '*') {
      Game_SetState(STATE_MENU); // Back to start
    }
    if (Inputs.KeyJustPressed == '9') {
      Game_SetState(STATE_HIGH_SCORES); // View scores
    }
    break;

  case STATE_HIGH_SCORES:
    if (Inputs.KeyJustPressed == '*') {
      Game_SetState(STATE_MENU);
    }
    break;

  case STATE_RUNNING:
    if (Inputs.KeyJustPressed == '#') {
      currentState = STATE_PAUSED;
      return;
    }

    // ... (Existing Game Logic here, moved into case) ...
    UpdateRunningLogic();
    break;
  default:
    break;
  }
}

// Private helper to keep Update clean
void UpdateRunningLogic(void) {
  // Save Old State
  player.old_y = player.y;
  player.old_state = player.state;
  player.old_lane = player.lane;
  player.old_shooting = player.shooting;
  isShooting = false;

  for (int i = 0; i < MAX_OBSTACLES; i++) {
    obstacles[i].old_x = obstacles[i].x;
    obstacles[i].was_active = obstacles[i].active;
  }

  // Dynamic Speed Control via Rotation Sensor
  float baseSpeed = 3.5f; // Normal
  if (currentDifficulty == DIFF_EASY)
    baseSpeed = 2.4f;
  else if (currentDifficulty == DIFF_HARD)
    baseSpeed = 5.5f;

  speed = baseSpeed * Inputs.SpeedMultiplier;

  // Sync energy/multiplier with the wider speed range
  if (Inputs.SpeedMultiplier > 2.0f) {
    energy += 0.2f; // High speed rewards more energy
    multiplier = 4;
  } else if (Inputs.SpeedMultiplier > 1.0f) {
    energy += 0.05f;
    multiplier = 2;
  } else {
    // Slower speeds drain or stabilize energy
    if (Inputs.SpeedMultiplier < 0.7f && energy > 0)
      energy -= 0.1f;
    multiplier = 1;
  }

  if (energy > 100.0f)
    energy = 100.0f;
  if (energy < 0.0f)
    energy = 0.0f;

  // Jump Logic with Double Jump
  bool jumpPressed = Inputs.JumpCmd && !player.prevJumpCmd;
  player.prevJumpCmd = Inputs.JumpCmd;

  if (jumpPressed) {
    if (player.state == RUNNING) {
      player.state = JUMPING;
      player.jumps = 1;
      player.vy = (multiplier == 4) ? -9.5f : (multiplier == 1 ? -6.5f : -7.5f);
      Audio_PlayJump();
      SetGameLEDs(GPIO_PIN_SET);
      SetGameRelays(GPIO_PIN_SET);
    } else if (player.state == JUMPING && player.jumps < 2) {
      player.jumps++;
      Audio_PlayJump();
      SetGameLEDs(GPIO_PIN_SET);
      SetGameRelays(GPIO_PIN_SET);
      player.vy = (multiplier == 4) ? -8.5f : (multiplier == 1 ? -5.5f : -6.5f);
    }
  }

  // Slide / Fast Fall
  if (Inputs.DuckCmd) {
    if (player.state == RUNNING) {
      player.state = SLIDING;
      Audio_PlayDuck();
    } else if (player.state == JUMPING)
      player.vy += 2.0f;
  } else {
    if (player.state == SLIDING)
      player.state = RUNNING;
  }

  // Shoot
  if (Inputs.ShootCmd) {
    isShooting = true;
    player.shooting = true;
    SetGameLEDs(GPIO_PIN_SET);
    int current_h = (multiplier == 4) ? 22 : PLAYER_H;
    int py = (int)player.y - (current_h / 2);

    for (int i = 0; i < MAX_OBSTACLES; i++) {
      if (obstacles[i].active && obstacles[i].type == 2) {
        float oTop = GROUND_Y - 25, oBottom = GROUND_Y - 15,
              oLeft = obstacles[i].x;
        if (obstacles[i].lane == player.lane && oLeft < ST7735_WIDTH) {
          if (py >= (oTop - 8) && py <= (oBottom + 8)) {
            obstacles[i].active = false;
            score++;
          }
        }
      }
    }
  } else {
    isShooting = false;
    player.shooting = false;
    // Don't turn OFF LEDs here if we are still jumping
    if (player.state != JUMPING) {
      SetGameLEDs(GPIO_PIN_RESET);
    }
  }

  // Keypad Actions in Running State
  if (Inputs.KeyJustPressed == '4')
    player.lane = LANE_L;
  if (Inputs.KeyJustPressed == '6')
    player.lane = LANE_R;

  // Ultimate Shield (0 key - One time powerup with penalty)
  if (Inputs.KeyJustPressed == '0' && score > 20) {
    score /= 2; // Penalty
    for (int i = 0; i < MAX_OBSTACLES; i++)
      obstacles[i].active = false; // Wipe screen
  }

  // Physics
  if (player.state == JUMPING) {
    player.y += player.vy;
    player.vy += 0.85f;
    if (player.y >= GROUND_Y) {
      player.y = GROUND_Y;
      player.state = RUNNING;
      player.vy = 0;
      player.jumps = 0;
      // Turn OFF LEDs/Relays on landing if not shooting
      if (!isShooting) {
        SetGameLEDs(GPIO_PIN_RESET);
        SetGameRelays(GPIO_PIN_RESET);
      }
    }
  }

  // Obstacles
  if (rand() % 100 < (int)(speed * 3.0f))
    SpawnObstacle();

  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (obstacles[i].active) {
      obstacles[i].x -= speed;
      float pLeft = PLAYER_X, pRight = PLAYER_X + PLAYER_W;
      int current_h = (multiplier == 4) ? 22 : PLAYER_H;
      float pH = (player.state == SLIDING) ? (current_h / 2) : current_h;
      float pTop = player.y - pH, pBottom = player.y;

      float oLeft = obstacles[i].x, oRight = obstacles[i].x + 10;
      float oTop = GROUND_Y - 20, oBottom = GROUND_Y;


      if (obstacles[i].type == 0) {
        // High obstacles need more clearance in landscape
        oBottom = GROUND_Y - 15;
        oTop = GROUND_Y - 35;
      } else if (obstacles[i].type == 1) {
        oTop = GROUND_Y - 10;
      } else if (obstacles[i].type == 2) {
        oTop = GROUND_Y - 25;
        oBottom = GROUND_Y - 15;
      }

      if (pRight + 5 > oLeft && pLeft - 5 < oRight && pBottom > oTop &&
          pTop < oBottom) {
        // Only Game Over for non-destructible slabs (Type 0 and 1)
        if (obstacles[i].type != 2) {
          Audio_PlayCrash();
          Game_SetState(STATE_GAME_OVER);
        }
      }
      if (obstacles[i].x < -20)
        obstacles[i].active = false;
    }
  }
}

void Game_Render(void) {
  switch (currentState) {
  case STATE_MENU:
    ST7735_WriteString(10, 20, "NEON SURVIVOR", Font_11x18, ST7735_CYAN,
                       ST7735_BLACK);

    ST7735_WriteString(10, 50, "1:Easy 2:Norm 3:Hard", Font_7x10, ST7735_WHITE,
                       ST7735_BLACK);
    char diffStr[20];
    sprintf(diffStr, "Diff: %s",
            currentDifficulty == DIFF_EASY
                ? "EASY"
                : (currentDifficulty == DIFF_NORMAL ? "NORM" : "HARD"));
    ST7735_WriteString(10, 70, diffStr, Font_7x10, ST7735_YELLOW, ST7735_BLACK);
    ST7735_WriteString(10, 110, "Press * to START", Font_7x10, ST7735_GREEN,
                       ST7735_BLACK);
    break;

  case STATE_PAUSED:
    ST7735_WriteString(30, 50, "PAUSED", Font_11x18, ST7735_YELLOW,
                       ST7735_BLACK);
    ST7735_WriteString(20, 80, "Press * to RESUME", Font_7x10, ST7735_WHITE,
                       ST7735_BLACK);
    break;

  case STATE_GAME_OVER:
    ST7735_WriteString(20, 30, "CRASHED!", Font_11x18, ST7735_RED,
                       ST7735_BLACK);
    char scoreStr[20];
    sprintf(scoreStr, "Score: %lu", score);
    ST7735_WriteString(20, 60, scoreStr, Font_11x18, ST7735_WHITE,
                       ST7735_BLACK);
    ST7735_WriteString(10, 90, "Press * to MENU", Font_7x10, ST7735_GREEN,
                       ST7735_BLACK);
    ST7735_WriteString(10, 110, "Press 9: High Scores", Font_7x10, ST7735_CYAN,
                       ST7735_BLACK);
    break;

  case STATE_HIGH_SCORES:
    ST7735_WriteString(10, 10, "HIGH SCORES", Font_11x18, ST7735_YELLOW,
                       ST7735_BLACK);
    ST7735_WriteString(10, 40, "1. 950", Font_7x10, ST7735_WHITE, ST7735_BLACK);
    ST7735_WriteString(10, 55, "2. 720", Font_7x10, ST7735_WHITE, ST7735_BLACK);
    ST7735_WriteString(10, 70, "3. 450", Font_7x10, ST7735_WHITE, ST7735_BLACK);
    ST7735_WriteString(10, 110, "Press * for MENU", Font_7x10, ST7735_GREEN,
                       ST7735_BLACK);
    break;

  case STATE_RUNNING:
    RenderRunningGame();
    break;
  default:
    break;
  }
}

void RenderRunningGame(void) {
  // Show Score on 7-Seg
  TM1637_DisplayDecimal(score, 0);




  // --- UI: Speed & Energy ---
  ST7735_FillRectangle(18, 5, 84, 6, ST7735_WHITE);
  ST7735_FillRectangle(20, 7, 80, 2, ST7735_BLACK);

  // Speed color logic
  uint16_t speedColor = ST7735_GREEN;
  if (multiplier == 4)
    speedColor = ST7735_MAGENTA;
  else if (energy > 80)
    speedColor = ST7735_RED;
  else if (energy > 40)
    speedColor = ST7735_YELLOW;

  ST7735_FillRectangle(20, 7, (int)(energy * 0.8f), 2, speedColor);

  // Numeric Multiplier Display
  char speedStr[10];
  sprintf(speedStr, "x%.1f", Inputs.SpeedMultiplier);
  ST7735_WriteString(105, 5, speedStr, Font_7x10, speedColor, ST7735_BLACK);

  // 1. Erase
  ST7735_FillRectangle(PLAYER_X, (int)player.old_y - 22, PLAYER_W, 22,
                       ST7735_BLACK);
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (obstacles[i].was_active) {
      int oY = (obstacles[i].type == 0)
                   ? GROUND_Y - 30
                   : (obstacles[i].type == 2 ? GROUND_Y - 25 : GROUND_Y - 10);
      ST7735_FillRectangle((int)obstacles[i].old_x, oY, 12, 10, ST7735_BLACK);
    }
  }
  if (player.old_shooting) {
    int py = (int)player.old_y - (PLAYER_H / 2);
    ST7735_FillRectangle(PLAYER_X + PLAYER_W, py - 4,
                         ST7735_WIDTH - (PLAYER_X + PLAYER_W), 9, ST7735_BLACK);
  }

  // 2. Draw Player
  uint16_t pColor = (isShooting)
                        ? ST7735_WHITE
                        : (player.lane == LANE_L ? ST7735_BLUE : ST7735_CYAN);
  int cur_h = (multiplier == 4) ? 22 : PLAYER_H;
  int h = (player.state == SLIDING) ? cur_h / 2 : cur_h;
  ST7735_FillRectangle(PLAYER_X, (int)player.y - h, PLAYER_W, h, pColor);

  // 3. Draw Obstacles
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (obstacles[i].active) {
      uint16_t oColor = (multiplier == 4)
                            ? ST7735_MAGENTA
                            : (speed < 1.5f ? ST7735_GREEN : ST7735_RED);
      int oY = (obstacles[i].type == 0)
                   ? GROUND_Y - 30
                   : (obstacles[i].type == 2 ? GROUND_Y - 25 : GROUND_Y - 10);
      if (obstacles[i].type == 0)
        oColor = ST7735_YELLOW;
      else if (obstacles[i].type == 2)
        oColor = ST7735_WHITE;
      ST7735_FillRectangle((int)obstacles[i].x, oY, 10, 10, oColor);
    }
  }

  // 4. Draw Laser
  if (isShooting) {
    int py = (int)player.y - ((multiplier == 4 ? 22 : PLAYER_H) / 2);
    ST7735_FillRectangle(PLAYER_X + PLAYER_W, py,
                         ST7735_WIDTH - (PLAYER_X + PLAYER_W), 1, ST7735_GREEN);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4,
                      GPIO_PIN_SET); // Buzzer ON (Short blip)
  } else {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // Buzzer OFF

    if (player.old_shooting) {
      // Erase with "Safe" max height logic to ensure coverage?
      // Or just standard center. A little artifact is okay for now.
      int py = (int)player.old_y - (PLAYER_H / 2);
      // If we shrank, we might miss the old laser.
      // We'll stick to standard H for erase, or maybe erase a 5px band?
      // Let's erase a slightly taller block just in case
      ST7735_FillRectangle(PLAYER_X + PLAYER_W, py - 2,
                           ST7735_WIDTH - (PLAYER_X + PLAYER_W), 5,
                           ST7735_BLACK);
    }
  }
}
