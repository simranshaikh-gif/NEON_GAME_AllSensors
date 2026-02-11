#include "audio.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include <stdbool.h>

// Simple Non-Blocking Audio State
static uint32_t tone_end_time = 0;
static uint32_t click_end_time = 0;
static bool tone_active = false;
static bool click_active = false;

void Audio_Init(void) {
  // Pins initialized in main.c (BUZZER_Pin, RELAY_Pin)
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_RESET);
}

void Audio_Update(void) {
  uint32_t now = HAL_GetTick();

  // 1. Handle Tone (Square Wave Bit-Bang)
  // Note: Ideally use a Timer PWM, but for simple beeps this is portable.
  // Since we are in the main loop, we can't bit-bang high freq efficiently
  // without blocking. ALTERNATIVE: Use a simple "Duration" check. For "Arcade
  // Style", a short blocking beep (10ms) is often acceptable, but we can try to
  // toggle only if tone_active.

  if (tone_active) {
    if (now >= tone_end_time) {
      tone_active = false;
      HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
    } else {
      // Very rough software PWM (Not ideal for main loop, but works for
      // "noise") We just toggle it every frame to make a "grating" noise, or
      // leave it High. A pure High on a piezoelectric buzzer usually clicks. A
      // square wave needs a timer.
      HAL_GPIO_TogglePin(BUZZER_GPIO_Port, BUZZER_Pin);
    }
  }

  // 2. Handle Relay Click (Haptic)
  if (click_active) {
    if (now >= click_end_time) {
      click_active = false;
      HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_RESET);
    }
  }
}

void Audio_PlayJump(void) {
  // Short Beep (50ms)
  tone_active = true;
  tone_end_time = HAL_GetTick() + 50;
}

void Audio_PlayDuck(void) {
  // Tiny blip (20ms)
  tone_active = true;
  tone_end_time = HAL_GetTick() + 20;
}

void Audio_PlayCrash(void) {
  // Long Noise (500ms)
  tone_active = true;
  tone_end_time = HAL_GetTick() + 500;

  // Physical Click
  Audio_HapticClick();
}

void Audio_HapticClick(void) {
  HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_SET);
  click_active = true;
  click_end_time = HAL_GetTick() + 50; // 50ms pulse
}
