# 🏠 Automated Home Automation System Based on ESP32

A complete IoT smart home system based on the ESP32, featuring real-time monitoring, intelligent automation, a live web dashboard, GSM SMS alerts, and SD card data logging.

---

## 📌 Overview

This project implements a functional **smart home automation system** using the **ESP32 microcontroller**.  
It integrates several sensors and actuators to monitor the environment, automate window and lighting control, detect anomalies, and notify the user.

The system provides:

- Real-time measurements  
- Automatic decision-making  
- Web-based dashboard  
- SMS alerts via GSM  
- Local LCD display  
- Data logging on SD card  

---

## 🚀 Features

### 🔍 Monitoring
- Inside & outside temperature (DHT11)  
- Light level detection (LDR)  
- Electrical current measurement (ACS712)  
- Door open/close detection (magnetic sensor)

### ⚙️ Automation
- Servo motor control for window automation  
- Automatic LED lighting based on ambient light  
- Alarm (buzzer) for anomalies or intrusion  
- Temperature-based decision system  

### 🌐 Connectivity
- Wi-Fi web dashboard built with HTML/CSS/JS  
- Live charts using Chart.js  
- JSON data endpoint `/data`  
- GSM SMS alerts (SIM800L)

### 🖥️ Local Display
- LCD 16x2 (I2C) showing live temperature, light level, and current

### 💾 Data Logging
- Measurements stored on a MicroSD card  
- Dashboard loads historical logs directly  

---

## 🛠️ Hardware Used

- ESP32 development board  
- 2× DHT11 sensors  
- LDR light sensor  
- ACS712 current sensor  
- Magnetic door sensor (reed switch)  
- Micro Servo motor  
- SIM800L GSM module  
- MicroSD card reader  
- LCD 16x2 I2C  
- Buzzer & LED  

---

## 💻 Software & Technologies

**Embedded Software**
- Arduino/C++  
- WiFi.h  
- WebServer.h  
- DHT.h  
- ESP32Servo.h  
- SD.h  
- LiquidCrystal_I2C.h  

**Web Dashboard**
- HTML5  
- CSS3 / Bootstrap  
- JavaScript  
- Chart.js  

**Tools**
- Cirkit Designer  
- Visual Studio Code  
- Arduino IDE  

---


---

## 🧠 System Architecture

- ESP32 reads all sensors  
- Executes automation rules  
- Controls servo motor, LED, and buzzer  
- Logs data to SD card  
- Hosts Wi-Fi web dashboard  
- Sends alerts through GSM module  

---

## 📈 Dashboard

- Live temperature, current, and light graphs  
- Auto-refreshing interface  
- Loads historical logs from SD card  
- Clean, responsive layout  

---

## 📬 SMS Alerts

The system sends an SMS when:

- Door is opened  
- Intrusion is detected  
- Temperature difference is abnormal  
- Current consumption is unusual  

---

## 🎯 Objectives

- Create a complete and functional home automation prototype  
- Integrate embedded systems + electronics + IoT + web development  
- Provide real-time monitoring and intelligent automation  
- Improve home comfort, energy efficiency, and security  

---

## 📜 License

This project is open-source and free to use for educational and personal purposes.


