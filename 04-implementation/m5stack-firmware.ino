/**
 * Care Copilot - M5Stack Firmware
 * Version: 1.0.0
 * Date: 2025-08-16
 * 
 * Main firmware for M5Stack Basic management terminal
 * Features:
 * - Multi-sensor integration
 * - Intelligent alert filtering
 * - WiFi/MQTT communication
 * - Local alert management
 */

#include <M5Stack.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <TinyGPSPlus.h>
#include <Preferences.h>

// ============================================
// Configuration
// ============================================
#define DEVICE_ID "M5_CARE_001"
#define FIRMWARE_VERSION "1.0.0"

// WiFi Configuration
// IMPORTANT: Replace these with your actual WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";        // Your WiFi network name
const char* password = "YOUR_WIFI_PASSWORD"; // Your WiFi password

// MQTT Configuration 
// IMPORTANT: Replace these with your actual MQTT broker details
const char* mqtt_server = "your-mqtt-server.com";  // MQTT broker address (e.g., "192.168.1.100" or "broker.mqtt.com")
const int mqtt_port = 1883;                        // MQTT port (default: 1883, SSL: 8883)
const char* mqtt_user = "care_copilot";            // MQTT username (leave empty if not required)
const char* mqtt_password = "secure_password";     // MQTT password (leave empty if not required)

// Topics
const char* TOPIC_SENSOR_DATA = "care/sensor/data";
const char* TOPIC_ALERT = "care/alert";
const char* TOPIC_STATUS = "care/status";
const char* TOPIC_COMMAND = "care/command";

// Pins Configuration
#define PIR_PIN 36          // PIR Motion Sensor
#define ULTRASONIC_TRIG 26  // Ultrasonic Trigger
#define ULTRASONIC_ECHO 35  // Ultrasonic Echo
#define LED_PIN 2           // Status LED
#define BUZZER_PIN 25       // Alert Buzzer

// Sensor Thresholds
#define TEMP_MIN 18.0       // Minimum temperature (°C)
#define TEMP_MAX 28.0       // Maximum temperature (°C)
#define HUMIDITY_MIN 40.0   // Minimum humidity (%)
#define HUMIDITY_MAX 70.0   // Maximum humidity (%)
#define DISTANCE_THRESHOLD 150  // Distance threshold (cm)
#define MOTION_TIMEOUT 300000   // Motion timeout (5 minutes)

// ============================================
// Global Variables
// ============================================
WiFiClient espClient;
PubSubClient mqtt(espClient);
Adafruit_BME280 bme;
TinyGPSPlus gps;
HardwareSerial GPS_Serial(2);
Preferences preferences;

// System State
struct SystemState {
    bool wifiConnected = false;
    bool mqttConnected = false;
    bool sensorsReady = false;
    uint8_t alertLevel = 0;  // 0: Normal, 1: Info, 2: Warning, 3: Emergency
    unsigned long lastMotion = 0;
    unsigned long lastHeartbeat = 0;
    float batteryLevel = 100.0;
} systemState;

// Sensor Data
struct SensorData {
    bool motion = false;
    float distance = 0.0;
    float temperature = 0.0;
    float humidity = 0.0;
    float pressure = 0.0;
    double latitude = 0.0;
    double longitude = 0.0;
    uint32_t timestamp = 0;
} currentData;

// Alert Management
enum AlertLevel {
    ALERT_NONE = 0,
    ALERT_INFO = 1,
    ALERT_WARNING = 2,
    ALERT_EMERGENCY = 3
};

struct Alert {
    AlertLevel level;
    String message;
    String type;
    uint32_t timestamp;
};

// ============================================
// Setup Functions
// ============================================
void setup() {
    // Initialize M5Stack
    M5.begin(true, false, true, true);
    M5.Power.begin();
    
    // Initialize Serial
    Serial.begin(115200);
    Serial.println(F("Care Copilot Starting..."));
    
    // Initialize Display
    setupDisplay();
    
    // Initialize Storage
    preferences.begin("care-copilot", false);
    loadSettings();
    
    // Initialize Pins
    pinMode(PIR_PIN, INPUT);
    pinMode(ULTRASONIC_TRIG, OUTPUT);
    pinMode(ULTRASONIC_ECHO, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    
    // Initialize Sensors
    setupSensors();
    
    // Initialize GPS
    GPS_Serial.begin(9600, SERIAL_8N1, 16, 17);
    
    // Initialize Network
    setupWiFi();
    setupMQTT();
    
    // Initialize Timers
    setupTimers();
    
    // System Ready
    systemState.sensorsReady = true;
    displayStatus("System Ready");
    playStartupSound();
}

void setupDisplay() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Care Copilot v1.0");
    M5.Lcd.setTextSize(1);
}

void setupSensors() {
    Wire.begin();
    
    // Initialize BME280 (Temperature, Humidity, Pressure)
    if (!bme.begin(0x76)) {
        Serial.println(F("BME280 sensor not found!"));
        displayError("Sensor Error");
    } else {
        Serial.println(F("BME280 initialized"));
    }
    
    // Configure BME280
    bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                    Adafruit_BME280::SAMPLING_X2,
                    Adafruit_BME280::SAMPLING_X16,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::FILTER_X16,
                    Adafruit_BME280::STANDBY_MS_0_5);
}

void setupWiFi() {
    displayStatus("Connecting WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        systemState.wifiConnected = true;
        Serial.println(F("\nWiFi connected"));
        Serial.print(F("IP address: "));
        Serial.println(WiFi.localIP());
        displayStatus("WiFi Connected");
    } else {
        displayError("WiFi Failed");
    }
}

void setupMQTT() {
    mqtt.setServer(mqtt_server, mqtt_port);
    mqtt.setCallback(mqttCallback);
    mqtt.setBufferSize(1024);
    reconnectMQTT();
}

void setupTimers() {
    // Setup periodic tasks using ESP32 hardware timers
    // Timer for sensor reading (every 1 second)
    // Timer for heartbeat (every 30 seconds)
    // Timer for data upload (every 5 minutes)
}

// ============================================
// Main Loop
// ============================================
void loop() {
    // Update M5Stack buttons
    M5.update();
    
    // Handle button events
    handleButtons();
    
    // Read sensors
    readSensors();
    
    // Process alerts
    processAlerts();
    
    // Update display
    updateDisplay();
    
    // Handle network
    if (!mqtt.connected()) {
        reconnectMQTT();
    }
    mqtt.loop();
    
    // Send periodic updates
    sendPeriodicUpdates();
    
    // Check battery
    checkBattery();
    
    // Small delay to prevent CPU overload
    delay(100);
}

// ============================================
// Sensor Functions
// ============================================
void readSensors() {
    // Read PIR Motion Sensor
    currentData.motion = digitalRead(PIR_PIN);
    if (currentData.motion) {
        systemState.lastMotion = millis();
    }
    
    // Read Ultrasonic Distance
    currentData.distance = readUltrasonicDistance();
    
    // Read Environmental Sensors
    currentData.temperature = bme.readTemperature();
    currentData.humidity = bme.readHumidity();
    currentData.pressure = bme.readPressure() / 100.0F;
    
    // Read GPS if available
    while (GPS_Serial.available() > 0) {
        if (gps.encode(GPS_Serial.read())) {
            if (gps.location.isValid()) {
                currentData.latitude = gps.location.lat();
                currentData.longitude = gps.location.lng();
            }
        }
    }
    
    // Update timestamp
    currentData.timestamp = millis();
}

float readUltrasonicDistance() {
    digitalWrite(ULTRASONIC_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(ULTRASONIC_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIG, LOW);
    
    long duration = pulseIn(ULTRASONIC_ECHO, HIGH, 30000);
    float distance = duration * 0.034 / 2;
    
    return distance;
}

// ============================================
// Alert Processing
// ============================================
void processAlerts() {
    AlertLevel newLevel = ALERT_NONE;
    String alertMessage = "";
    String alertType = "";
    
    // Check environmental conditions
    if (currentData.temperature < TEMP_MIN || currentData.temperature > TEMP_MAX) {
        newLevel = max(newLevel, ALERT_WARNING);
        alertMessage = "Temperature out of range: " + String(currentData.temperature) + "°C";
        alertType = "temperature";
    }
    
    if (currentData.humidity < HUMIDITY_MIN || currentData.humidity > HUMIDITY_MAX) {
        newLevel = max(newLevel, ALERT_INFO);
        alertMessage = "Humidity out of range: " + String(currentData.humidity) + "%";
        alertType = "humidity";
    }
    
    // Check motion timeout (possible fall or emergency)
    unsigned long motionElapsed = millis() - systemState.lastMotion;
    if (motionElapsed > MOTION_TIMEOUT && currentData.distance < DISTANCE_THRESHOLD) {
        newLevel = ALERT_EMERGENCY;
        alertMessage = "No motion detected for extended period";
        alertType = "motion_timeout";
    }
    
    // Check sudden distance change (possible fall)
    static float lastDistance = 0;
    float distanceChange = abs(currentData.distance - lastDistance);
    if (distanceChange > 100 && currentData.distance < 50) {
        newLevel = ALERT_EMERGENCY;
        alertMessage = "Possible fall detected";
        alertType = "fall";
    }
    lastDistance = currentData.distance;
    
    // Apply intelligent filtering based on time of day
    newLevel = applyTimeBasedFiltering(newLevel);
    
    // Send alert if level changed
    if (newLevel != systemState.alertLevel && newLevel > ALERT_NONE) {
        sendAlert(newLevel, alertMessage, alertType);
    }
    
    systemState.alertLevel = newLevel;
}

AlertLevel applyTimeBasedFiltering(AlertLevel level) {
    // Get current hour (would need RTC for accurate time)
    int hour = (millis() / 3600000) % 24;
    
    // Night time (22:00 - 06:00): More sensitive
    if (hour >= 22 || hour < 6) {
        // Upgrade info to warning during night
        if (level == ALERT_INFO) {
            return ALERT_WARNING;
        }
    } else {
        // Day time: Less sensitive for minor alerts
        if (level == ALERT_INFO && random(100) < 50) {
            return ALERT_NONE;  // Filter out 50% of info alerts during day
        }
    }
    
    return level;
}

// ============================================
// Communication Functions
// ============================================
void sendAlert(AlertLevel level, String message, String type) {
    if (!systemState.mqttConnected) {
        // Store locally if not connected
        storeLocalAlert(level, message, type);
        return;
    }
    
    StaticJsonDocument<512> doc;
    doc["device_id"] = DEVICE_ID;
    doc["timestamp"] = millis();
    doc["level"] = level;
    doc["message"] = message;
    doc["type"] = type;
    doc["location"]["lat"] = currentData.latitude;
    doc["location"]["lng"] = currentData.longitude;
    
    char buffer[512];
    serializeJson(doc, buffer);
    
    mqtt.publish(TOPIC_ALERT, buffer, true);
    
    // Local alert indication
    indicateAlert(level);
}

void sendSensorData() {
    if (!systemState.mqttConnected) return;
    
    StaticJsonDocument<1024> doc;
    doc["device_id"] = DEVICE_ID;
    doc["timestamp"] = currentData.timestamp;
    
    JsonObject data = doc.createNestedObject("data");
    data["motion"] = currentData.motion;
    data["distance"] = currentData.distance;
    data["temperature"] = currentData.temperature;
    data["humidity"] = currentData.humidity;
    data["pressure"] = currentData.pressure;
    data["battery"] = systemState.batteryLevel;
    
    JsonObject location = doc.createNestedObject("location");
    location["lat"] = currentData.latitude;
    location["lng"] = currentData.longitude;
    
    doc["alert_level"] = systemState.alertLevel;
    
    char buffer[1024];
    serializeJson(doc, buffer);
    
    mqtt.publish(TOPIC_SENSOR_DATA, buffer);
}

void sendHeartbeat() {
    if (!systemState.mqttConnected) return;
    
    StaticJsonDocument<256> doc;
    doc["device_id"] = DEVICE_ID;
    doc["timestamp"] = millis();
    doc["status"] = "online";
    doc["firmware"] = FIRMWARE_VERSION;
    doc["uptime"] = millis();
    doc["free_heap"] = ESP.getFreeHeap();
    doc["battery"] = systemState.batteryLevel;
    
    char buffer[256];
    serializeJson(doc, buffer);
    
    mqtt.publish(TOPIC_STATUS, buffer, true);
}

void sendPeriodicUpdates() {
    static unsigned long lastSensorUpdate = 0;
    static unsigned long lastHeartbeat = 0;
    
    // Send sensor data every 5 seconds
    if (millis() - lastSensorUpdate > 5000) {
        sendSensorData();
        lastSensorUpdate = millis();
    }
    
    // Send heartbeat every 30 seconds
    if (millis() - lastHeartbeat > 30000) {
        sendHeartbeat();
        lastHeartbeat = millis();
    }
}

// ============================================
// MQTT Functions
// ============================================
void reconnectMQTT() {
    if (!systemState.wifiConnected) return;
    
    int attempts = 0;
    while (!mqtt.connected() && attempts < 3) {
        Serial.print(F("Attempting MQTT connection..."));
        
        String clientId = "M5Stack-" + String(random(0xffff), HEX);
        
        if (mqtt.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
            Serial.println(F("connected"));
            systemState.mqttConnected = true;
            
            // Subscribe to command topic
            mqtt.subscribe(TOPIC_COMMAND);
            
            // Send initial status
            sendHeartbeat();
            
        } else {
            Serial.print(F("failed, rc="));
            Serial.print(mqtt.state());
            Serial.println(F(" retry in 5 seconds"));
            delay(5000);
            attempts++;
        }
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // Convert payload to string
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    Serial.print(F("Message arrived ["));
    Serial.print(topic);
    Serial.print(F("] "));
    Serial.println(message);
    
    // Parse JSON command
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.print(F("JSON parse failed: "));
        Serial.println(error.f_str());
        return;
    }
    
    // Handle commands
    String command = doc["command"];
    
    if (command == "test_alert") {
        sendAlert(ALERT_WARNING, "Test alert from command", "test");
    } else if (command == "reboot") {
        ESP.restart();
    } else if (command == "update_config") {
        updateConfiguration(doc["config"]);
    } else if (command == "clear_alert") {
        systemState.alertLevel = ALERT_NONE;
        indicateAlert(ALERT_NONE);
    }
}

// ============================================
// Display Functions
// ============================================
void updateDisplay() {
    static unsigned long lastDisplayUpdate = 0;
    
    // Update display every second
    if (millis() - lastDisplayUpdate < 1000) return;
    lastDisplayUpdate = millis();
    
    M5.Lcd.fillScreen(BLACK);
    
    // Header
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(getAlertColor(static_cast<AlertLevel>(systemState.alertLevel)));
    M5.Lcd.println("Care Copilot");
    
    // Status indicators
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(WHITE);
    
    // Network status
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.print("WiFi: ");
    M5.Lcd.setTextColor(systemState.wifiConnected ? GREEN : RED);
    M5.Lcd.println(systemState.wifiConnected ? "Connected" : "Disconnected");
    
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(10, 55);
    M5.Lcd.print("MQTT: ");
    M5.Lcd.setTextColor(systemState.mqttConnected ? GREEN : RED);
    M5.Lcd.println(systemState.mqttConnected ? "Connected" : "Disconnected");
    
    // Sensor data
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(10, 80);
    M5.Lcd.println("=== Sensor Data ===");
    
    M5.Lcd.setCursor(10, 100);
    M5.Lcd.print("Motion: ");
    M5.Lcd.setTextColor(currentData.motion ? YELLOW : WHITE);
    M5.Lcd.println(currentData.motion ? "Detected" : "None");
    
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(10, 115);
    M5.Lcd.printf("Distance: %.1f cm", currentData.distance);
    
    M5.Lcd.setCursor(10, 130);
    M5.Lcd.printf("Temp: %.1f C", currentData.temperature);
    
    M5.Lcd.setCursor(10, 145);
    M5.Lcd.printf("Humidity: %.1f %%", currentData.humidity);
    
    M5.Lcd.setCursor(10, 160);
    M5.Lcd.printf("Pressure: %.1f hPa", currentData.pressure);
    
    // Alert status
    M5.Lcd.setCursor(10, 185);
    M5.Lcd.println("=== Alert Status ===");
    M5.Lcd.setCursor(10, 200);
    M5.Lcd.setTextColor(getAlertColor(static_cast<AlertLevel>(systemState.alertLevel)));
    M5.Lcd.println(getAlertLevelString(static_cast<AlertLevel>(systemState.alertLevel)));
    
    // Battery
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(10, 220);
    M5.Lcd.printf("Battery: %.0f%%", systemState.batteryLevel);
    
    // Footer
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 235);
    M5.Lcd.setTextColor(DARKGREY);
    M5.Lcd.printf("Uptime: %lu min", millis() / 60000);
}

uint16_t getAlertColor(AlertLevel level) {
    switch (level) {
        case ALERT_NONE: return GREEN;
        case ALERT_INFO: return CYAN;
        case ALERT_WARNING: return YELLOW;
        case ALERT_EMERGENCY: return RED;
        default: return WHITE;
    }
}

String getAlertLevelString(AlertLevel level) {
    switch (level) {
        case ALERT_NONE: return "Normal - All Clear";
        case ALERT_INFO: return "Info - Monitor";
        case ALERT_WARNING: return "Warning - Check Soon";
        case ALERT_EMERGENCY: return "EMERGENCY - Immediate Action";
        default: return "Unknown";
    }
}

void displayStatus(String message) {
    M5.Lcd.fillRect(0, 200, 320, 40, BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 210);
    M5.Lcd.println(message);
}

void displayError(String error) {
    M5.Lcd.fillRect(0, 200, 320, 40, RED);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 210);
    M5.Lcd.println(error);
}

// ============================================
// Alert Indication
// ============================================
void indicateAlert(AlertLevel level) {
    switch (level) {
        case ALERT_NONE:
            digitalWrite(LED_PIN, LOW);
            noTone(BUZZER_PIN);
            break;
            
        case ALERT_INFO:
            digitalWrite(LED_PIN, HIGH);
            delay(100);
            digitalWrite(LED_PIN, LOW);
            break;
            
        case ALERT_WARNING:
            for (int i = 0; i < 3; i++) {
                digitalWrite(LED_PIN, HIGH);
                tone(BUZZER_PIN, 1000, 200);
                delay(200);
                digitalWrite(LED_PIN, LOW);
                delay(200);
            }
            break;
            
        case ALERT_EMERGENCY:
            for (int i = 0; i < 10; i++) {
                digitalWrite(LED_PIN, HIGH);
                tone(BUZZER_PIN, 2000, 100);
                delay(100);
                digitalWrite(LED_PIN, LOW);
                delay(100);
            }
            break;
    }
}

void playStartupSound() {
    tone(BUZZER_PIN, 523, 200);  // C
    delay(200);
    tone(BUZZER_PIN, 659, 200);  // E
    delay(200);
    tone(BUZZER_PIN, 784, 200);  // G
    delay(200);
}

// ============================================
// Button Handling
// ============================================
void handleButtons() {
    if (M5.BtnA.wasPressed()) {
        // Button A: Test alert
        sendAlert(ALERT_INFO, "Manual test alert", "manual");
    }
    
    if (M5.BtnB.wasPressed()) {
        // Button B: Clear alerts
        systemState.alertLevel = ALERT_NONE;
        displayStatus("Alerts Cleared");
    }
    
    if (M5.BtnC.wasPressed()) {
        // Button C: System info
        showSystemInfo();
    }
}

void showSystemInfo() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 10);
    
    M5.Lcd.println("=== System Information ===");
    M5.Lcd.printf("Device ID: %s\n", DEVICE_ID);
    M5.Lcd.printf("Firmware: %s\n", FIRMWARE_VERSION);
    M5.Lcd.printf("Free Heap: %d\n", ESP.getFreeHeap());
    M5.Lcd.printf("WiFi RSSI: %d dBm\n", WiFi.RSSI());
    M5.Lcd.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    M5.Lcd.printf("Uptime: %lu min\n", millis() / 60000);
    
    delay(5000);
}

// ============================================
// Utility Functions
// ============================================
void checkBattery() {
    static unsigned long lastBatteryCheck = 0;
    
    if (millis() - lastBatteryCheck < 60000) return;  // Check every minute
    lastBatteryCheck = millis();
    
    // Read battery voltage (simplified - actual implementation would need ADC)
    int batteryVoltage = M5.Power.getBatteryLevel();
    systemState.batteryLevel = batteryVoltage;
    
    // Low battery alert
    if (systemState.batteryLevel < 20) {
        sendAlert(ALERT_WARNING, "Low battery: " + String(systemState.batteryLevel) + "%", "battery");
    }
}

void storeLocalAlert(AlertLevel level, String message, String type) {
    // Store alert locally when offline
    // Implementation would use SPIFFS or SD card
    preferences.putUInt("last_alert_level", level);
    preferences.putString("last_alert_msg", message);
    preferences.putString("last_alert_type", type);
    preferences.putUInt("last_alert_time", millis());
}

void loadSettings() {
    // Load saved settings from preferences
    // WiFi credentials, thresholds, etc.
}

void updateConfiguration(JsonObject config) {
    // Update configuration from remote command
    if (config.containsKey("temp_min")) {
        float newTempMin = config["temp_min"];
        preferences.putFloat("temp_min", newTempMin);
    }
    
    if (config.containsKey("temp_max")) {
        float newTempMax = config["temp_max"];
        preferences.putFloat("temp_max", newTempMax);
    }
    
    // Apply other configuration updates...
    
    displayStatus("Config Updated");
}

// ============================================
// Error Handling
// ============================================
void handleError(String error) {
    Serial.print(F("ERROR: "));
    Serial.println(error);
    displayError(error);
    
    // Log error
    preferences.putString("last_error", error);
    preferences.putUInt("last_error_time", millis());
}
