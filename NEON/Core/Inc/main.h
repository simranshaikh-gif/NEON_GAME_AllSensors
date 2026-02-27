/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
// --- TFT Display (SPI1) ---
#define TFT_CS_Pin GPIO_PIN_6
#define TFT_CS_GPIO_Port GPIOB
#define TFT_DC_Pin GPIO_PIN_9
#define TFT_DC_GPIO_Port GPIOA
#define TFT_RST_Pin GPIO_PIN_7
#define TFT_RST_GPIO_Port GPIOC
// SCK=PA5, MOSI=PA7 (SPI1 Pins)

// --- 7-Segment Display (TM1637) ---
#define SEG_DIO_Pin GPIO_PIN_9
#define SEG_DIO_GPIO_Port GPIOC
#define SEG_CLK_Pin GPIO_PIN_8
#define SEG_CLK_GPIO_Port GPIOA

// --- Color Sensor (TCS3200) ---
#define TCS_S0_Pin GPIO_PIN_11
#define TCS_S0_GPIO_Port GPIOC
#define TCS_S1_Pin GPIO_PIN_12
#define TCS_S1_GPIO_Port GPIOC
#define TCS_S2_Pin GPIO_PIN_11
#define TCS_S2_GPIO_Port GPIOA
#define TCS_S3_Pin GPIO_PIN_12
#define TCS_S3_GPIO_Port GPIOA
#define TCS_OE_Pin GPIO_PIN_2
#define TCS_OE_GPIO_Port GPIOB
#define TCS_OUT_Pin GPIO_PIN_4
#define TCS_OUT_GPIO_Port GPIOB

// --- Sensors & Inputs ---
#define ROTATION_Pin GPIO_PIN_0
#define ROTATION_GPIO_Port GPIOA // Potentiometer
#define LIGHT_Pin GPIO_PIN_1
#define LIGHT_GPIO_Port GPIOB // LDR
#define TEMP_Pin GPIO_PIN_0
#define TEMP_GPIO_Port GPIOB // Temperature
#define TOUCH_Pin GPIO_PIN_8
#define TOUCH_GPIO_Port GPIOC // Switch OUT

// --- Push Buttons ---
#define BTN1_Pin GPIO_PIN_8
#define BTN1_GPIO_Port GPIOB
#define BTN2_Pin GPIO_PIN_6
#define BTN2_GPIO_Port GPIOC
#define BTN3_Pin GPIO_PIN_9
#define BTN3_GPIO_Port GPIOB
#define BTN4_Pin GPIO_PIN_5
#define BTN4_GPIO_Port GPIOC

// Alias for Game Actions
#define BTN_UP_Pin BTN2_Pin
#define BTN_UP_GPIO_Port BTN2_GPIO_Port
#define BTN_DOWN_Pin BTN4_Pin
#define BTN_DOWN_GPIO_Port BTN4_GPIO_Port

// --- Matrix Keypad (4x3) ---
#define KEY_R1_Pin GPIO_PIN_10
#define KEY_R1_Port GPIOA
#define KEY_R2_Pin GPIO_PIN_15
#define KEY_R2_Port GPIOB
#define KEY_R3_Pin GPIO_PIN_14
#define KEY_R3_Port GPIOB
#define KEY_R4_Pin GPIO_PIN_13
#define KEY_R4_Port GPIOB

#define KEY_C1_Pin GPIO_PIN_3
#define KEY_C1_Port GPIOB
#define KEY_C2_Pin GPIO_PIN_4
#define KEY_C2_Port GPIOC
#define KEY_C3_Pin GPIO_PIN_5
#define KEY_C3_Port GPIOB

// --- Audio & Relays ---
#define BUZZER_Pin GPIO_PIN_4
#define BUZZER_GPIO_Port GPIOA
#define RL1_Pin GPIO_PIN_2
#define RL1_GPIO_Port GPIOD
#define RL2_Pin GPIO_PIN_12
#define RL2_GPIO_Port GPIOB

// --- Status LEDs ---
#define LED1_Pin GPIO_PIN_1
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_10
#define LED2_GPIO_Port GPIOB
#define LED3_Pin GPIO_PIN_0
#define LED3_GPIO_Port GPIOB
#define LED4_Pin GPIO_PIN_1
#define LED4_GPIO_Port GPIOC

// RGB (Discrete)
#define RGB_R_Pin GPIO_PIN_3
#define RGB_R_GPIO_Port GPIOC
#define RGB_G_Pin GPIO_PIN_0
#define RGB_G_GPIO_Port GPIOC
#define RGB_B_Pin GPIO_PIN_2
#define RGB_B_GPIO_Port GPIOC

// --- SD Card (SPI3) ---
#define SD_CS_Pin GPIO_PIN_15
#define SD_CS_GPIO_Port GPIOA
#define SD_SCK_Pin GPIO_PIN_10
#define SD_SCK_GPIO_Port GPIOC
#define SD_MISO_Pin GPIO_PIN_11
#define SD_MISO_GPIO_Port GPIOC
#define SD_MOSI_Pin GPIO_PIN_12
#define SD_MOSI_GPIO_Port GPIOC

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
