# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is the Care Copilot M5Stack IoT system - an intelligent monitoring system for elderly care facilities and individual homes that prioritizes dignity and independence. The project is being developed for the M5Stack Global Innovation Contest 2025.

## Architecture

The system consists of three main components:

1. **M5Stack Firmware** (`04-implementation/m5stack-firmware.ino`)
   - ESP32-based IoT device with multi-sensor integration
   - Intelligent alert filtering based on time and context
   - MQTT communication for real-time data transmission
   - Local alert management with offline capabilities

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

- **Hardware**: M5Stack Core2, M5StickC Plus2, GPS Module, PIR sensor, Ultrasonic sensor, BME280 environmental sensor
- **Communication**: MQTT, WebSocket, REST API
- **Database**: PostgreSQL (time-series data), Redis (caching)
- **Notifications**: LINE Messaging API
- **Languages**: C++ (Arduino), JavaScript (Node.js), Python (AI/Analytics)

## Development Commands

### Arduino/M5Stack Development
```bash
# Use Arduino IDE 2.0+
# Board: ESP32 Arduino -> M5Stack-Core-ESP32
# Upload Speed: 921600
# Flash Frequency: 80MHz
```

### Backend Server
```bash
# Install dependencies
npm install

# Run development server (no nodemon configured yet)
node 04-implementation/backend-server.js

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
4. **Low Cost**: Total hardware cost under ¥20,000

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

Currently in MVP development phase for M5Stack Contest submission (deadline: 2025-08-22). The system is designed to demonstrate core functionality with real hardware integration.