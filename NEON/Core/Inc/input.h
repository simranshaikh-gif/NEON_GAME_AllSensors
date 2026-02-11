#ifndef INC_INPUT_H_
#define INC_INPUT_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  // Digital Actions
  bool JumpCmd;  // From Top Button or Keypad '2'
  bool DuckCmd;  // From Slide Button, Bottom Button, or Keypad '8'
  bool PauseCmd; // From Keypad '#' or '*'
  bool ShootCmd; // From Touch Sensor

  // Analog Modifiers (0-100 or specific range)
  uint16_t SpeedDial;    // From Potentiometer (0-1000 for 0.1% res)
  uint16_t Brightness;   // From LDR (0-1000 for 0.1% res)
  float SpeedMultiplier; // Dynamically calculated (0.6x to 1.5x)

  // Complex Inputs
  char KeyPressed;      // Current key being held
  int Temperature;      // Raw or Scaled
  char KeyJustPressed;  // First frame a key is pressed
  bool NewKeyAvailable; // Flag for scanning logic

  // Debug/Raw
  uint16_t AdcPotRaw;  // PA0
  uint16_t AdcLdrRaw;  // PB1
  uint16_t AdcTempRaw; // PB0
} InputState_t;

extern InputState_t Inputs;

void Input_Init(void);
void Input_Poll(void);

#endif /* INC_INPUT_H_ */
