/**
 * Care Copilot - M5Stack Fire Basic
 * M5Unifiedを使った確実に動く基本版
 */

#include <M5Unified.h>

#define DEVICE_ID "M5_FIRE_BASIC"
#define FIRMWARE_VERSION "1.0.0"

void setup() {
    // M5Stack Fire初期化
    auto cfg = M5.config();
    cfg.internal_imu = false;  // IMUも一旦無効
    cfg.internal_rtc = false;  // RTCも無効
    M5.begin(cfg);
    
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== M5Stack Fire Basic ===");
    
    // 画面表示
    M5.Display.setTextSize(2);
    M5.Display.clear();
    M5.Display.println("Care Copilot");
    M5.Display.println("Basic Test");
    
    // 音を鳴らす
    Serial.println("Playing sound...");
    M5.Speaker.tone(523, 200);
    delay(200);
    M5.Speaker.tone(659, 200);
    delay(200);
    M5.Speaker.tone(784, 200);
    delay(200);
    
    M5.Display.println("\nButtons:");
    M5.Display.println("A: Red + Beep");
    M5.Display.println("B: Green");  
    M5.Display.println("C: Blue");
    
    Serial.println("Ready!");
}

void loop() {
    M5.update();
    
    // ボタンA（赤＋音）
    if (M5.BtnA.wasPressed()) {
        Serial.println("Button A");
        M5.Display.clear(TFT_RED);
        M5.Display.setCursor(0, 100);
        M5.Display.println("  Button A");
        M5.Speaker.tone(1000, 100);
        delay(500);
        showMenu();
    }
    
    // ボタンB（緑）
    if (M5.BtnB.wasPressed()) {
        Serial.println("Button B");
        M5.Display.clear(TFT_GREEN);
        M5.Display.setCursor(0, 100);
        M5.Display.println("  Button B");
        delay(500);
        showMenu();
    }
    
    // ボタンC（青）
    if (M5.BtnC.wasPressed()) {
        Serial.println("Button C");
        M5.Display.clear(TFT_BLUE);
        M5.Display.setCursor(0, 100);
        M5.Display.println("  Button C");
        delay(500);
        showMenu();
    }
    
    // 10秒ごとに生存確認
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 10000) {
        Serial.print("Running: ");
        Serial.print(millis()/1000);
        Serial.println(" seconds");
        lastPrint = millis();
    }
    
    delay(10);
}

void showMenu() {
    M5.Display.clear();
    M5.Display.setCursor(0, 0);
    M5.Display.println("Care Copilot");
    M5.Display.println("Basic Test");
    M5.Display.println("\nButtons:");
    M5.Display.println("A: Red + Beep");
    M5.Display.println("B: Green");
    M5.Display.println("C: Blue");
}