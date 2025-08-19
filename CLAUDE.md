# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is the Care Copilot M5Stack IoT system - an intelligent monitoring system for elderly care facilities and individual homes that prioritizes dignity and independence. The project is being developed for the M5Stack Global Innovation Contest 2025.

## Architecture

The system has two implementation patterns:

1. **Basic Version** (`04-implementation/m5stack-fire-basic/`)
   - M5Stack Fire with M5Unified library
   - IMU-based fall detection only
   - Minimal cost (¥8,000), prototype testing

2. **Unified All Version** (`04-implementation/m5stack-fire-unified-all/`)
   - IMU + ToF4M + ENV.4 + GPS all integrated
   - Port A: ToF4M + ENV.4 via I2C hub
   - Port C: GPS module
   - Full demonstration for M5Stack Contest 2025
   - Total cost: ¥20,000

2. **Backend Server** (`04-implementation/backend-server.js`)
   - Node.js/Express server with WebSocket support
   - PostgreSQL database for data persistence
   - Redis for caching and real-time state management
   - LINE Bot integration for notifications
   - AI-powered anomaly detection and alert filtering

3. **Frontend Dashboard** (web-based monitoring interface)
   - Real-time WebSocket updates
   - Multi-facility support
   - Analytics and QOL reporting

## Key Technologies

- **Hardware**: M5Stack Fire (IMU + LED Bar), Optional sensors (GPS, PIR, BME280)
- **Firmware**: M5Unified Library (unified API for M5Stack devices)
- **Communication**: MQTT, WebSocket, REST API
- **Database**: PostgreSQL (time-series data), Redis (caching)
- **Notifications**: LINE Messaging API
- **Languages**: C++ (Arduino + M5Unified), JavaScript (Node.js), Python (AI/Analytics)

## Development Commands

### Arduino/M5Stack Development
```bash
# Use Arduino IDE 2.0+
# Board Manager URL: https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json
# Board: M5Stack -> M5Stack-Fire
# Library: M5Unified (latest version)
# Upload Speed: 921600
# Flash Frequency: 80MHz
```

### Backend Server
```bash
# Install dependencies
cd 04-implementation
npm install

# Run development server
node backend-server.js

# For M5Stack Fire testing
# WiFi: 2.4GHz only (5GHz not supported)
# MQTT Broker: test.mosquitto.org (public test server)

# Environment variables needed in .env:
# - PORT
# - MQTT_BROKER
# - DB_HOST, DB_PORT, DB_NAME, DB_USER, DB_PASSWORD
# - REDIS_HOST, REDIS_PORT
# - LINE_CHANNEL_ACCESS_TOKEN, LINE_CHANNEL_SECRET
# - JWT_SECRET
```

### Python Analytics (if needed)
```bash
# Activate virtual environment
source venv/bin/activate  # Mac/Linux

# Install dependencies
pip install pandas numpy scikit-learn matplotlib requests
```

## Important Design Principles

1. **Privacy-First**: No cameras, dignity-preserving monitoring
2. **Alert Intelligence**: Reduces alert fatigue by 80% through context-aware filtering
3. **Independence Support**: Enables safe outdoor activities with GPS tracking
4. **Low Cost**: Basic version ¥8,000, Full version ¥20,000

## Hardware Configuration

### I2C Addresses (Port A with hub)
- ToF4M: 0x29
- SHT4X (ENV.4): 0x44
- BMP280 (ENV.4): 0x76

### UART (Port C)
- GPS: Auto-detect baud rate (115200/38400/9600/57600/4800)

## Alert Level System

- **Level 0 (INFO)**: Logged only, no notifications
- **Level 1 (CAUTION)**: Dashboard update only
- **Level 2 (WARNING)**: LINE notification to staff
- **Level 3 (EMERGENCY)**: Immediate multi-channel alerts

## Time-Based Filtering

The system adjusts sensitivity based on time of day:
- Night hours (22:00-06:00): Increased sensitivity, lower thresholds
- Day hours: Standard thresholds, reduced minor alerts

## Database Schema

Key tables:
- `facilities`: Care facilities
- `residents`: Individual residents with care levels
- `devices`: M5Stack devices and their assignments
- `sensor_data`: Time-series sensor readings
- `alerts`: Alert history with acknowledgment tracking
- `users`: Staff accounts with notification preferences

## MQTT Topics

- `care/+/data`: Sensor data from devices
- `care/+/alert`: Alert notifications
- `care/+/status`: Device heartbeat/status
- `care/command/+`: Commands to devices

## Project Status

MVP development phase for M5Stack Contest 2025 (deadline: 2025-08-22).
- ✅ Basic version: Working
- ✅ Unified All version: ALL SENSORS WORKING!
  - ToF: Distance measurement confirmed (1991mm)
  - ENV.4: Temperature/humidity working (31.3°C, 72.4%)
  - GPS: 13 satellites, location fixed (33.59348, 130.40200)
- ✅ Backend server: Ready for integration

## Recent Updates (2025/08/19)

- Successfully integrated all sensors in single firmware
- GPS auto baud rate detection working (115200 bps)
- I2C hub successfully managing multiple sensors
- Simplified to 2 clean implementations (Basic + Unified All)
- Contest-ready demonstration confirmed