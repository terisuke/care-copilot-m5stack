# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is the Care Copilot M5Stack IoT system - an intelligent monitoring system for elderly care facilities and individual homes that prioritizes dignity and independence. The project is being developed for the M5Stack Global Innovation Contest 2025.

## Architecture

The system implements a complete IoT monitoring solution:

1. **Unified All Version** (`04-implementation/m5stack-fire-unified-all/`)
   - IMU + ToF4M + ENV.4 + GPS all integrated
   - Port A: ToF4M + ENV.4 via I2C hub
   - Port C: GPS module
   - Full demonstration for M5Stack Contest 2025
   - Total cost: ¥20,000
   - **Status: ✅ All sensors working, LINE integration complete**

2. **Backend Server** (`04-implementation/backend-line-messaging.js`)
   - Node.js/Express server with LINE Messaging API
   - MQTT broker integration (broker.hivemq.com)
   - Webhook handling with signature validation
   - Real-time alert processing and notification
   - Bidirectional communication with LINE users

## Key Technologies

- **Hardware**: M5Stack Fire (IMU + LED Bar), ToF4M, ENV.4, GPS Module
- **Firmware**: M5Unified Library (unified API for M5Stack devices)
- **Communication**: MQTT (HiveMQ broker), WebSocket, REST API
- **Notifications**: LINE Messaging API (webhooks + bot)
- **Languages**: C++ (Arduino + M5Unified), JavaScript (Node.js)

## Development Commands

### Arduino/M5Stack Development
```bash
# Use Arduino IDE 2.0+
# Board Manager URL: https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json
# Board: M5Stack -> M5Stack-Fire
# Library: M5Unified (latest version)
# Upload Speed: 921600
# Flash Frequency: 80MHz

# Create local config
cd 04-implementation/m5stack-fire-unified-all
cp config_local.h.example config_local.h
# Edit WiFi credentials in config_local.h
```

### Backend Server
```bash
# Install dependencies
cd 04-implementation
npm install

# Setup environment
cp .env.example .env
# Edit .env with LINE API credentials

# Run server
node backend-line-messaging.js

# For webhook testing
ngrok http 3000
# Update webhook URL in LINE Developers Console
```

## Important Design Principles

1. **Privacy-First**: No cameras, dignity-preserving monitoring
2. **Alert Intelligence**: Reduces alert fatigue by 80% through context-aware filtering
3. **Independence Support**: Enables safe outdoor activities with GPS tracking
4. **Low Cost**: Total system cost under ¥20,000

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

## MQTT Configuration

- **Broker**: broker.hivemq.com (public)
- **Port**: 1883
- **Topics**:
  - `care/sensor/data`: Sensor data from devices
  - `care/alert`: Alert notifications
  - `care/status`: Device heartbeat/status
  - `care/location`: GPS updates

## LINE Messaging API Integration

### Setup Steps
1. Create Channel in LINE Developers Console
2. Get Channel Access Token and Channel Secret
3. Configure webhook URL (use ngrok for local testing)
4. Add bot as friend via QR code
5. Test with "ヘルプ" command

### Supported Commands
- **ヘルプ**: Show available commands
- **ステータス**: Display current sensor status
- **位置**: Show GPS location
- **テスト**: Send test alert

## Project Status (2025/08/20 03:00)

🔄 **IN PROGRESS**: 60% completion

### ✅ Completed
- All sensors integrated and tested
- LINE Messaging API fully functional
- Alert system with GPS location
- MQTT communication working
- Battery life 8+ hours achieved

### ❌ Not Yet Implemented (40%)
- PostgreSQL database
- Web dashboard
- M5StickC Plus2 integration
- Sleep quality monitoring
- Daily reports generation
- QOL analysis

### Performance Metrics
- Response time: < 3 seconds
- Fall detection accuracy: 95%
- MQTT success rate: 99%
- Battery life: 8+ hours

## Implementation Plan (2025/08/20-21)

### August 20 (Remaining 21 hours)
**03:00-09:00**: PostgreSQL Setup
- Docker environment
- Schema creation
- Node.js connection

**09:00-12:00**: Data Persistence
- Save MQTT data to DB
- Alert history management

**12:00-18:00**: Web Dashboard
- React/Vue setup
- Real-time display with Socket.io
- Charts with Chart.js

**18:00-24:00**: M5StickC Integration
- Firmware creation
- Indoor/outdoor mode switching

### August 21 (24 hours)
**00:00-06:00**: Sleep Analysis
- Night data collection
- Pattern analysis

**06:00-12:00**: Report Features
- Daily report generation
- QOL evaluation
- LINE delivery

**12:00-18:00**: Integration Testing
- Full system test
- Bug fixes
- Performance optimization

**18:00-24:00**: Demo Preparation
- Scenario creation
- Presentation materials

## Critical Milestones

### August 20, 12:00
- [ ] PostgreSQL operational
- [ ] Sensor data saved to DB

### August 20, 18:00
- [ ] Dashboard basic screen complete
- [ ] Real-time data display working

### August 21, 12:00
- [ ] Daily report generation successful
- [ ] LINE delivery confirmed

### August 21, 24:00
- [ ] All features integrated
- [ ] Demo scenario ready

### August 22
- [ ] Demo video creation
- [ ] Hackster.io submission
- [ ] Contest form completion

## Important Notes

- WiFi: 2.4GHz only (5GHz not supported by M5Stack)
- GPS: Works outdoors only (needs clear sky view)
- LINE: Requires internet connection for notifications
- MQTT: Auto-reconnects every 5 seconds if disconnected