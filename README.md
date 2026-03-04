# Smart room monitoring and controlling system

## Introduction
This project is a smart room monitoring and controlling system designed to manage and automate various intelligent devices within a room. Developed using PlatformIO with the Arduino framework, the system is architected on FreeRTOS, enabling robust multitasking and real-time operation. Each functional module is implemented as a separate RTOS task, allowing efficient parallel processing. Inter-task communication is handled through semaphores and queues, ensuring reliable synchronization and data exchange between components. The project demonstrates practical integration of IoT devices, real-time control, and modular software design for smart environments.

## Structure of include and src folders
```
Logic_Design_Project/
│
......
|
├── include/
│   ├── coreiot.h                # Core system definitions
│   ├── global.h                 # Global variables and settings
│   ├── led_blinky.h             # LED control task header
│   ├── neo_blinky.h             # NeoPixel LED task header
│   ├── project_includes.h       # Project-wide includes
│   ├── task_check_info.h        # Info checking task header
│   ├── task_handler.h           # Main task handler (important)
│   ├── task_optimize.h          # Optimization task header
│   ├── task_webserver.h         # Web server task header
│   ├── task_wifi.h              # WiFi management task header
│   ├── task1_human_detection.h  # Human detection task header
│   └── temp_humi_monitor.h      # Temperature & humidity monitor
│
├── src/
│   ├── coreiot.cpp              # Core system logic
│   ├── global.cpp               # Global variables implementation
│   ├── led_blinky.cpp           # LED control task
│   ├── main.cpp                 # Main entry point (important)
│   ├── neo_blinky.cpp           # NeoPixel LED task
│   ├── task_check_info.cpp      # Info checking task
│   ├── task_handler.cpp         # Main task handler (important)
│   ├── task_optimize.cpp        # Optimization logic
│   ├── task_webserver.cpp       # Web server implementation
│   ├── task_wifi.cpp            # WiFi management
│   ├── task1_human_detection.cpp  # Human detection logic
│   └── temp_humi_monitor.cpp    # Temperature & humidity monitor
|
......
|
└──platformio.ini                # Project configuration file for PlatformIO
```

## Report & Demo videos
Google Drive Links:
- Report: [Click here to view](https://drive.google.com/file/d/1yi8DWGoU1KzVPDwOEIt7yzzesaamX2iz/view?usp=sharing)
- Demo videos: [Click here to view](https://drive.google.com/drive/folders/1MqlARrh2gpE3AqRm6Oz_yvyIyh0CAHSH)

## System's features
- **NEO Pixel Light Control**: The system controls NEO Pixel LEDs, allowing users to customize colors and lighting modes. Configuration and control are accessible via a web interface hosted directly on the ESP32, enabling real-time adjustments from any device on the network.
- **Temperature & Humidity Monitoring**: Utilizes the DHT20 sensor to measure temperature and humidity. Sensor data is sent to both the web server and CoreIOT (ThingsBoard), providing live updates and intuitive visualization on dashboards for users.
- **CoreIOT (ThingsBoard) Integration**:The device connects to CoreIOT (ThingsBoard) for remote monitoring and control. All key data (sensor readings, human presence, device status) is synchronized, and remote commands can be executed from the platform.
- **Human Presence Detection**: Employs the LD2410 sensor to detect human presence in the room. Detection results are transmitted to both the web server and CoreIOT, supporting real-time monitoring and enabling further automation.
- **Energy Optimization**: Implements a Power Optimize mode that leverages human presence data. When no presence is detected, the system enters light sleep mode to conserve energy, automatically resuming normal operation when someone is detected.




