# 🔧 ENGR 478 - Lab 5 Project: Interrupt-Driven LED Control and 2-Bit Counter

This repository contains embedded C programs for **Lab 5** of ENGR 478 – *Design with Microprocessors* at San Francisco State University. The lab focuses on GPIO control, timer-based scheduling, edge-triggered external interrupts, and stateful logic using the STM32L476 Nucleo-64 board.

## 📁 Project Structure

- **Lab5_project/** – LED blinking using SysTick timer (Part A)
- **Switch_LED_delay_Interrupt/** – Button-controlled LED toggling using EXTI and Timer 6 for debounce (Part B)
- **Switch_LED_Counter/** – 2-bit rotary counter using SW1 (increment) and SW2 (decrement)
- **.gitignore** – Ignores build output, backup files, and temporary Keil project files

---

## 🛠️ Features

### ✅ Part A – LED Blinking with SysTick
- Blinks an LED every 0.5 seconds using the **SysTick timer interrupt**
- Demonstrates periodic scheduling without blocking the main loop

### ✅ Part B – EXTI with Timer-Based Debounce
- **SW1 (PC2)** triggers an **rising edge interrupt** (pull-down logic)
- **SW2 (PC3)** triggers a **falling edge interrupt** (pull-up logic)
- Both buttons are debounced using **Timer 6**
- Each switch toggles its corresponding LED

### ✅ Extra – 2-Bit Rotary Counter
- Implements a 2-bit counter with **cyclical behavior (0 to 3)**
- **SW1 increments**, **SW2 decrements**
- LED1 displays **bit 0 (LSB)**, LED2 displays **bit 1 (MSB)**
- Uses edge-triggered interrupts and debouncing to ensure reliable state updates

---

## ⚙️ Hardware Configuration

- **SW1 → PC2 (increment):**
  - Connected to 3.3V through a pull-down resistor
  - Rising-edge triggered

- **SW2 → PC3 (decrement):**
  - Connected to GND through a pull-up resistor
  - Falling-edge triggered

- **LED1 → PB4:**
  - Anode to PB4, cathode through resistor to GND (positive logic)

- **LED2 → PB5:**
  - Cathode to PB5, anode through resistor to 3.3V (negative logic)

---

## 🔌 Development Environment

- **Microcontroller:** STM32L476RG (Nucleo-64)
- **IDE:** Keil uVision5
- **Debugger:** ST-Link/V2
- **Language:** Embedded C

---

## 📄 License

This project is intended for **educational and instructional use** in ENGR 478.  
Not licensed for commercial use or redistribution.

---

## 🙋 Authors

**The Nucleo Team** – ENGR 478 Spring 2025  
- Nhan Nguyen  
- Vi Gallegos

🔗 GitHub Repository: [https://github.com/nhannguyensf/lab5-engr478](https://github.com/nhannguyensf/lab5-engr478)
