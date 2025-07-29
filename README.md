# Vibration Anomaly Detection Using TinyML

This project implements a **real-time vibration anomaly detection system** using an **autoencoder neural network** deployed on an **Arduino Nano 33 BLE Sense Rev2** with an **MPU6050 accelerometer**. A secondary **LCD via Arduino Uno** displays anomaly status.

---

## Overview

- **Goal:** Detect abnormal vibration patterns using machine learning on-device
- **Board:** Arduino Nano 33 BLE Sense Rev2
- **Sensor:** MPU6050 (Accelerometer + Gyro)
- **Display:** I2C LCD connected via Arduino Uno (separate 5V supply)
- **Model:** Trained Autoencoder converted to `.tflite` and `.h` formats
- **Inference:** Performed using TensorFlow Lite for Microcontrollers
- **Output:** Anomaly status displayed on LCD and/or via serial monitor

---

## Purpose

- Monitor vibration behavior of rotating machinery (like a fan)
- Detect unusual patterns using unsupervised machine learning (autoencoder)
- Run everything **locally** on a microcontroller (no cloud or server)
- Explore capabilities of **TinyML** for industrial IoT applications

---

## How It Works

1. **Sensor Data Collection**
   - The MPU6050 collects acceleration data on X, Y, Z axes at ~200 Hz.
   - 200 samples per axis are collected in a buffer.

2. **Feature Extraction**
   - Compute **MAD (Median Absolute Deviation)** for each axis.
   - These 3 values represent vibration variability.

3. **Anomaly Detection**
   - MAD values are fed into a trained **autoencoder model**.
   - The model reconstructs the input and outputs new MAD values.
   - **MSE (Mean Squared Error)** is calculated between input and output.
   - If MSE > threshold → anomaly is detected.

4. **Display / Alert**
   - Anomaly status is printed on an LCD via an Arduino Uno.
   - Optionally output to serial monitor or buzzer.

---

## Hardware Required

| Component               | Description                          |
|-------------------------|--------------------------------------|
| Arduino Nano 33 BLE Sense Rev2 | Main MCU running ML model        |
| MPU6050 Module          | 3-axis accelerometer                 |
| Arduino Uno             | Drives 5V LCD display                |
| I2C LCD (16x2 or 20x4)  | Displays "Anomaly Detected" status  |
| Small Fan (or vibration source) | To generate real vibration      |
| Common GND connection   | Shared GND between Nano and Uno      |
| Jumper wires            | For connections                      |

---

## Software Used

- Arduino IDE
- TensorFlow Lite for Microcontrollers
- `MPU6050_light` library (not Adafruit)
- LCD I2C library (`LiquidCrystal_I2C`)
- Python (for model training, if you retrain)

---

## Connections

### MPU6050 to Nano 33 BLE Sense
| MPU6050 Pin | Nano Pin     |
|-------------|--------------|
| VCC         | 3.3V         |
| GND         | GND          |
| SDA         | A4 (SDA)     |
| SCL         | A5 (SCL)     |

### LCD to Arduino Uno (5V Side)
| LCD Pin | Uno Pin    |
|---------|------------|
| VCC     | 5V         |
| GND     | GND        |
| SDA     | A4         |
| SCL     | A5         |

### Nano ↔ Uno Serial Bridge (Optional)
- Nano TX → Uno RX
- Nano GND ↔ Uno GND (Shared ground is **required**)

---

## References
Edge AI anomaly detection - DigiKey
