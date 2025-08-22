/**
 * PochiGuard Configuration
 * 犬用GPSトラッカー＆活動量計
 * For M5StickC Plus2
 */

#ifndef CONFIG_H
#define CONFIG_H

// ====================================
// 犬のプロフィール設定
// ====================================
#define DOG_NAME "ノア"           // 愛犬の名前
#define DOG_BREED "チワワ"        // 犬種
#define DOG_WEIGHT 2.1           // 体重(kg)
#define DOG_BIRTH_YEAR 2023      // 誕生年
#define DOG_BIRTH_MONTH 9        // 誕生月
#define DOG_BIRTH_DAY 28         // 誕生日
// 年齢は自動計算されます（2025年8月22日現在：1歳10ヶ月）

// ====================================
// 自宅位置設定（福岡市早良区室見）
// ====================================
#define HOME_LATITUDE 33.581452   // 自宅の緯度
#define HOME_LONGITUDE 130.3423642 // 自宅の経度
#define ESCAPE_RADIUS 1000.0       // 脱走判定距離（メートル）= 1km
#define WARNING_RADIUS 500.0       // 警告距離（メートル）= 500m

// ====================================
// WiFi設定（config_local.hで上書き）
// ====================================
#ifndef WIFI_SSID
#define WIFI_SSID "your-wifi-ssid"
#define WIFI_PASSWORD "your-wifi-password"
#endif

// ====================================
// MQTT設定
// ====================================
#ifndef MQTT_SERVER
#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASS ""
#endif

// ====================================
// デバイス識別
// ====================================
#define DEVICE_ID "PochiGuard_Noah"
#define FIRMWARE_VERSION "1.0.0"

// ====================================
// LINE Messaging API設定
// ====================================
#ifndef LINE_CHANNEL_ACCESS_TOKEN
#define LINE_CHANNEL_ACCESS_TOKEN "your-line-channel-access-token"
#define LINE_USER_ID "your-line-user-id"
#endif

// ====================================
// GPS設定（M5StickC Plus2 Grove Port）
// ====================================
#define GPS_BAUD_RATE 9600
#define GPS_RX_PIN 32  // Grove Port G32
#define GPS_TX_PIN 33  // Grove Port G33

// ====================================
// 活動検知しきい値（チワワ用に最適化）
// ====================================
#define STEP_THRESHOLD 1.15        // 歩数検知しきい値（小型犬用に調整）
#define RUN_THRESHOLD 1.8          // 走行判定しきい値
#define FALL_THRESHOLD 2.5         // 転倒検知しきい値
#define TREMBLING_THRESHOLD 0.05   // 震え検知しきい値
#define REST_THRESHOLD 0.3         // 休息判定しきい値

// ====================================
// カロリー計算パラメータ（チワワ2.1kg用）
// ====================================
#define CALORIE_FACTOR 0.03        // 小型犬用カロリー係数
#define BASAL_METABOLIC_RATE 70    // 基礎代謝率（kcal/日）

// ====================================
// タイミング設定（ミリ秒）
// ====================================
#define IMU_READ_INTERVAL 50       // IMU読み取り間隔（歩数検知用）
#define GPS_READ_INTERVAL 1000     // GPS読み取り間隔
#define LOCATION_CHECK_INTERVAL 5000  // 位置チェック間隔（5秒）
#define MQTT_PUBLISH_INTERVAL 10000   // MQTT送信間隔（10秒）
#define LINE_NOTIFY_INTERVAL 300000   // 脱走時の通知間隔（5分）
#define DISPLAY_UPDATE_INTERVAL 500   // 画面更新間隔
#define ACTIVITY_SUMMARY_INTERVAL 3600000  // 活動サマリー送信間隔（1時間）

// ====================================
// アラートレベル定義
// ====================================
#define ALERT_INFO 0       // 情報
#define ALERT_CAUTION 1    // 注意
#define ALERT_WARNING 2    // 警告
#define ALERT_EMERGENCY 3  // 緊急

// ====================================
// デバッグ設定
// ====================================
#define DEBUG_MODE true    // シリアル出力の有効/無効
#define DEBUG_GPS true     // GPS詳細デバッグ
#define DEBUG_IMU false    // IMU詳細デバッグ

#endif // CONFIG_H
