#include "keypad.h"

// Keypad logic uses direct pin states or a internal map

void Keypad_Init(void) {
  // GPIO Init is handled in main.c MX_GPIO_Init
  // Columns are Outputs, Rows are Inputs with Pull-up
  // Initial state: Columns High (Inactive)
  HAL_GPIO_WritePin(KEY_C1_Port, KEY_C1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(KEY_C2_Port, KEY_C2_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(KEY_C3_Port, KEY_C3_Pin, GPIO_PIN_SET);
}

char Keypad_GetKey(void) {
  // Scan Columns
  // Col 1
  HAL_GPIO_WritePin(KEY_C1_Port, KEY_C1_Pin, GPIO_PIN_RESET);
  for (volatile int i = 0; i < 50; i++); // Settlement delay
  if (HAL_GPIO_ReadPin(KEY_R1_Port, KEY_R1_Pin) == GPIO_PIN_RESET) {
    HAL_GPIO_WritePin(KEY_C1_Port, KEY_C1_Pin, GPIO_PIN_SET);
    return '1';
  }
  if (HAL_GPIO_ReadPin(KEY_R2_Port, KEY_R2_Pin) == GPIO_PIN_RESET) {
    HAL_GPIO_WritePin(KEY_C1_Port, KEY_C1_Pin, GPIO_PIN_SET);
    return '4';
  }
  if (HAL_GPIO_ReadPin(KEY_R3_Port, KEY_R3_Pin) == GPIO_PIN_RESET) {
    HAL_GPIO_WritePin(KEY_C1_Port, KEY_C1_Pin, GPIO_PIN_SET);
    return '7';
  }
  if (HAL_GPIO_ReadPin(KEY_R4_Port, KEY_R4_Pin) == GPIO_PIN_RESET) {
    HAL_GPIO_WritePin(KEY_C1_Port, KEY_C1_Pin, GPIO_PIN_SET);
    return '*';
  }
  HAL_GPIO_WritePin(KEY_C1_Port, KEY_C1_Pin, GPIO_PIN_SET);

  // Col 2
  HAL_GPIO_WritePin(KEY_C2_Port, KEY_C2_Pin, GPIO_PIN_RESET);
  for (volatile int i = 0; i < 50; i++); // Settlement delay
  if (HAL_GPIO_ReadPin(KEY_R1_Port, KEY_R1_Pin) == GPIO_PIN_RESET) {
    HAL_GPIO_WritePin(KEY_C2_Port, KEY_C2_Pin, GPIO_PIN_SET);
    return '2';
  }
  if (HAL_GPIO_ReadPin(KEY_R2_Port, KEY_R2_Pin) == GPIO_PIN_RESET) {
    HAL_GPIO_WritePin(KEY_C2_Port, KEY_C2_Pin, GPIO_PIN_SET);
    return '5';
  }
  if (HAL_GPIO_ReadPin(KEY_R3_Port, KEY_R3_Pin) == GPIO_PIN_RESET) {
    HAL_GPIO_WritePin(KEY_C2_Port, KEY_C2_Pin, GPIO_PIN_SET);
    return '8';
  }
  if (HAL_GPIO_ReadPin(KEY_R4_Port, KEY_R4_Pin) == GPIO_PIN_RESET) {
    HAL_GPIO_WritePin(KEY_C2_Port, KEY_C2_Pin, GPIO_PIN_SET);
    return '0';
  }
  HAL_GPIO_WritePin(KEY_C2_Port, KEY_C2_Pin, GPIO_PIN_SET);

  // Col 3
  HAL_GPIO_WritePin(KEY_C3_Port, KEY_C3_Pin, GPIO_PIN_RESET);
  for (volatile int i = 0; i < 50; i++); // Settlement delay
  if (HAL_GPIO_ReadPin(KEY_R1_Port, KEY_R1_Pin) == GPIO_PIN_RESET) {
    HAL_GPIO_WritePin(KEY_C3_Port, KEY_C3_Pin, GPIO_PIN_SET);
    return '3';
  }
  if (HAL_GPIO_ReadPin(KEY_R2_Port, KEY_R2_Pin) == GPIO_PIN_RESET) {
    HAL_GPIO_WritePin(KEY_C3_Port, KEY_C3_Pin, GPIO_PIN_SET);
    return '6';
  }
  if (HAL_GPIO_ReadPin(KEY_R3_Port, KEY_R3_Pin) == GPIO_PIN_RESET) {
    HAL_GPIO_WritePin(KEY_C3_Port, KEY_C3_Pin, GPIO_PIN_SET);
    return '9';
  }
  if (HAL_GPIO_ReadPin(KEY_R4_Port, KEY_R4_Pin) == GPIO_PIN_RESET) {
    HAL_GPIO_WritePin(KEY_C3_Port, KEY_C3_Pin, GPIO_PIN_SET);
    return '#';
  }
  HAL_GPIO_WritePin(KEY_C3_Port, KEY_C3_Pin, GPIO_PIN_SET);

  return 0; // No key
}
