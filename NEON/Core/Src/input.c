#include "input.h"
#include "keypad.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Bare Metal ADC Helper
// Channel 0 = PA0 (Pot)
// Channel 9 = PB1 (LDR) - Check Datasheet, PB1 is ADC1_IN9
static uint16_t ReadADC_BareMetal(uint8_t channel);
static float MLX90614_ReadTemp(void);

InputState_t Inputs; // Global Definition

void Input_Init(void) {
  Inputs.JumpCmd = false;
  Inputs.DuckCmd = false;
  Inputs.ShootCmd = false;
  Inputs.PauseCmd = false;
  Inputs.SpeedDial = 50;
  Inputs.Brightness = 100;
  Inputs.SpeedMultiplier = 1.0f;
}

void Input_Poll(void) {
  // 1. Digital Inputs
  bool touch = (HAL_GPIO_ReadPin(TOUCH_GPIO_Port, TOUCH_Pin) == GPIO_PIN_SET);
  bool btnTop =
      (HAL_GPIO_ReadPin(BTN_TOP_GPIO_Port, BTN_TOP_Pin) == GPIO_PIN_SET);
  bool btnBot =
      (HAL_GPIO_ReadPin(BTN_BOT_GPIO_Port, BTN_BOT_Pin) == GPIO_PIN_SET);
  bool btnSlide =
      (HAL_GPIO_ReadPin(BTN_SLIDE_GPIO_Port, BTN_SLIDE_Pin) == GPIO_PIN_SET);

  static char lastKey = 0;
  char currentKey = Keypad_GetKey(); // ENABLED
  // char currentKey = 0;

  Inputs.KeyPressed = currentKey;
  if (currentKey != 0 && lastKey == 0) {
    Inputs.KeyJustPressed = currentKey;
    Inputs.NewKeyAvailable = true;
  } else {
    Inputs.KeyJustPressed = 0;
    Inputs.NewKeyAvailable = false;
  }
  lastKey = currentKey;

  // Map Keypad to Game Actions (Workaround for PC6 Conflict)
  if (currentKey == '2')
    btnTop = true; // '2' = Jump
  if (currentKey == '8')
    btnBot = true; // '8' = Duck

  Inputs.JumpCmd = btnTop;
  Inputs.DuckCmd = btnSlide || btnBot;
  Inputs.ShootCmd = touch;

  if (Inputs.KeyJustPressed == '#' || Inputs.KeyJustPressed == '*') {
    Inputs.PauseCmd = !Inputs.PauseCmd;
  }

  // 2. Analog Inputs (Bare Metal)
  uint16_t rawPot = ReadADC_BareMetal(0); // ADC1_IN0 (PA0)
  uint16_t rawLdr = ReadADC_BareMetal(9); // ADC1_IN9 (PB1)
  // uint16_t rawTemp = ReadADC_BareMetal(8); // ADC1_IN8 (PB0) - REMOVED

  // --- Software Filtering (Low Pass Filter) ---
  static float filteredPot = -1.0f;
  static float filteredLdr = -1.0f;
  const float alpha = 0.1f; // Original alpha

  // Initialize filters on first run
  if (filteredPot < 0) {
    filteredPot = (float)rawPot;
    filteredLdr = (float)rawLdr;
  }

  // Update Filters
  filteredPot = (alpha * (float)rawPot) + ((1.0f - alpha) * filteredPot);
  filteredLdr = (alpha * (float)rawLdr) + ((1.0f - alpha) * filteredLdr);

  // Read Digital Temp
  float mlxTemp = MLX90614_ReadTemp();
  if (mlxTemp < -100.0f)
    mlxTemp = 25.0f; // Error fallback

  // Final Scaled Values for DWIN
  // Temp: Float Celsius -> Int * 10 (25.5 -> 255)
  Inputs.Temperature = (int)(mlxTemp * 10.0f);

  Inputs.SpeedDial = ((uint16_t)filteredPot * 1000) / 4095;
  Inputs.Brightness = ((uint16_t)filteredLdr * 1000) / 4095;

  Inputs.SpeedMultiplier = 0.5f + ((float)rawPot / 4095.0f) * 2.5f;

  // Serial Logging REMOVED for DWIN Dashboard compatibility
}

// --- Open-Drain Soft I2C Driver for MLX90614 ---
#define SDA_PORT GPIOC
#define SDA_PIN GPIO_PIN_9
#define SCL_PORT GPIOA
#define SCL_PIN GPIO_PIN_8

#define SDA_H HAL_GPIO_WritePin(SDA_PORT, SDA_PIN, GPIO_PIN_SET)
#define SDA_L HAL_GPIO_WritePin(SDA_PORT, SDA_PIN, GPIO_PIN_RESET)
#define SCL_H HAL_GPIO_WritePin(SCL_PORT, SCL_PIN, GPIO_PIN_SET)
#define SCL_L HAL_GPIO_WritePin(SCL_PORT, SCL_PIN, GPIO_PIN_RESET)
#define SDA_READ (HAL_GPIO_ReadPin(SDA_PORT, SDA_PIN) == GPIO_PIN_SET)

// Delay for I2C (~100kHz equivalent)
static void I2C_Delay(void) {
  for (volatile int i = 0; i < 200; i++)
    __NOP();
}

static void I2C_Start(void) {
  SDA_H;
  SCL_H;
  I2C_Delay();
  SDA_L;
  I2C_Delay();
  SCL_L;
  I2C_Delay();
}

static void I2C_Stop(void) {
  SDA_L;
  I2C_Delay();
  SCL_H;
  I2C_Delay();
  SDA_H;
  I2C_Delay();
}

static uint8_t I2C_WriteByte(uint8_t data) {
  for (int i = 0; i < 8; i++) {
    if (data & 0x80)
      SDA_H;
    else
      SDA_L;
    data <<= 1;
    I2C_Delay();
    SCL_H;
    I2C_Delay();
    SCL_L;
    I2C_Delay();
  }
  // ACK Check
  SDA_H; // Release SDA
  I2C_Delay();
  SCL_H;
  I2C_Delay();
  uint8_t ack = !SDA_READ;
  SCL_L;
  I2C_Delay();
  return ack;
}

static uint8_t I2C_ReadByte(uint8_t ack) {
  uint8_t data = 0;
  SDA_H; // Release SDA
  for (int i = 0; i < 8; i++) {
    data <<= 1;
    SCL_H;
    I2C_Delay();
    if (SDA_READ)
      data |= 1;
    SCL_L;
    I2C_Delay();
  }
  // Send ACK/NACK
  if (ack)
    SDA_L;
  else
    SDA_H;
  I2C_Delay();
  SCL_H;
  I2C_Delay();
  SCL_L;
  I2C_Delay();
  SDA_H; // Release
  return data;
}

static void I2C_BusClear(void) {
  // Toggle SCL 9 times to unstick slaves
  SDA_H;
  for (int i = 0; i < 9; i++) {
    SCL_H;
    I2C_Delay();
    SCL_L;
    I2C_Delay();
  }
  I2C_Start();
  I2C_Stop();
}

static float MLX90614_ReadTemp(void) {
  static bool firstRun = true;
  if (firstRun) {
    I2C_BusClear();
    firstRun = false;
  }

  int retries = 3;
  while (retries--) {
    I2C_Start();
    if (!I2C_WriteByte(0xB4)) { // 0x5A<<1 | 0 (Write)
      I2C_Stop();
      continue;
    }
    if (!I2C_WriteByte(0x07)) { // Command 0x07 (TOBJ1)
      I2C_Stop();
      continue;
    }

    I2C_Start(); // Ref Start
    if (!I2C_WriteByte(
            0xB5)) { // 0x5A<<1 | 1 (Read) // Fixed: 0xB5 is Read Address
      I2C_Stop();
      continue;
    }

    uint8_t low = I2C_ReadByte(1);  // ACK
    uint8_t high = I2C_ReadByte(1); // ACK
    uint8_t pec = I2C_ReadByte(0);  // NACK
    I2C_Stop();

    (void)pec;
    uint16_t raw = (uint16_t)high << 8 | low;

    // Check for error values usually found in noise
    if (raw == 0xFFFF || raw == 0)
      return -4.0f;

    return (float)raw * 0.02f - 273.15f;
  }

  // Try clearing bus if failed
  I2C_BusClear();
  return -2.0f; // Communication Failed (Timeout)
}

static uint16_t ReadADC_BareMetal(uint8_t channel) {
  // 1. Select Channel in SQR3
  // Clear SQR3 entirely for single-conversion mode
  ADC1->SQR3 = 0;
  // Set channel in first sequence position
  ADC1->SQR3 = (channel & 0x1F);

  // Ensure ADC is ON
  ADC1->CR2 |= ADC_CR2_ADON;

  // Settlement delay
  for (volatile int i = 0; i < 500; i++)
    ;

  // Set original sample time (144 cycles)
  if (channel <= 9) {
    ADC1->SMPR2 &= ~(7 << (channel * 3));
    ADC1->SMPR2 |= (5 << (channel * 3));
  } else {
    ADC1->SMPR1 &= ~(7 << ((channel - 10) * 3));
    ADC1->SMPR1 |= (5 << ((channel - 10) * 3));
  }

  // Start Conversion
  ADC1->CR2 |= ADC_CR2_SWSTART;

  // Wait for EOC
  uint32_t timeout = 10000;
  while (!(ADC1->SR & ADC_SR_EOC)) {
    if (--timeout == 0)
      return 0;
  }

  return (uint16_t)(ADC1->DR);
}
