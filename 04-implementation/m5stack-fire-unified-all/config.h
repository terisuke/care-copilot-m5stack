/**
 * Configuration file for Care Copilot M5Stack Fire
 * 
 * IMPORTANT: Copy this file to "config_local.h" and update with your settings
 * The config_local.h file will be ignored by git to keep your credentials safe
 */

#ifndef CONFIG_H
#define CONFIG_H

// WiFi Settings
#define WIFI_SSID "your-wifi-ssid"
#define WIFI_PASSWORD "your-wifi-password"

// MQTT Settings
// #define MQTT_SERVER "test.mosquitto.org"  // 元のサーバー（コメントアウト）
#define MQTT_SERVER "broker.hivemq.com"  // 代替サーバー1（最も安定）
// #define MQTT_SERVER "broker.emqx.io"   // 代替サーバー2
// #define MQTT_SERVER "mqtt.eclipse.org"  // 代替サーバー3
#define MQTT_PORT 1883
#define MQTT_USER ""  // Leave empty for public brokers
#define MQTT_PASS ""  // Leave empty for public brokers

// Device Configuration
#define DEVICE_ID "M5_FIRE_ALL_SENSORS"
#define FIRMWARE_VERSION "3.0.0"

// Alert Thresholds
#define FALL_THRESHOLD 2.5         // G-force threshold for fall detection
#define DISTANCE_NEAR 500          // mm - proximity warning
#define DISTANCE_FAR 2000          // mm - bed exit detection
#define DISTANCE_MAX 4000          // mm - maximum measurement distance
#define TEMP_HIGH 35.0             // °C - high temperature warning
#define TEMP_LOW 15.0              // °C - low temperature warning
#define HUMIDITY_HIGH 80.0         // % - high humidity warning
#define HUMIDITY_LOW 30.0          // % - low humidity warning

// Timing Settings (milliseconds)
#define IMU_READ_INTERVAL 100       // IMU sensor reading interval
#define TOF_READ_INTERVAL 200       // ToF sensor reading interval
#define ENV_READ_INTERVAL 2000      // Environmental sensor reading interval
#define GPS_READ_INTERVAL 1000      // GPS reading interval
#define MQTT_PUBLISH_INTERVAL 5000  // MQTT data publish interval
#define DISPLAY_UPDATE_INTERVAL 1000 // Display refresh interval
#define NO_MOTION_TIMEOUT 600000    // 10 minutes - no motion alert timeout

// GPS Configuration
#define GPS_BAUD_RATE 9600          // Default GPS baud rate
#define GPS_RX_PIN 16               // Port C RX pin
#define GPS_TX_PIN 17               // Port C TX pin

// I2C Addresses
#define TOF_I2C_ADDR 0x29           // ToF4M sensor
#define SHT4X_I2C_ADDR 0x44         // ENV.4 temperature/humidity sensor
#define BMP280_I2C_ADDR 0x76        // ENV.4 pressure sensor

// Display Settings
#define DISPLAY_TEXT_SIZE 2
#define DISPLAY_SMALL_TEXT_SIZE 1

// Debug Settings
#define DEBUG_SERIAL true           // Enable serial debug output
#define DEBUG_MQTT false            // Enable MQTT debug messages

#endif // CONFIG_H