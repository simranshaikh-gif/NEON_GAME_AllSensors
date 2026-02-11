# NEON GAME - All Sensors Project

This project is a comprehensive embedded system application combining an STM32 microcontroller for game logic and sensor processing, a DWIN HMI display for user interaction, and an ESP32 for a remote web dashboard.

## 📌 Project Overview

**NEON GAME** ("NEON RUSH") is an interactive game system that utilizes various sensors and actuators:
*   **Core Controller**: STM32F401RE
*   **IoT Dashboard**: ESP32 (received data via UART from STM32)
*   **Display**: DWIN HMI Display & TM1637 7-Segment Display
*   **Sensors**: TCS3200 Color Sensor, LDR (Light), Temperature, Touch Sensor.
*   **Input**: 4x4 Keypad, Slide Switch, Push Buttons.
*   **Feedback**: Buzzer, Relay, RGB LED, Indicator LEDs.

---

## 📂 Directory Structure

*   **`/NEON`**: Main STM32CubeIDE project (C Code). Contains all game logic, drivers, and Main.c.
*   **`/NEON_DASHBOARD`**: PlatformIO project for ESP32. Handles WiFi connection and serves the Web Dashboard.
*   **`/DWIN_NEON`**: DWIN DGUS II Project files (Icons, Images, Fonts) for the HMI display.

---

## 🛠 Hardware Architecture & Pinout

### 1. STM32F401RE (Main Controller)

**Displays & Indicators**
| Component | Pin | Function |
| :--- | :--- | :--- |
| **DWIN Display** | PA2 (TX), PA3 (RX) | UART2 Communication (115200 baud) |
| **TM1637 Display** | CLK=PB10, DIO=PB4 | 7-Segment Timer/Score Display |
| **RGB LED** | R=PC3, G=PC0, B=PC2 | Status Indication |
| **Discrete LEDs** | PC1 (LED1), PB0 (LED3), PA12 (LED4), PA5 (Onboard LD2) | Game Status LEDs |
| **Buzzer** | PA4 | Audio Feedback |
| **Relay** | PA1 | External Actuator Control |

**Sensors**
| Component | Pin | Function |
| :--- | :--- | :--- |
| **TCS3200 Color** | S0=PC10, S1=PC11, S2=PA7, S3=PA6, OE=PB6, OUT=PB4 | Color Detection |
| **Touch Sensor** | PC8 | User Input |
| **LDR** | PB1 | Ambient Light Sensing (ADC) |
| **Temperature** | (Internal/I2C) | Temperature Monitoring |

**Inputs**
| Component | Pin | Function |
| :--- | :--- | :--- |
| **Keypad (Rows)** | PA10, PB15, PB14, PB13 | Matrix Keypad Input |
| **Keypad (Cols)** | PB3, PC4, PB5 | Matrix Keypad Input |
| **Slide Switch** | PB8 | Mode Selection |
| **Push Buttons** | PC6 (UP), PC5 (DOWN) | Game Navigation |

### 2. ESP32 (Web Dashboard)

*   **Communication**: Receives JSON data from STM32 via **UART2** (Serial2 on ESP32 Pins 16/17).
    *   *Note: Ensure common ground between STM32 and ESP32.*
*   **WiFi**: Connects to configured SSID to server a web page.

---

## 🚀 Setup & Installation

### 1. STM32 Firmware (`/NEON`)
1.  Open the project in **STM32CubeIDE**.
2.  Build the project.
3.  Flash the `.elf` or `.bin` to the STM32F401RE board.

### 2. ESP32 Dashboard (`/NEON_DASHBOARD`)
1.  Open the project in **VS Code** with **PlatformIO**.
2.  Update `ssid` and `password` in `src/main.cpp` with your WiFi credentials.
3.  Upload the code to your ESP32 board.
4.  Open the Serial Monitor (115200 baud) to find the ESP32's **IP Address**.
5.  Enter the IP address in a web browser to view the dashboard.

### 3. DWIN Display (`/DWIN_NEON`)
1.  Copy the `DWIN_SET` folder to the root of a micro SD card.
2.  Insert the SD card into the DWIN display.
3.  Power cycle the display to update the firmware/assets.
4.  Remove SD card and restart.

---

## 🎮 How to Use

1.  **Power On**: Ensure all components (STM32, ESP32, Display) are powered.
2.  **Game Mode**: Use the Keypad or Slide Switch to select game modes.
3.  **Play**: Follow the on-screen instructions on the DWIN display.
4.  **Monitor**:
    *   **Local**: View scores on the TM1637 and status on DWIN.
    *   **Remote**: Open the ESP32 IP address on your phone/PC to see real-time "Temperature", "Luminosity", "Color", and "Game Status".

---

## ⚠️ Notes
*   **UART Conflict**: The STM32 sends data to both DWIN and ESP32 via `huart2` (PA2). Ensure the electrical connections (wiring) usually allow for a "Parallel" connection (STM32 TX -> DWIN RX & ESP32 RX) for monitoring, as the ESP32 only *reads* the data.
