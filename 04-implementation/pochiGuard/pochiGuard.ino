/**
 * PochiGuard - 犬用GPSトラッカー＆活動量計
 * For M5StickC Plus2
 * 
 * 機能：
 * - GPS位置追跡（脱走検知）
 * - 歩数計・活動量計測
 * - LINE通知（5分ごとの位置情報）
 * - リアルタイム活動モニタリング
 * 
 * ノアちゃん専用設定済み
 */

// 設定ファイルを最初に読み込む（重要）
#include "config.h"
#if __has_include("config_local.h")
  #include "config_local.h"
#endif

#include <M5StickCPlus2.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TinyGPSPlus.h>
#include <time.h>

// ====================================
// グローバル変数
// ====================================
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

// 犬の活動データ
struct DogActivity {
    // GPS関連
    double latitude = 0;
    double longitude = 0;
    float distanceFromHome = 0;
    bool gpsValid = false;
    uint8_t satellites = 0;
    
    // 脱走検知
    bool isEscaped = false;
    unsigned long escapeStartTime = 0;
    unsigned long lastEscapeNotify = 0;
    
    // IMU関連（活動量）
    uint32_t stepCount = 0;
    uint32_t dailySteps = 0;
    float activityLevel = 0;
    String activityType = "休息";
    float calories = 0;
    
    // 統計
    uint32_t activeMinutes = 0;
    uint32_t restMinutes = 0;
    float totalDistance = 0;
    float maxSpeed = 0;
    
    // 健康監視
    bool fallDetected = false;
    bool tremblingDetected = false;
    
    // システム
    int batteryLevel = 0;
    bool wifiConnected = false;
} dogData;

// タイマー管理
unsigned long lastIMURead = 0;
unsigned long lastGPSRead = 0;
unsigned long lastLocationCheck = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastActivitySummary = 0;
unsigned long activityStartTime = 0;

// 歩数検知用変数
float lastAccelMagnitude = 1.0;
unsigned long lastStepTime = 0;
int stepBuffer[5] = {0};
int stepBufferIndex = 0;

// WiFiクライアント
WiFiClient client;

// ====================================
// 年齢計算関数
// ====================================
String calculateAge() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "不明";
    }
    
    int currentYear = timeinfo.tm_year + 1900;
    int currentMonth = timeinfo.tm_mon + 1;
    int currentDay = timeinfo.tm_mday;
    
    int years = currentYear - DOG_BIRTH_YEAR;
    int months = currentMonth - DOG_BIRTH_MONTH;
    int days = currentDay - DOG_BIRTH_DAY;
    
    if (days < 0) {
        months--;
        days += 30;
    }
    if (months < 0) {
        years--;
        months += 12;
    }
    
    return String(years) + "歳" + String(months) + "ヶ月";
}

// ====================================
// セットアップ
// ====================================
void setup() {
    // M5StickC Plus2初期化
    auto cfg = M5.config();
    cfg.internal_imu = true;
    cfg.internal_rtc = true;
    M5.begin(cfg);
    
    Serial.begin(115200);
    Serial.println("\n=== PochiGuard Starting ===");
    Serial.print("愛犬: ");
    Serial.print(DOG_NAME);
    Serial.print(" (");
    Serial.print(DOG_BREED);
    Serial.println(")");
    
    // ディスプレイ初期設定
    M5.Display.setRotation(1);
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(0, 0);
    M5.Display.println("PochiGuard");
    M5.Display.setTextSize(1);
    M5.Display.print("for ");
    M5.Display.println(DOG_NAME);
    M5.Display.println("\n初期化中...");
    
    // IMU初期化
    M5.Imu.init();
    Serial.println("IMU: OK");
    M5.Display.println("IMU: OK");
    
    // GPS初期化（Grove Port使用）
    Serial.println("Initializing GPS...");
    M5.Display.println("GPS初期化...");
    
    // ボーレート自動検出
    int baudRates[] = {9600, 38400, 115200, 57600, 4800};
    bool gpsDetected = false;
    
    for (int i = 0; i < 5; i++) {
        gpsSerial.begin(baudRates[i], SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
        delay(500);
        
        String testData = "";
        while (gpsSerial.available() > 0) {
            char c = gpsSerial.read();
            testData += c;
            if (testData.length() > 100) break;
        }
        
        if (testData.indexOf("$G") >= 0) {
            gpsDetected = true;
            Serial.print("GPS detected at ");
            Serial.print(baudRates[i]);
            Serial.println(" bps");
            M5.Display.print("GPS: ");
            M5.Display.print(baudRates[i]);
            M5.Display.println("bps");
            break;
        }
    }
    
    if (!gpsDetected) {
        gpsSerial.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
        Serial.println("GPS not detected, using default 9600 bps");
        M5.Display.println("GPS: 検索中");
    }
    
    // WiFi接続
    setupWiFi();
    
    // 時刻同期
    configTime(9 * 3600, 0, "pool.ntp.org");
    
    // 起動音
    M5.Speaker.tone(523, 100);
    delay(100);
    M5.Speaker.tone(659, 100);
    
    // 開始時刻記録
    activityStartTime = millis();
    
    Serial.println("=== Setup Complete ===");
    delay(2000);
}

// ====================================
// メインループ
// ====================================
void loop() {
    M5.update();
    
    // ボタン処理
    handleButtons();
    
    // IMU読み取り（歩数検知）
    if (millis() - lastIMURead > IMU_READ_INTERVAL) {
        readIMU();
        detectSteps();
        detectActivity();
        lastIMURead = millis();
    }
    
    // GPS読み取り
    if (millis() - lastGPSRead > GPS_READ_INTERVAL) {
        readGPS();
        lastGPSRead = millis();
    }
    
    // 位置チェック（5秒ごと）
    if (millis() - lastLocationCheck > LOCATION_CHECK_INTERVAL) {
        checkLocation();
        lastLocationCheck = millis();
    }
    
    // 画面更新
    if (millis() - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
        updateDisplay();
        lastDisplayUpdate = millis();
    }
    
    // 活動サマリー送信（1時間ごと）
    if (millis() - lastActivitySummary > ACTIVITY_SUMMARY_INTERVAL) {
        sendActivitySummary();
        lastActivitySummary = millis();
    }
    
    // バッテリーレベル更新
    dogData.batteryLevel = M5.Power.getBatteryLevel();
    
    delay(10);
}

// ====================================
// IMU関連
// ====================================
void readIMU() {
    auto imu_update = M5.Imu.update();
    if (!imu_update) return;
}

void detectSteps() {
    auto data = M5.Imu.getImuData();
    
    // 加速度の大きさを計算
    float magnitude = sqrt(
        data.accel.x * data.accel.x +
        data.accel.y * data.accel.y +
        data.accel.z * data.accel.z
    );
    
    // ピーク検出による歩数カウント
    unsigned long currentTime = millis();
    if (magnitude > STEP_THRESHOLD && 
        lastAccelMagnitude <= STEP_THRESHOLD &&
        currentTime - lastStepTime > 200) {  // 200ms = 最速5歩/秒
        
        dogData.stepCount++;
        dogData.dailySteps++;
        lastStepTime = currentTime;
        
        // カロリー計算（体重と歩数から）
        dogData.calories = dogData.dailySteps * DOG_WEIGHT * CALORIE_FACTOR;
        
        if (DEBUG_IMU) {
            Serial.print("Step detected! Total: ");
            Serial.println(dogData.stepCount);
        }
    }
    
    lastAccelMagnitude = magnitude;
    
    // 転倒検知
    if (magnitude > FALL_THRESHOLD) {
        dogData.fallDetected = true;
        sendAlert("転倒検知", "⚠️ " + String(DOG_NAME) + "が転倒した可能性があります！", true);
    }
}

void detectActivity() {
    auto data = M5.Imu.getImuData();
    
    // 活動レベル計算（0-100）
    float accelVariance = abs(data.accel.x) + abs(data.accel.y) + abs(data.accel.z);
    dogData.activityLevel = min(100.0f, accelVariance * 20);
    
    // 活動タイプ判定
    if (dogData.activityLevel < 10) {
        dogData.activityType = "睡眠";
        dogData.restMinutes++;
    } else if (dogData.activityLevel < 30) {
        dogData.activityType = "休息";
        dogData.restMinutes++;
    } else if (dogData.activityLevel < 60) {
        dogData.activityType = "歩行";
        dogData.activeMinutes++;
    } else if (dogData.activityLevel < 80) {
        dogData.activityType = "走行";
        dogData.activeMinutes++;
    } else {
        dogData.activityType = "遊び";
        dogData.activeMinutes++;
    }
}

// ====================================
// GPS関連
// ====================================
void readGPS() {
    while (gpsSerial.available() > 0) {
        char c = gpsSerial.read();
        if (gps.encode(c)) {
            if (gps.location.isValid()) {
                dogData.latitude = gps.location.lat();
                dogData.longitude = gps.location.lng();
                dogData.gpsValid = true;
                
                // 自宅からの距離計算
                dogData.distanceFromHome = calculateDistance(
                    dogData.latitude, dogData.longitude,
                    HOME_LATITUDE, HOME_LONGITUDE
                );
                
                if (DEBUG_GPS) {
                    Serial.print("GPS: ");
                    Serial.print(dogData.latitude, 6);
                    Serial.print(", ");
                    Serial.print(dogData.longitude, 6);
                    Serial.print(" | Distance from home: ");
                    Serial.print(dogData.distanceFromHome);
                    Serial.println("m");
                }
            }
            
            if (gps.satellites.isValid()) {
                dogData.satellites = gps.satellites.value();
            }
            
            if (gps.speed.isValid()) {
                float speed = gps.speed.kmph();
                if (speed > dogData.maxSpeed) {
                    dogData.maxSpeed = speed;
                }
            }
        }
    }
}

void checkLocation() {
    if (!dogData.gpsValid) return;
    
    // 脱走検知
    if (dogData.distanceFromHome > ESCAPE_RADIUS) {
        if (!dogData.isEscaped) {
            // 新規脱走検知
            dogData.isEscaped = true;
            dogData.escapeStartTime = millis();
            
            String message = "🚨 緊急通知：" + String(DOG_NAME) + "が自宅から1km以上離れました！\n";
            message += "現在地：https://maps.google.com/maps?q=";
            message += String(dogData.latitude, 6) + "," + String(dogData.longitude, 6);
            
            sendLineNotification(message);
            dogData.lastEscapeNotify = millis();
        } else {
            // 脱走継続中 - 5分ごとに通知
            if (millis() - dogData.lastEscapeNotify > LINE_NOTIFY_INTERVAL) {
                String message = "📍 " + String(DOG_NAME) + "の現在地（脱走継続中）\n";
                message += "自宅から: " + String(dogData.distanceFromHome, 0) + "m\n";
                message += "https://maps.google.com/maps?q=";
                message += String(dogData.latitude, 6) + "," + String(dogData.longitude, 6);
                
                sendLineNotification(message);
                dogData.lastEscapeNotify = millis();
            }
        }
    } else if (dogData.distanceFromHome > WARNING_RADIUS) {
        // 警告距離
        static unsigned long lastWarning = 0;
        if (millis() - lastWarning > 60000) {  // 1分に1回
            String message = "⚠️ " + String(DOG_NAME) + "が自宅から" + 
                           String(dogData.distanceFromHome, 0) + "m離れています";
            sendLineNotification(message);
            lastWarning = millis();
        }
    } else {
        // 自宅範囲内に戻った
        if (dogData.isEscaped) {
            dogData.isEscaped = false;
            String message = "✅ " + String(DOG_NAME) + "が自宅範囲内に戻りました！";
            sendLineNotification(message);
        }
    }
}

// ====================================
// 通信関連
// ====================================
void setupWiFi() {
    Serial.print("Connecting to WiFi");
    M5.Display.println("\nWiFi接続中...");
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        dogData.wifiConnected = true;
        Serial.println("\nWiFi Connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        M5.Display.println("WiFi: OK");
    } else {
        Serial.println("\nWiFi Failed!");
        M5.Display.println("WiFi: NG");
    }
}

void sendLineNotification(String message) {
    if (!dogData.wifiConnected) {
        Serial.println("WiFi not connected, cannot send LINE notification");
        return;
    }
    
    HTTPClient http;
    http.begin("https://api.line.me/v2/bot/message/broadcast");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + String(LINE_CHANNEL_ACCESS_TOKEN));
    
    StaticJsonDocument<512> doc;
    JsonArray messages = doc.createNestedArray("messages");
    JsonObject msg = messages.createNestedObject();
    msg["type"] = "text";
    msg["text"] = message;
    
    String jsonStr;
    serializeJson(doc, jsonStr);
    
    int httpCode = http.POST(jsonStr);
    
    if (httpCode > 0) {
        Serial.print("LINE notification sent: ");
        Serial.println(httpCode);
    } else {
        Serial.print("LINE notification failed: ");
        Serial.println(http.errorToString(httpCode));
    }
    
    http.end();
}

void sendAlert(String title, String message, bool urgent) {
    String alertMsg = title + "\n" + message;
    
    if (urgent) {
        // 警告音
        M5.Speaker.tone(1000, 500);
    }
    
    sendLineNotification(alertMsg);
}

void sendActivitySummary() {
    String age = calculateAge();
    
    String summary = "🐕 " + String(DOG_NAME) + "の活動レポート\n";
    summary += "━━━━━━━━━━━━━━━\n";
    summary += "年齢: " + age + "\n";
    summary += "体重: " + String(DOG_WEIGHT) + "kg\n\n";
    summary += "📊 過去1時間の統計\n";
    summary += "歩数: " + String(dogData.stepCount) + "歩\n";
    summary += "消費カロリー: " + String(dogData.calories, 1) + "kcal\n";
    summary += "活動時間: " + String(dogData.activeMinutes / 60) + "分\n";
    summary += "休息時間: " + String(dogData.restMinutes / 60) + "分\n";
    
    if (dogData.maxSpeed > 0) {
        summary += "最高速度: " + String(dogData.maxSpeed, 1) + "km/h\n";
    }
    
    summary += "\n💊 健康状態: ";
    if (dogData.fallDetected || dogData.tremblingDetected) {
        summary += "要注意";
    } else {
        summary += "良好";
    }
    
    sendLineNotification(summary);
    
    // カウンターリセット
    dogData.stepCount = 0;
    dogData.activeMinutes = 0;
    dogData.restMinutes = 0;
}

// ====================================
// UI関連
// ====================================
void handleButtons() {
    // ボタンA: テスト通知
    if (M5.BtnA.wasPressed()) {
        Serial.println("Test notification triggered");
        String testMsg = "🧪 テスト通知\n";
        testMsg += String(DOG_NAME) + "の現在地:\n";
        
        if (dogData.gpsValid) {
            testMsg += "https://maps.google.com/maps?q=";
            testMsg += String(dogData.latitude, 6) + "," + String(dogData.longitude, 6);
            testMsg += "\n自宅から: " + String(dogData.distanceFromHome, 0) + "m";
        } else {
            testMsg += "GPS信号待機中...";
        }
        
        sendLineNotification(testMsg);
        
        // フィードバック
        M5.Speaker.tone(523, 100);
    }
    
    // ボタンB: 画面切り替え
    if (M5.BtnB.wasPressed()) {
        static int displayMode = 0;
        displayMode = (displayMode + 1) % 3;
        M5.Speaker.tone(659, 50);
    }
}

void updateDisplay() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setCursor(0, 0);
    
    // ヘッダー
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(2);
    M5.Display.print("PochiGuard");
    
    // バッテリー表示
    M5.Display.setCursor(180, 0);
    M5.Display.setTextSize(1);
    if (dogData.batteryLevel > 50) {
        M5.Display.setTextColor(TFT_GREEN);
    } else if (dogData.batteryLevel > 20) {
        M5.Display.setTextColor(TFT_YELLOW);
    } else {
        M5.Display.setTextColor(TFT_RED);
    }
    M5.Display.print(dogData.batteryLevel);
    M5.Display.println("%");
    
    // ノアちゃん情報
    M5.Display.setCursor(0, 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(1);
    M5.Display.print(DOG_NAME);
    M5.Display.print(" (");
    M5.Display.print(calculateAge());
    M5.Display.println(")");
    
    // GPS状態
    M5.Display.setCursor(0, 35);
    if (dogData.gpsValid) {
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.print("GPS: OK (");
        M5.Display.print(dogData.satellites);
        M5.Display.println("衛星)");
        
        // 自宅からの距離
        M5.Display.setCursor(0, 50);
        if (dogData.distanceFromHome > ESCAPE_RADIUS) {
            M5.Display.setTextColor(TFT_RED);
            M5.Display.print("⚠️ 脱走中! ");
        } else if (dogData.distanceFromHome > WARNING_RADIUS) {
            M5.Display.setTextColor(TFT_YELLOW);
            M5.Display.print("注意: ");
        } else {
            M5.Display.setTextColor(TFT_GREEN);
            M5.Display.print("安全: ");
        }
        M5.Display.print(dogData.distanceFromHome, 0);
        M5.Display.println("m");
    } else {
        M5.Display.setTextColor(TFT_YELLOW);
        M5.Display.println("GPS: 検索中...");
    }
    
    // 活動情報
    M5.Display.setCursor(0, 70);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.print("歩数: ");
    M5.Display.println(dogData.dailySteps);
    
    M5.Display.setTextSize(1);
    M5.Display.print("活動: ");
    M5.Display.println(dogData.activityType);
    M5.Display.print("カロリー: ");
    M5.Display.print(dogData.calories, 1);
    M5.Display.println("kcal");
    
    // ボタンガイド
    M5.Display.setCursor(0, 120);
    M5.Display.setTextColor(TFT_DARKGREY);
    M5.Display.println("[A]テスト [B]切替");
}

// ====================================
// ユーティリティ
// ====================================
float calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371000;  // 地球の半径（メートル）
    double dLat = degreesToRadians(lat2 - lat1);
    double dLon = degreesToRadians(lon2 - lon1);
    
    double a = sin(dLat/2) * sin(dLat/2) +
               cos(degreesToRadians(lat1)) * cos(degreesToRadians(lat2)) *
               sin(dLon/2) * sin(dLon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    
    return R * c;
}

double degreesToRadians(double degrees) {
    return degrees * PI / 180.0;
}
