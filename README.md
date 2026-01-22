# Dual-Chip IC & Component Tester (ATmega328p)

A universal electronics tester powered by **two** ATmega328p microcontrollers working in a Master/Slave configuration. This system combines a **Digital Logic IC Tester** (74LS series) and an **Analog Component Tester** (Resistors, Capacitors, Diodes, Transistors) into a single device with a shared LCD interface.

## 🧠 System Architecture

The device uses a unique **"Invisible Bridge" I2C Architecture** to allow two separate chips to share one display seamlessly.

*   **Chip A (Master U2) - Component Tester**:
    *   Controls the **16x2 I2C LCD** and **7-Segment Display**.
    *   Performs Analog measurements (Resistors, Caps, Transistors) using a **Resistor Ladder** network.
    *   Acts as the **I2C Bus Master**.
*   **Chip B (Slave U6) - IC Tester**:
    *   Controls the **ZIF Socket** for testing logic clips.
    *   Acts as an **I2C Slave (Address 8)**.
    *   When active, it sends test results (e.g., "IC: 7400") to the Master for display.

## ✨ Features

### 1. IC Tester Mode (Slave)
*   Tests **74LS Series Logic ICs**.
*   **Supported Chips**:
    *   Logic Gates: 7400 (NAND), 7402 (NOR), 7408 (AND), 7432 (OR), 7486 (XOR).
    *   Inverters: 7404 (Hex Inverter).
    *   Complex Logic: 7447 (BCD Decoder), 7490 (Counter), 74148 (Encoder), 74194 (Shift Reg).
*   **Automatic Pin Detection**: Shifts pinout for 14-pin vs 16-pin chips automatically.

### 2. Component Tester Mode (Master)
*   **7-Segment Display Tester**: Cycles 0-F using an onboard **74LS595 Shift Register**.
*   **Resistor Meter**: Measures resistance (10Ω - 500kΩ) via 680Ω voltage divider.
*   **Capacitance Meter**: Measures capacitance via RC time constant (470kΩ charge path).
*   **Diode/LED Tester**: Detects Diode orientation (Anode/Cathode) and Forward Voltage drop.
*   **Transistor Tester**: Detects NPN vs PNP Bipolar Junction Transistors (BJT).

## 🛠️ Hardware Setup

### Master Microcontroller (U2) Pinout
| Function | Arduino Pin | Physical Pin | Description |
| :--- | :--- | :--- | :--- |
| **Switch (Mode)** | VCC/GND | - | Switch toggles power between chips |
| **Button (Test)** | A3 | PC3 | Active LOW (Input Pullup) |
| **LCD (I2C)** | A4 / A5 | PC4 / PC5 | SDA / SCL (Addr 0x27) |
| **Shift Data** | D4 | PD4 | 74LS595 Data (SER) |
| **Shift Latch** | D5 | PD5 | 74LS595 Latch (RCLK) |
| **Shift Clock** | D6 | PD6 | 74LS595 Clock (SRCLK) |
| **TP1 (Sense)** | A0 | PC0 | Test Probe 1 Sense |
| **TP2 (Sense)** | A1 | PC1 | Test Probe 2 Sense |
| **TP3 (Sense)** | A2 | PC2 | Test Probe 3 Sense |
| **TP Drivers** | D8-D13 | PB0-PB5 | 680Ω / 470kΩ Resistor Ladder |

### Slave Microcontroller (U6) Pinout
| Function | Arduino Pin | Physical Pin | Description |
| :--- | :--- | :--- | :--- |
| **Button (Test)** | A3 | PC3 | Active LOW (Input Pullup) |
| **I2C (Output)** | A4 / A5 | PC4 / PC5 | Sends data to Master (Addr 8) |
| **ZIF Socket** | D2-D13, A0-A2 | - | Digitally driven test patterns |

---

## 🚀 Usage Guide

### 1. Burn the Firmware
*   Open `Component_Tester_v1_1/Component_Tester_v1_1.ino` and upload to the **Master Chip (U2)**.
*   Open `IC_Tester_v1_1/IC_Tester_v1_1.ino` and upload to the **Slave Chip (U6)**.

### 2. Testing Components (Resistors, LEDs, Transistors)
1.  Flip the **Toggle Switch** to **COMPONENTS**.
2.  The LCD will show `COMPONENT TESTER`.
3.  Insert your component into the female header holes (J2).
    *   **Resistor/LED**: Use Pin 1 and Pin 2.
    *   **Transistor**: Use Pin 1 (Emitter), Pin 2 (Base), Pin 3 (Collector).
4.  Press the **TEST** button.
5.  LCD displays the result (e.g., `Resistor 1-2: 1000R` or `NPN Transistor`).

### 3. Testing Logic ICs
1.  Flip the **Toggle Switch** to **IC**.
2.  The LCD (driven by the Master) will wait for data.
3.  Insert a 74LS series chip into the ZIF socket (Top aligned).
4.  Press the **TEST** button.
5.  LCD displays: `IC: 7400 Quad 2-NAND` (or similar).

---

## 📂 File Structure
*   **/Component_Tester_v1_1**: Source code for the Master Chip. Handles LCD, I2C Polling, and Analog Tests.
*   **/IC_Tester_v1_1**: Source code for the Slave Chip. Handles Digital Logic patterns and I2C Reporting.
*   **/Reference**: (Optional) Schematics or datasheets.

## ⚠️ Troubleshooting
*   **LCD Blank?** Check the I2C Address in code (`0x27`) and the contrast potentiometer.
*   **"Unknown IC"?** Ensure the chip is fully seated in the ZIF socket.
*   **Screen Glitches?** Ensure the Toggle Switch is providing clean power transitions.
