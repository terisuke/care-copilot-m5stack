/**
 * Configuration Template for Care Copilot M5Stack Firmware
 * 
 * Instructions:
 * 1. Copy this file and rename it to "config.h"
 * 2. Fill in your actual configuration values
 * 3. Include "config.h" in the main firmware file instead of hardcoded values
 * 4. Add config.h to .gitignore to keep your credentials private
 */

#ifndef CONFIG_H
#define CONFIG_H

// Device Configuration
#define DEVICE_ID "M5_CARE_001"  // Unique identifier for this device

// WiFi Configuration
#define WIFI_SSID "YOUR_WIFI_SSID"        // Your WiFi network name
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD" // Your WiFi password

// MQTT Configuration
#define MQTT_SERVER "192.168.1.100"     // MQTT broker IP or hostname
#define MQTT_PORT 1883                  // MQTT port (1883 for non-SSL, 8883 for SSL)
#define MQTT_USER ""                    // MQTT username (leave empty if not required)
#define MQTT_PASSWORD ""                // MQTT password (leave empty if not required)
#define MQTT_CLIENT_ID "m5stack-care"   // MQTT client identifier

// Alert Thresholds
#define MOTION_TIMEOUT_MS 300000        // 5 minutes no motion = alert
#define FALL_DETECTION_THRESHOLD 150    // cm/s acceleration threshold
#define TEMP_HIGH_THRESHOLD 28.0        // Temperature too high (°C)
#define TEMP_LOW_THRESHOLD 18.0         // Temperature too low (°C)
#define HUMIDITY_HIGH_THRESHOLD 70      // Humidity too high (%)
#define HUMIDITY_LOW_THRESHOLD 30       // Humidity too low (%)

// Timing Configuration
#define SENSOR_READ_INTERVAL 10000      // Read sensors every 10 seconds
#define DATA_SEND_INTERVAL 60000        // Send data every minute
#define HEARTBEAT_INTERVAL 300000       // Send heartbeat every 5 minutes

// GPS Configuration (if using external GPS)
#define GPS_BAUD_RATE 9600
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17

// Debug Configuration
#define DEBUG_SERIAL true               // Enable serial debug output
#define DEBUG_DISPLAY true              // Show debug info on display

#endif // CONFIG_H