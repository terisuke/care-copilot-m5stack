/**
 * Care Copilot M5Stack Fire + ToF + ENV.4 + GPS
 * 全センサー統合版
 * 転倒検知・離床検知・環境モニタリング・位置追跡
 */

#include <M5Unified.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <VL53L1X.h>      // ToF4M sensor
#include "M5UnitENV.h"    // ENV.4 sensor
#include <TinyGPSPlus.h>

// Configuration - create config_local.h with your settings
#if __has_include("config_local.h")
  #include "config_local.h"
#else
  #include "config.h"
#endif

// WiFi設定 (config.hから読み込み)
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

// MQTT設定 (config.hから読み込み)
const char* mqtt_server = MQTT_SERVER;
const int mqtt_port = MQTT_PORT;

// MQTTトピック
const char* TOPIC_SENSOR_DATA = "care/sensor/data";
const char* TOPIC_ENV_DATA = "care/environment";
const char* TOPIC_DISTANCE = "care/distance";
const char* TOPIC_LOCATION = "care/location";
const char* TOPIC_ALERT = "care/alert";
const char* TOPIC_STATUS = "care/status";

// センサーしきい値はconfig.hで定義

// Pa.HUB v2.1設定
#define PAHUB_I2C_ADDR 0x70  // Pa.HUBのI2Cアドレス
#define PAHUB_CH_TOF   0     // ToFセンサー用チャンネル
#define PAHUB_CH_ENV4  1     // ENV.4センサー用チャンネル

// Pa.HUBチャンネル選択関数
void selectPaHubChannel(uint8_t channel) {
    if (channel > 7) return;
    Wire.beginTransmission(PAHUB_I2C_ADDR);
    Wire.write(1 << channel);
    Wire.endTransmission();
    delay(10);  // チャンネル切り替え待機
}

// グローバル変数
WiFiClient espClient;
PubSubClient mqtt(espClient);

// PubSubClientのバッファサイズを増やす（デフォルト256）
void setupMqttBuffer() {
    mqtt.setBufferSize(1024);  // JSONメッセージ用に拡大
}
VL53L1X tof_sensor;  // ToF sensor
SHT4X sht4;          // 温湿度センサー
BMP280 bmp;          // 気圧センサー
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

// システム状態
struct {
    bool wifiConnected = false;
    bool mqttConnected = false;
    bool tofConnected = false;
    bool env4Connected = false;
    bool gpsConnected = false;
    int alertLevel = 0;
    String alertMessage = "";
} systemState;

// センサーデータ
struct {
    // IMU
    float accelX = 0, accelY = 0, accelZ = 0;
    float gyroX = 0, gyroY = 0, gyroZ = 0;
    bool fallDetected = false;
    // ToF
    uint16_t distance = 0;
    uint8_t rangeStatus = 0;
    bool motionDetected = false;
    String zoneStatus = "UNKNOWN";
    // ENV.4
    float temperature = 0;
    float humidity = 0;
    float pressure = 0;
    float altitude = 0;
    // GPS
    double latitude = 0;
    double longitude = 0;
    float gpsAltitude = 0;
    uint8_t satellites = 0;
    bool gpsValid = false;
    // System
    int batteryLevel = 0;
} sensorData;

// タイマー
unsigned long lastSensorRead = 0;
unsigned long lastToFRead = 0;
unsigned long lastEnvRead = 0;
unsigned long lastGpsRead = 0;
unsigned long lastMqttPublish = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastMotionTime = 0;

// 距離履歴（動き検出用）
#define HISTORY_SIZE 10
uint16_t distanceHistory[HISTORY_SIZE];
int historyIndex = 0;

// ============================================
// セットアップ
// ============================================
void setup() {
    // M5Stack Fire初期化
    auto cfg = M5.config();
    cfg.internal_imu = true;
    cfg.internal_rtc = true;
    M5.begin(cfg);
    
    Serial.begin(115200);
    Serial.println("\n=== Care Copilot Fire ALL SENSORS ===");
    Serial.println("ToF + ENV.4 + GPS Unified Version");
    
    // 画面初期設定
    M5.Display.setTextSize(2);
    M5.Display.clear();
    M5.Display.println("Care Copilot Fire");
    M5.Display.println("ALL SENSORS");
    M5.Display.println("Initializing...");
    
    // I2C初期化（Port A - Pa.HUB v2.1経由でToFとENV.4を接続）
    Serial.println("\nInitializing I2C with Pa.HUB v2.1 on Port A...");
    Wire.begin(21, 22);  // Port A (SDA=21, SCL=22)
    delay(100);
    
    // Pa.HUBの接続確認
    Serial.println("Checking Pa.HUB connection...");
    Wire.beginTransmission(PAHUB_I2C_ADDR);
    uint8_t pahub_error = Wire.endTransmission();
    if (pahub_error == 0) {
        Serial.println("Pa.HUB v2.1 detected at 0x70");
        M5.Display.println("Pa.HUB: OK");
    } else {
        Serial.println("Pa.HUB not found! Error: " + String(pahub_error));
        M5.Display.println("Pa.HUB: Not found!");
    }
    
    // ToF4Mセンサー初期化（Pa.HUBチャンネル0）
    Serial.println("\nInitializing ToF sensor on Pa.HUB CH0...");
    M5.Display.println("ToF init (CH0)...");
    
    selectPaHubChannel(PAHUB_CH_TOF);  // チャンネル0を選択
    tof_sensor.setTimeout(500);
    if (!tof_sensor.init()) {
        Serial.println("ToF sensor not found on CH0 (I2C: 0x29)");
        M5.Display.println("ToF: Not found");
        systemState.tofConnected = false;
    } else {
        // 長距離モード設定
        tof_sensor.setDistanceMode(VL53L1X::Long);
        tof_sensor.setMeasurementTimingBudget(50000);
        tof_sensor.startContinuous(50);
        Serial.println("ToF sensor OK on CH0 - Long range mode");
        systemState.tofConnected = true;
    }
    
    // ENV.4センサー初期化（Pa.HUBチャンネル1）
    Serial.println("\nInitializing ENV.4 sensors on Pa.HUB CH1...");
    M5.Display.println("ENV.4 init (CH1)...");
    
    selectPaHubChannel(PAHUB_CH_ENV4);  // チャンネル1を選択
    
    // SHT4X（温湿度）初期化
    if (!sht4.begin(&Wire, SHT40_I2C_ADDR_44, 21, 22, 400000U)) {
        Serial.println("SHT4X not found on CH1 (I2C: 0x44)");
        M5.Display.println("ENV.4: Not found");
        systemState.env4Connected = false;
    } else {
        sht4.setPrecision(SHT4X_HIGH_PRECISION);
        sht4.setHeater(SHT4X_NO_HEATER);
        Serial.println("SHT4X OK on CH1 (I2C: 0x44)");
        systemState.env4Connected = true;
    }
    
    // BMP280（気圧）初期化（同じチャンネル1）
    if (!bmp.begin(&Wire, BMP280_I2C_ADDR, 21, 22, 400000U)) {
        Serial.println("BMP280 not found on CH1 (I2C: 0x76)");
    } else {
        bmp.setSampling(BMP280::MODE_NORMAL,
                       BMP280::SAMPLING_X2,
                       BMP280::SAMPLING_X16,
                       BMP280::FILTER_X16,
                       BMP280::STANDBY_MS_500);
        Serial.println("BMP280 OK on CH1 (I2C: 0x76)");
    }
    
    // GPS初期化（Port C）
    Serial.println("\nInitializing GPS on Port C...");
    M5.Display.println("GPS init...");
    
    // ボーレート自動検出（ENV4ファイルから実証済みの方法）
    int baudRates[] = {115200, 38400, 9600, 57600, 4800};
    String baudNames[] = {"115200", "38400", "9600", "57600", "4800"};
    bool gpsDetected = false;
    int correctBaud = 9600;
    
    Serial.println("Testing different baud rates...");
    for (int i = 0; i < 5; i++) {
        Serial.print("Testing ");
        Serial.print(baudNames[i]);
        Serial.print(" bps... ");
        
        gpsSerial.begin(baudRates[i], SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
        delay(500);
        
        String testData = "";
        int count = 0;
        while (gpsSerial.available() > 0 && count < 100) {
            char c = gpsSerial.read();
            testData += c;
            count++;
        }
        
        // NMEAフォーマットの確認（$Gで始まる）
        if (testData.indexOf("$G") >= 0) {
            gpsDetected = true;
            correctBaud = baudRates[i];
            Serial.println("SUCCESS! GPS detected.");
            Serial.println("Sample: " + testData.substring(0, 50));
            break;
        } else if (testData.length() > 0) {
            Serial.println("Data received but not GPS format");
        } else {
            Serial.println("No data");
        }
    }
    
    if (gpsDetected) {
        gpsSerial.begin(correctBaud, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
        systemState.gpsConnected = true;
        Serial.println("\nGPS initialized at " + String(correctBaud) + " bps");
        M5.Display.println("GPS OK: " + String(correctBaud));
    } else {
        Serial.println("\nWARNING: GPS not detected or wrong baud rate");
        Serial.println("Using default 9600 bps");
        gpsSerial.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    }
    
    // WiFi接続
    setupWiFi();
    
    // MQTT設定
    setupMqttBuffer();  // バッファサイズ設定
    mqtt.setServer(mqtt_server, mqtt_port);
    mqtt.setCallback(mqttCallback);
    
    // MQTT初回接続試行
    if (systemState.wifiConnected) {
        Serial.println("\nAttempting initial MQTT connection...");
        reconnectMQTT();
    }
    
    // 起動音
    M5.Speaker.tone(523, 200);
    delay(200);
    M5.Speaker.tone(659, 200);
    delay(200);
    M5.Speaker.tone(784, 200);
    
    Serial.println("\n=== Setup Complete ===");
    Serial.println("Sensors status:");
    Serial.println("- ToF: " + String(systemState.tofConnected ? "OK" : "NG"));
    Serial.println("- ENV.4: " + String(systemState.env4Connected ? "OK" : "NG"));
    Serial.println("- GPS: " + String(systemState.gpsConnected ? "Detected" : "Waiting"));
    
    updateDisplay();
}

// ============================================
// メインループ
// ============================================
void loop() {
    M5.update();
    
    // ボタン処理
    handleButtons();
    
    // WiFi維持
    if (!WiFi.isConnected()) {
        systemState.wifiConnected = false;
        setupWiFi();
    } else {
        systemState.wifiConnected = true;
    }
    
    // MQTT維持
    if (systemState.wifiConnected) {
        if (!mqtt.connected()) {
            systemState.mqttConnected = false;
            static unsigned long lastReconnectAttempt = 0;
            unsigned long now = millis();
            if (now - lastReconnectAttempt > 5000) {  // 5秒ごとに再接続試行
                lastReconnectAttempt = now;
                reconnectMQTT();
            }
        } else {
            mqtt.loop();
            systemState.mqttConnected = true;
        }
    }
    
    // IMUセンサー読み取り
    if (millis() - lastSensorRead > IMU_READ_INTERVAL) {
        readIMU();
        checkFallDetection();
        lastSensorRead = millis();
    }
    
    // ToFセンサー読み取り
    if (millis() - lastToFRead > TOF_READ_INTERVAL) {
        readToF();
        checkDistanceAlerts();
        lastToFRead = millis();
    }
    
    // ENV.4センサー読み取り
    if (millis() - lastEnvRead > ENV_READ_INTERVAL) {
        readEnv4();
        checkEnvironmentAlerts();
        lastEnvRead = millis();
    }
    
    // GPS読み取り
    if (millis() - lastGpsRead > GPS_READ_INTERVAL) {
        readGPS();
        lastGpsRead = millis();
    }
    
    // MQTT送信
    if (millis() - lastMqttPublish > MQTT_PUBLISH_INTERVAL && systemState.mqttConnected) {
        publishSensorData();
        lastMqttPublish = millis();
    }
    
    // 画面更新
    if (millis() - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
        updateDisplay();
        lastDisplayUpdate = millis();
    }
    
    delay(10);
}

// ============================================
// センサー読み取り
// ============================================
void readIMU() {
    auto imu_update = M5.Imu.update();
    if (imu_update) {
        auto data = M5.Imu.getImuData();
        sensorData.accelX = data.accel.x;
        sensorData.accelY = data.accel.y;
        sensorData.accelZ = data.accel.z;
        sensorData.gyroX = data.gyro.x;
        sensorData.gyroY = data.gyro.y;
        sensorData.gyroZ = data.gyro.z;
    }
    sensorData.batteryLevel = M5.Power.getBatteryLevel();
}

void readToF() {
    if (!systemState.tofConnected) return;
    
    selectPaHubChannel(PAHUB_CH_TOF);  // ToFチャンネルを選択
    sensorData.distance = tof_sensor.read();
    sensorData.rangeStatus = tof_sensor.ranging_data.range_status;
    
    // 履歴更新
    distanceHistory[historyIndex] = sensorData.distance;
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;
    
    // 動き検出
    detectMotion();
    
    // ゾーン判定
    if (sensorData.distance < DISTANCE_NEAR) {
        sensorData.zoneStatus = "NEAR";
    } else if (sensorData.distance < DISTANCE_FAR) {
        sensorData.zoneStatus = "NORMAL";
    } else if (sensorData.distance < DISTANCE_MAX) {
        sensorData.zoneStatus = "FAR";
    } else {
        sensorData.zoneStatus = "OUT_OF_RANGE";
    }
    
    Serial.print("Distance: ");
    Serial.print(sensorData.distance);
    Serial.print("mm, Zone: ");
    Serial.print(sensorData.zoneStatus);
    Serial.print(", Motion: ");
    Serial.println(sensorData.motionDetected ? "YES" : "NO");
}

void readEnv4() {
    if (!systemState.env4Connected) return;
    
    selectPaHubChannel(PAHUB_CH_ENV4);  // ENV.4チャンネルを選択
    
    // 温湿度読み取り
    if (sht4.update()) {
        sensorData.temperature = sht4.cTemp;
        sensorData.humidity = sht4.humidity;
        Serial.print("Temp: ");
        Serial.print(sensorData.temperature);
        Serial.print("°C, Humidity: ");
        Serial.print(sensorData.humidity);
        Serial.println("%");
    }
    
    // 気圧読み取り
    if (bmp.update()) {
        sensorData.pressure = bmp.pressure;
        sensorData.altitude = bmp.altitude;
        Serial.print("Pressure: ");
        Serial.print(sensorData.pressure);
        Serial.print("Pa, Altitude: ");
        Serial.print(sensorData.altitude);
        Serial.println("m");
    }
}

void readGPS() {
    static unsigned long lastDebugTime = 0;
    
    // GPSデータ読み取り（ENV4ファイルから実証済みの方法）
    int bytesAvailable = gpsSerial.available();
    if (bytesAvailable > 0 && millis() - lastDebugTime > 5000) {
        Serial.println("GPS: " + String(bytesAvailable) + " bytes available");
        lastDebugTime = millis();
    }
    
    while (gpsSerial.available() > 0) {
        char c = gpsSerial.read();
        
        if (gps.encode(c)) {
            Serial.println("\n=== GPS Status ===");
            Serial.print("Chars processed: ");
            Serial.println(gps.charsProcessed());
            Serial.print("Sentences: ");
            Serial.println(gps.sentencesWithFix());
            
            if (gps.location.isValid()) {
                sensorData.latitude = gps.location.lat();
                sensorData.longitude = gps.location.lng();
                sensorData.gpsValid = true;
                
                static bool firstFix = true;
                if (firstFix) {
                    Serial.println("\n*** GPS FIX ACQUIRED! ***");
                    Serial.print("Location: ");
                    Serial.print(sensorData.latitude, 6);
                    Serial.print(", ");
                    Serial.println(sensorData.longitude, 6);
                    firstFix = false;
                }
            }
            
            if (gps.altitude.isValid()) {
                sensorData.gpsAltitude = gps.altitude.meters();
            }
            
            if (gps.satellites.isValid()) {
                sensorData.satellites = gps.satellites.value();
            }
        }
    }
    
    // GPS診断情報（10秒後にデータがない場合）
    if (millis() > 10000 && gps.charsProcessed() < 10) {
        Serial.println("\nWARNING: No valid GPS data after 10 seconds");
        Serial.println("Possible issues:");
        Serial.println("1. Indoor location (move near window)");
        Serial.println("2. Wiring issue (check Port C connections)");
    }
}

// ============================================
// 検知処理
// ============================================
void detectMotion() {
    // 距離変化の分散を計算
    float avgDistance = 0;
    for (int i = 0; i < HISTORY_SIZE; i++) {
        avgDistance += distanceHistory[i];
    }
    avgDistance /= HISTORY_SIZE;
    
    float variance = 0;
    for (int i = 0; i < HISTORY_SIZE; i++) {
        float diff = distanceHistory[i] - avgDistance;
        variance += diff * diff;
    }
    variance /= HISTORY_SIZE;
    
    // 分散が閾値を超えたら動き検出
    sensorData.motionDetected = (variance > 10000);
    
    if (sensorData.motionDetected) {
        lastMotionTime = millis();
    }
}

void checkFallDetection() {
    float accelMagnitude = sqrt(
        sensorData.accelX * sensorData.accelX +
        sensorData.accelY * sensorData.accelY +
        sensorData.accelZ * sensorData.accelZ
    );
    
    static float lastMagnitude = 1.0;
    float change = abs(accelMagnitude - lastMagnitude);
    
    if (change > FALL_THRESHOLD) {
        sensorData.fallDetected = true;
        systemState.alertLevel = 3;
        systemState.alertMessage = "Fall detected!";
        
        String alertMsg = "Fall detected!";
        if (sensorData.gpsValid) {
            alertMsg += " Location: " + String(sensorData.latitude, 6) + 
                       "," + String(sensorData.longitude, 6);
        }
        sendAlert(3, alertMsg);
    }
    lastMagnitude = accelMagnitude;
}

void checkDistanceAlerts() {
    if (!systemState.tofConnected) return;
    
    static String lastZone = "UNKNOWN";
    
    // ゾーン変化検出
    if (sensorData.zoneStatus != lastZone) {
        Serial.println("Zone changed: " + lastZone + " -> " + sensorData.zoneStatus);
        
        if (sensorData.zoneStatus == "FAR" && lastZone == "NORMAL") {
            // 離床検知
            systemState.alertLevel = 2;
            systemState.alertMessage = "Bed exit detected";
            sendAlert(2, "Bed exit detected - Distance: " + String(sensorData.distance) + "mm");
        } else if (sensorData.zoneStatus == "NEAR") {
            // 接近検知
            systemState.alertLevel = 1;
            systemState.alertMessage = "Someone approaching";
            sendAlert(1, "Proximity alert - Distance: " + String(sensorData.distance) + "mm");
        }
        
        lastZone = sensorData.zoneStatus;
    }
    
    // 長時間動きなし検知
    if (millis() - lastMotionTime > NO_MOTION_TIMEOUT && !sensorData.motionDetected) {
        systemState.alertLevel = 1;
        systemState.alertMessage = "No motion for 10min";
    }
}

void checkEnvironmentAlerts() {
    if (!systemState.env4Connected) return;
    
    // 温度チェック
    if (sensorData.temperature > TEMP_HIGH) {
        if (systemState.alertLevel < 2) {
            systemState.alertLevel = 2;
            systemState.alertMessage = "High temperature!";
            sendAlert(2, "High temp: " + String(sensorData.temperature) + "°C");
        }
    } else if (sensorData.temperature < TEMP_LOW && sensorData.temperature > 0) {
        if (systemState.alertLevel < 1) {
            systemState.alertLevel = 1;
            systemState.alertMessage = "Low temperature";
        }
    }
    
    // 湿度チェック
    if (sensorData.humidity > HUMIDITY_HIGH) {
        if (systemState.alertLevel < 1) {
            systemState.alertLevel = 1;
            systemState.alertMessage = "High humidity";
        }
    } else if (sensorData.humidity < HUMIDITY_LOW && sensorData.humidity > 0) {
        if (systemState.alertLevel < 1) {
            systemState.alertLevel = 1;
            systemState.alertMessage = "Low humidity";
        }
    }
}

// ============================================
// WiFi/MQTT
// ============================================
void setupWiFi() {
    Serial.println("Connecting to WiFi...");
    M5.Display.println("\nConnecting WiFi...");
    
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        systemState.wifiConnected = true;
        Serial.println("\nWiFi Connected!");
        Serial.println("IP: " + WiFi.localIP().toString());
    }
}

void reconnectMQTT() {
    if (!mqtt.connected()) {
        Serial.print("Attempting MQTT connection to ");
        Serial.print(mqtt_server);
        Serial.print(":");
        Serial.print(mqtt_port);
        Serial.print("...");
        
        String clientId = "M5Fire-" + String(random(0xffff), HEX);
        
        // タイムアウトを短くして、接続試行
        mqtt.setSocketTimeout(5);  // 5秒でタイムアウト
        
        if (mqtt.connect(clientId.c_str())) {
            systemState.mqttConnected = true;
            Serial.println(" Connected!");
            Serial.println("Client ID: " + clientId);
            mqtt.subscribe("care/command/+");
        } else {
            systemState.mqttConnected = false;
            Serial.print(" Failed, rc=");
            Serial.print(mqtt.state());
            Serial.println(" retry in 5 seconds");
            
            // MQTTエラーコードの説明
            switch(mqtt.state()) {
                case -4: Serial.println("MQTT_CONNECTION_TIMEOUT"); break;
                case -3: Serial.println("MQTT_CONNECTION_LOST"); break;
                case -2: Serial.println("MQTT_CONNECT_FAILED"); break;
                case -1: Serial.println("MQTT_DISCONNECTED"); break;
                case 0: Serial.println("MQTT_CONNECTED"); break;
                case 1: Serial.println("MQTT_CONNECT_BAD_PROTOCOL"); break;
                case 2: Serial.println("MQTT_CONNECT_BAD_CLIENT_ID"); break;
                case 3: Serial.println("MQTT_CONNECT_UNAVAILABLE"); break;
                case 4: Serial.println("MQTT_CONNECT_BAD_CREDENTIALS"); break;
                case 5: Serial.println("MQTT_CONNECT_UNAUTHORIZED"); break;
            }
        }
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.println("MQTT received: " + message);
}

void publishSensorData() {
    StaticJsonDocument<768> doc;  // サイズ拡大
    
    doc["device_id"] = DEVICE_ID;
    doc["timestamp"] = millis();
    
    // IMUデータ
    JsonObject imu = doc.createNestedObject("imu");
    imu["accel_x"] = sensorData.accelX;
    imu["accel_y"] = sensorData.accelY;
    imu["accel_z"] = sensorData.accelZ;
    imu["fall_detected"] = sensorData.fallDetected;
    
    // ToFデータ
    if (systemState.tofConnected) {
        JsonObject tof = doc.createNestedObject("tof");
        tof["distance"] = sensorData.distance;
        tof["zone"] = sensorData.zoneStatus;
        tof["motion"] = sensorData.motionDetected;
    }
    
    // ENV.4データ
    if (systemState.env4Connected) {
        JsonObject env = doc.createNestedObject("environment");
        env["temperature"] = sensorData.temperature;
        env["humidity"] = sensorData.humidity;
        env["pressure"] = sensorData.pressure;
        env["altitude"] = sensorData.altitude;
    }
    
    // GPSデータ
    if (sensorData.gpsValid) {
        JsonObject gps = doc.createNestedObject("gps");
        gps["latitude"] = sensorData.latitude;
        gps["longitude"] = sensorData.longitude;
        gps["altitude"] = sensorData.gpsAltitude;
        gps["satellites"] = sensorData.satellites;
    }
    
    // システムデータ
    doc["battery"] = sensorData.batteryLevel;
    doc["alert_level"] = systemState.alertLevel;
    doc["wifi_rssi"] = WiFi.RSSI();
    
    char buffer[768];
    serializeJson(doc, buffer);
    
    if (mqtt.publish(TOPIC_SENSOR_DATA, buffer)) {
        Serial.println("Data published to MQTT");
    }
}

void sendAlert(int level, String message) {
    StaticJsonDocument<256> doc;
    doc["device_id"] = DEVICE_ID;
    doc["level"] = level;
    doc["message"] = message;
    doc["timestamp"] = millis();
    
    if (sensorData.gpsValid) {
        doc["latitude"] = sensorData.latitude;
        doc["longitude"] = sensorData.longitude;
    }
    
    char buffer[256];
    serializeJson(doc, buffer);
    
    mqtt.publish(TOPIC_ALERT, buffer);
    
    if (level >= 3) {
        M5.Speaker.tone(1000, 500);
    } else if (level >= 2) {
        M5.Speaker.tone(800, 300);
    }
}

// ============================================
// UI
// ============================================
void handleButtons() {
    if (M5.BtnA.wasPressed()) {
        // テスト用にレベル2に変更（LINE通知が送信される）
        sendAlert(2, "Test alert - Button A (Level 2)");
    }
    
    if (M5.BtnB.wasPressed()) {
        systemState.alertLevel = 0;
        sensorData.fallDetected = false;
        systemState.alertMessage = "";
    }
    
    if (M5.BtnC.wasPressed()) {
        showDetailedInfo();
    }
}

void updateDisplay() {
    M5.Display.clear();
    M5.Display.setCursor(0, 0);
    M5.Display.setTextSize(2);
    
    // タイトル
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.println("Care Copilot");
    M5.Display.setTextColor(TFT_WHITE);
    
    // 接続状態（1行目）
    M5.Display.print("W:");
    M5.Display.setTextColor(systemState.wifiConnected ? TFT_GREEN : TFT_RED);
    M5.Display.print(systemState.wifiConnected ? "OK " : "NG ");
    M5.Display.setTextColor(TFT_WHITE);
    
    M5.Display.print("M:");
    M5.Display.setTextColor(systemState.mqttConnected ? TFT_GREEN : TFT_RED);
    M5.Display.print(systemState.mqttConnected ? "OK " : "NG ");
    M5.Display.setTextColor(TFT_WHITE);
    
    M5.Display.print("T:");
    M5.Display.setTextColor(systemState.tofConnected ? TFT_GREEN : TFT_RED);
    M5.Display.print(systemState.tofConnected ? "OK " : "NG ");
    M5.Display.setTextColor(TFT_WHITE);
    
    M5.Display.print("E:");
    M5.Display.setTextColor(systemState.env4Connected ? TFT_GREEN : TFT_RED);
    M5.Display.print(systemState.env4Connected ? "OK " : "NG ");
    M5.Display.setTextColor(TFT_WHITE);
    
    M5.Display.print("G:");
    M5.Display.setTextColor(sensorData.gpsValid ? TFT_GREEN : TFT_YELLOW);
    M5.Display.println(sensorData.gpsValid ? "OK" : "..");
    M5.Display.setTextColor(TFT_WHITE);
    
    // IMU状態（転倒検知）
    M5.Display.print("\nIMU: ");
    if (sensorData.fallDetected) {
        M5.Display.setTextColor(TFT_RED);
        M5.Display.println("FALL DETECTED!");
    } else {
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.println("Normal");
    }
    M5.Display.setTextColor(TFT_WHITE);
    
    // ToFデータ
    if (systemState.tofConnected) {
        M5.Display.println("\nDist: " + String(sensorData.distance) + "mm");
        M5.Display.println("Zone: " + sensorData.zoneStatus);
    }
    
    // ENV.4データ
    if (systemState.env4Connected) {
        M5.Display.println("\nT:" + String(sensorData.temperature, 1) + "C H:" + 
                          String(sensorData.humidity, 1) + "%");
    }
    
    // GPSデータ（コンパクト表示）
    if (sensorData.gpsValid) {
        M5.Display.println("\nGPS: Fixed");
        M5.Display.println("Sat:" + String(sensorData.satellites));
    } else {
        M5.Display.println("\nGPS: Searching");
        M5.Display.println("Sat:" + String(sensorData.satellites));
    }
    
    // アラート（画面中央に大きく表示）
    if (systemState.alertLevel > 0) {
        M5.Display.setCursor(10, 140);
        M5.Display.setTextColor(systemState.alertLevel >= 2 ? TFT_RED : TFT_YELLOW);
        M5.Display.setTextSize(2);
        M5.Display.println("*** ALERT ***");
        M5.Display.setCursor(10, 160);
        M5.Display.setTextSize(1);
        M5.Display.println(systemState.alertMessage);
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(TFT_WHITE);
    }
    
    // バッテリーとMQTT状態（最下部に固定）
    M5.Display.setCursor(0, 195);
    M5.Display.setTextSize(1);
    M5.Display.println("Bat:" + String(sensorData.batteryLevel) + "% MQTT:" + mqtt_server);
    if (!systemState.mqttConnected) {
        M5.Display.setTextColor(TFT_YELLOW);
        M5.Display.println("Retrying MQTT connection...");
        M5.Display.setTextColor(TFT_WHITE);
    }
    M5.Display.setCursor(0, 220);
    M5.Display.setTextSize(2);
    M5.Display.println("[A]Alert [B]Clear [C]Info");
}

void showDetailedInfo() {
    M5.Display.clear();
    M5.Display.setCursor(0, 0);
    M5.Display.setTextSize(1);
    
    M5.Display.println("=== ALL SENSORS INFO ===");
    
    M5.Display.println("\nIMU:");
    M5.Display.println("Accel: X=" + String(sensorData.accelX, 2) + 
                       " Y=" + String(sensorData.accelY, 2) + 
                       " Z=" + String(sensorData.accelZ, 2));
    
    if (systemState.tofConnected) {
        M5.Display.println("\nToF:");
        M5.Display.println("Distance: " + String(sensorData.distance) + " mm");
        M5.Display.println("Zone: " + sensorData.zoneStatus);
        M5.Display.println("Motion: " + String(sensorData.motionDetected ? "Detected" : "None"));
    }
    
    if (systemState.env4Connected) {
        M5.Display.println("\nENV.4:");
        M5.Display.println("Temperature: " + String(sensorData.temperature, 2) + " C");
        M5.Display.println("Humidity: " + String(sensorData.humidity, 2) + " %");
        M5.Display.println("Pressure: " + String(sensorData.pressure) + " Pa");
        M5.Display.println("Altitude: " + String(sensorData.altitude, 2) + " m");
    }
    
    if (sensorData.gpsValid) {
        M5.Display.println("\nGPS:");
        M5.Display.println("Lat: " + String(sensorData.latitude, 6));
        M5.Display.println("Lng: " + String(sensorData.longitude, 6));
        M5.Display.println("Alt: " + String(sensorData.gpsAltitude, 1) + " m");
        M5.Display.println("Satellites: " + String(sensorData.satellites));
    } else {
        M5.Display.println("\nGPS: Searching...");
        M5.Display.println("Satellites: " + String(sensorData.satellites));
    }
    
    M5.Display.println("\nSystem:");
    M5.Display.println("Battery: " + String(sensorData.batteryLevel) + "%");
    M5.Display.println("WiFi RSSI: " + String(WiFi.RSSI()) + " dBm");
    M5.Display.println("Uptime: " + String(millis()/1000) + " s");
    M5.Display.println("\nI2C Addresses:");
    M5.Display.println("Pa.HUB: 0x70");
    M5.Display.println("ToF(CH0): 0x29, ENV4(CH1): 0x44/0x76");
    
    M5.Display.println("\nPress any button to return");
    M5.Display.setTextSize(2);
    
    while (!M5.BtnA.wasPressed() && !M5.BtnB.wasPressed() && !M5.BtnC.wasPressed()) {
        M5.update();
        delay(10);
    }
}