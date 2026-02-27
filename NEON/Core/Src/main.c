
/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : NEON RUSH - Main Program Body
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "audio.h"
#include "dwin.h"
#include "game.h"
#include "input.h"
#include "keypad.h"
#include "st7735.h" // Added for ST7735_SPI_Init if needed
#include "tcs3200.h"
#include "tm1637.h"
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern GameState_t currentState;
extern uint32_t highScore;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6; // Added for ESP32 Dashboard (PC6/PC7)

/* USER CODE BEGIN PV */
typedef struct {
  uint8_t Red;
  uint8_t Green;
  uint8_t Blue;
  uint8_t ColorID;
} ColorState_t;

ColorState_t Color;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
// static void MX_USART1_UART_Init(void); // Removed
static void MX_USART2_UART_Init(void);
// static void MX_USART6_UART_Init(void); // Removed
static void MX_SPI1_Init(void);
static void MX_SPI3_Init(void);
static void MX_ADC1_Init(void);

// ...

// static void MX_USART1_UART_Init(void); // REMOVED

/**
 * @brief USART6 Initialization Function (PC6=TX, PC7=RX)
 * @param None
 * @retval None
 */
/**
 * @brief USART6 Initialization Function (PC6=TX, PC7=RX) - REMOVED
 */
// static void MX_USART6_UART_Init(void) { ... }

// (Code Moved to MX_GPIO_Init)

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == TCS_OUT_Pin) {
    TCS3200_PulseCallback();
  }
}

int _write(int file, char *ptr, int len) {
  HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}
/* USER CODE END 4 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_SPI3_Init();
  MX_ADC1_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  MX_ADC1_Init(); // Safe inside User Code 2 block
  // Initialize Subsystems
  printf("System Init Starting...\r\n");

  printf("Init TM1637...\r\n");
  TM1637_Init(); // New Physical Display

  printf("Init TCS3200...\r\n");
  TCS3200_Init();

  printf("Init Input...\r\n");
  Input_Init();

  printf("Init Keypad...\r\n");
  Keypad_Init();

  printf("Init Game...\r\n");
  Game_Init();

  // printf("Init ESP32 Connection (USART6)...\r\n");
  // MX_USART6_UART_Init(); // Removed

  printf("System Init Complete. Entering Loop.\r\n");

  // --- FORCE ALL LEDS ON (User Request) ---
  HAL_GPIO_WritePin(GPIOC, LED1_Pin | RGB_R_Pin | RGB_G_Pin | RGB_B_Pin,
                    GPIO_PIN_SET);                             // LED1 + RGB
  HAL_GPIO_WritePin(GPIOB, LED2_Pin | LED3_Pin, GPIO_PIN_SET); // LED2 + LED3
  HAL_GPIO_WritePin(GPIOA, LED4_Pin, GPIO_PIN_SET); // LED4 (LD2 removed)

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // --- MAIN GAME LOOP ---

    // 1. Read Inputs
    // Input reading happens inside Game_Update via Input_Update()

    // 2. Update Game Logic
    Game_Update(); // ENABLED: No Conflicts!
    // Input_Poll(); // Removed duplicate call (Done inside Game_Update)

    Audio_Update(); // Handle Buzzer/Relay durations

    // 3. Render Game
    Game_Render(); // ENABLED: No Conflicts!

    // 4. DWIN Dashboard Update (FINAL INTEGRATION)
    static uint32_t lastDwinTick = 0;
    if (HAL_GetTick() - lastDwinTick > 200) { // 5Hz update rate
      lastDwinTick = HAL_GetTick();

      // --- ESP32 JSON Dashboard Update ---
      char jsonBuf[200]; // Create a storage buffer for the text
      const char *statusStr = (currentState == STATE_RUNNING)     ? "Running"
                              : (currentState == STATE_GAME_OVER) ? "GameOver"
                                                                  : "Idle";

      // Create the JSON string
      int len =
          snprintf(jsonBuf, sizeof(jsonBuf),
                   "{\"temp\":%.1f,\"lux\":%d,\"r\":%d,\"g\":%d,\"b\":%d,"
                   "\"st\":\"%s\"}\n",
                   (float)Inputs.Temperature / 10.0f, (int)Inputs.Brightness,
                   Color.Red, Color.Green, Color.Blue, statusStr);

      // Transmit it over UART2 (the same one DWIN uses)
      HAL_UART_Transmit(&huart2, (uint8_t *)jsonBuf, len, 100);

      // --- Sensor Dashboard Update (Optimized) ---
      // Read colors ONLY when NOT running (removes 120ms lag spike during play)
      if (currentState != STATE_RUNNING) {
        TCS3200_GetRGB(&Color.Red, &Color.Green, &Color.Blue);

        // Determine ColorID based on current RGB
        if (Color.Red > 150 && Color.Green < 100 && Color.Blue < 100)
          Color.ColorID = 1; // RED
        else if (Color.Green > 150 && Color.Red < 100 && Color.Blue < 100)
          Color.ColorID = 2; // GREEN
        else if (Color.Blue > 150 && Color.Red < 100 && Color.Green < 100)
          Color.ColorID = 3; // BLUE
        else
          Color.ColorID = 0;
      }

      // Always send sensor data to DWIN (Temp/Light are fast)
      DWIN_Update(Inputs.Temperature, Inputs.Brightness, Color.Red * 10,
                  Color.Green * 100, Color.Blue * 10, Color.ColorID, highScore);
    }

    // 6. Slow Serial Debug (MONITOR MODE)
    static uint32_t lastDebugTick = 0;
    if (HAL_GetTick() - lastDebugTick > 1000) {
      lastDebugTick = HAL_GetTick();
      printf("[MONITOR] T:%d.%d L:%d.%d RGB:(%d,%d,%d) ID:%d\r\n",
             Inputs.Temperature / 10, Inputs.Temperature % 10,
             Inputs.Brightness / 10, Inputs.Brightness % 10, Color.Red,
             Color.Green, Color.Blue, Color.ColorID);
    }

    // 5. Game Speed Control
    HAL_Delay(50); // ~20 FPS for high-speed response

    // Heartbeat (LD2 Removed)

    // (Dashboard Logic Removed for Revert)
    // ---------------------------------------------------------
  }

  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void) {

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler =
      SPI_BAUDRATEPRESCALER_16; // Slowed down for stability
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */
}

/**
 * @brief SPI3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI3_Init(void) {
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler =
      SPI_BAUDRATEPRESCALER_128; // Start slow for SD init
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /* --- INITIAL STATES --- */
  HAL_GPIO_WritePin(GPIOA, BUZZER_Pin | LED1_Pin | TFT_DC_Pin | SEG_CLK_Pin,
                    GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB,
                    TFT_CS_Pin | LED2_Pin | LED3_Pin | RL2_Pin | BTN3_Pin,
                    GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOC,
                    TFT_RST_Pin | LED4_Pin | RGB_R_Pin | RGB_G_Pin | RGB_B_Pin |
                        SEG_DIO_Pin,
                    GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, RL1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin,
                    GPIO_PIN_SET); // SD CS Idle High

  // Move on to other pins

  /*Configure GPIO pin : LED4_Pin */
  GPIO_InitStruct.Pin = LED4_Pin | SD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Set SD_CS_Pin HIGH */
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);

  /* --- PORT B --- */
  // Outputs: TFT_CS, LED2, LED3, RL2, TCS_OE, Key_C1, Key_C3, BTN3
  GPIO_InitStruct.Pin = TFT_CS_Pin | LED2_Pin | LED3_Pin | RL2_Pin |
                        TCS_OE_Pin | KEY_C1_Pin | KEY_C3_Pin | BTN3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // Inputs: Keypad Rows 2, 3, 4 (PB15, PB14, PB13)
  GPIO_InitStruct.Pin = KEY_R2_Pin | KEY_R3_Pin | KEY_R4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* --- PORT A (Remaining) --- */
  // Outputs: TCS_S2, TCS_S3, TFT_DC, BUZZER, LED1 (Push-Pull)
  GPIO_InitStruct.Pin =
      TCS_S2_Pin | TCS_S3_Pin | TFT_DC_Pin | BUZZER_Pin | LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Output: SEG_CLK (Open-Drain for shared bus)
  GPIO_InitStruct.Pin = SEG_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Analog: Rotation (PA0), Temp (PB0), Light (PB1)
  GPIO_InitStruct.Pin = ROTATION_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = TEMP_Pin | LIGHT_Pin;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // Input: Keypad R1 (PA10)
  GPIO_InitStruct.Pin = KEY_R1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KEY_R1_Port, &GPIO_InitStruct);

  /* --- PORT C --- */
  // Outputs: TFT_RST, LED4, RGB_R, RGB_G, RGB_B, KEY_C2, TCS_S0, TCS_S1
  // (Push-Pull)
  GPIO_InitStruct.Pin = TFT_RST_Pin | LED4_Pin | RGB_R_Pin | RGB_G_Pin |
                        RGB_B_Pin | KEY_C2_Pin | TCS_S0_Pin | TCS_S1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  // Output: SEG_DIO (Open-Drain for shared bus)
  GPIO_InitStruct.Pin = SEG_DIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  // Inputs: Touch Sensor, Button 2, Button 4
  GPIO_InitStruct.Pin = TOUCH_Pin | BTN2_Pin | BTN4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* --- PORT D --- */
  // Output: TCS_OE (replacing PB7 with PB2 per manual)
  GPIO_InitStruct.Pin = TCS_OE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // Input: TCS_OUT (PB4) Interrupt mode
  GPIO_InitStruct.Pin = TCS_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // Outputs: RL1 (PD2)
  GPIO_InitStruct.Pin = RL1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* NVIC */
  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
/**
 * @brief ADC1 Initialization Function (Bare Metal)
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {
  // 1. Enable ADC1 Clock
  __HAL_RCC_ADC1_CLK_ENABLE();

  // 2. Configure ADC1 (Simple Single Conversion)
  // Reset CR1 and CR2
  ADC1->CR1 = 0;
  ADC1->CR2 = 0;

  // Set Prescaler (PCLK2 / 4) in CCR
  ADC->CCR = 0x00010000; // ADCPRE = 01 (Div 4)

  // Configure Sampling Time for stability (480 cycles for high impedance)
  // Channel 0 (PA0), Channel 8 (PB0), Channel 9 (PB1)
  // SMP0 [2:0], SMP8 [26:24], SMP9 [29:27]
  ADC1->SMPR2 |= (0x7 << 0);  // Ch 0 -> 480 cycles
  ADC1->SMPR2 |= (0x7 << 24); // Ch 8 -> 480 cycles
  ADC1->SMPR2 |= (0x7 << 27); // Ch 9 -> 480 cycles

  // 3. Enable ADC
  ADC1->CR2 |= ADC_CR2_ADON;

  // 4. Wait for stabilization
  HAL_Delay(10);
}

/* USER CODE BEGIN 4 */
int __io_putchar(int ch) {
  // HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
