# IoT System for Fire and Gas Detection with Real-Time Alerting

## Introduction
This project is an intelligent environmental monitoring system designed to detect hazardous conditions (fire and gas leaks) in real-time and provide multi-channel alerts. Built with PlatformIO using the Arduino framework, the system is architected on FreeRTOS for robust multitasking and real-time operation. Each functional module is implemented as a separate RTOS task, enabling efficient parallel processing and responsive threat detection. The core of the system uses a TinyML model (Random Forest) trained on real sensor data to classify environmental states into three categories: Normal, Gas Leak, and Fire. Inter-task communication is handled through semaphores and queues, ensuring reliable synchronization and data exchange between sensor acquisition, AI inference, alerting, and cloud synchronization components. The project demonstrates practical integration of machine learning on edge devices, real-time IoT monitoring, and modular software design for smart safety systems.

## System Architecture

The system consists of three main layers:

1. **Edge Device (ESP32-S3)**: Collects sensor data, runs TinyML model, broadcasts via local WebSocket.
2. **Local Server (PC)**: Receives data from ESP32, stores in SQLite database, provides REST API and WebSocket dashboard.
3. **Cloud Platforms**: Integrates with ThingsBoard (CoreIOT) and Telegram Bot for remote monitoring and alerts.

## Project Structure

```
Multi_dis_project/
│
├── include/
│   ├── collect_data.h           # Data collection task (labeled dataset)
│   ├── coreiot.h                # ThingsBoard/CoreIOT integration
│   ├── global.h                 # Global variables, queues, semaphores
│   ├── led_blinky.h             # LED indicator task header
│   ├── neo_blinky.h             # NeoPixel LED status display header
│   ├── project_includes.h       # Project-wide includes and macros
│   ├── random_forest_model.h    # TinyML Random Forest model (C/C++ header)
│   ├── task_check_info.h        # System info checking task header
│   ├── task_handler.h           # Main task handler (important)
│   ├── task_optimize.h          # Power optimization task header
│   ├── task_telegram.h          # Telegram Bot alerting task header
│   ├── task_webserver.h         # Local web server task header
│   ├── task_wifi.h              # WiFi & network management header
│   └── temp_humi_monitor.h      # Temperature/humidity monitoring & AI inference
│
├── src/
│   ├── collect_data.cpp         # Data collection with state machine
│   ├── coreiot.cpp              # ThingsBoard MQTT client implementation
│   ├── global.cpp               # Global variables & queue/semaphore init
│   ├── led_blinky.cpp           # LED blinking task implementation
│   ├── main.cpp                 # Main entry point (important)
│   ├── neo_blinky.cpp           # NeoPixel animation based on sensor state
│   ├── task_check_info.cpp      # System diagnostics task
│   ├── task_handler.cpp         # Main task spawner & coordinator
│   ├── task_optimize.cpp        # Power saving & optimization logic
│   ├── task_telegram.cpp        # Telegram Bot message sending
│   ├── task_webserver.cpp       # AsyncWebServer with WebSocket broadcast
│   ├── task_wifi.cpp            # WiFi connection & Internet sync
│   └── temp_humi_monitor.cpp    # Sensor reading + Random Forest inference + multi-platform publish
│
├── model/
│   ├── data/
│   │   ├── create_csv.ipynb           # Dataset generation notebook
│   │   ├── fire_dataset.csv           # Labeled training dataset
│   │   └── testing_set.csv            # Test set for validation
│   │
│   ├── notebook/
│   │   ├── compare_models.ipynb       # ML model comparison (SVM, DT, RF, etc.)
│   │   ├── main_model.ipynb           # Primary model training pipeline
│   │   └── random_forest.ipynb        # Random Forest model training & export
│   │
│   └── output/
│       └── random_forest_model.h      # Exported C/C++ header for ESP32 embedding
│
├── server/
│   ├── app.py                   # FastAPI backend (receives ESP32 data via WebSocket)
│   ├── requirements.txt         # Python dependencies
│   ├── run.bat                  # Batch script to start server
│   ├── sensor_data.db           # SQLite database (auto-created, stores all sensor readings)
│   │
│   └── static/
│       ├── index.html           # Dashboard web UI
│       ├── script.js            # WebSocket client + real-time visualization
│       └── styles.css           # Dashboard styling
│
├── README.md                    # This file
├── platformio.ini               # PlatformIO project configuration
│
└── [Other config files]
```

## System's Features

### 1. **Multi-Sensor Environmental Monitoring**
- **DHT20 Sensor**: Temperature (°C) and humidity (%) measurement
- **MQ-135 Gas Sensor**: Air quality and gas concentration detection (0-4095 ADC)
- **Real-time Sampling**: 1-second interval data acquisition

### 2. **TinyML-Based Fire & Gas Detection**
- **Random Forest Model**: 15 decision trees, 98% accuracy on test set
- **Input Features**: temperature, humidity, smoke_value, temp_diff, gas_diff
- **Output Classes**: 0=Normal, 1=Gas Leak, 2=Fire
- **Inference**: <5ms latency, <100KB memory footprint
- **Model Export**: C/C++ header via `micromlgen` for embedded deployment

### 3. **Real-Time Alert System**
- **Telegram Bot**: Instant notifications with sensor readings when hazards detected
- **NeoPixel LEDs**: Visual status indicators (Normal, Gas Leak, Fire)
- **Semaphore-based Triggering**: Synchronized event handling across tasks

### 4. **Multi-Platform Data Publishing**
- **ThingsBoard (CoreIOT)**: MQTT telemetry streaming with remote RPC control
- **Local PC Database**: SQLite storage via WebSocket client (auto-prunes to 100,000 records)
- **REST API**: Endpoints for latest reading, history queries, and statistics

### 5. **Modular RTOS Architecture**
- **10+ Independent Tasks**: Sensor reading, AI inference, alerting, web server, WiFi, database sync
- **Inter-task Communication**: Semaphores & queues for safe data exchange
- **Real-time Guarantees**: FreeRTOS scheduling ensures responsive threat detection

## Getting Started

### Prerequisites
- ESP32-S3 development board
- DHT20 & MQ-135 sensors
- Python 3.8+ (for PC server)
- PlatformIO IDE or CLI

### Setup Instructions

1. **Prepare ESP32 Firmware**:
   ```bash
   cd d:\main\Multi_dis_project
   pio run -t upload  # Compile and flash to ESP32
   ```

2. **Start Local Server**:
   ```bash
   cd server
   .\run.bat  # Windows batch script
   ```

3. **Access Dashboard**:
   ```
   http://localhost:8000
   ```

4. **Configure Settings** (via ESP32 web UI):
   ```
   http://<ESP32_IP>:80
   ```
   - WiFi SSID/Password
   - ThingsBoard token & server
   - Telegram Bot token

## Acknowledgments
- TinyML framework using `micromlgen` for model export
- ThingsBoard open-source IoT platform
- FreeRTOS real-time kernel
- AsyncWebServer & WebSocket libraries