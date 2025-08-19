# Care Copilot M5Stack - 実装

## 📁 実装パターン

### ✅ 2つの構成

#### 1️⃣ **Basic版** (`m5stack-fire-basic/`)
**最小構成・コンセプト実証用**
- **センサー**: IMU（内蔵）のみ
- **機能**: 転倒検知、基本的な動き検出、WiFi/MQTT通信
- **用途**: プロトタイプ、動作確認、最小コスト構成
- **必要機器**: M5Stack Fireのみ（¥8,000）

#### 2️⃣ **Unified All版** (`m5stack-fire-unified-all/`)
**全センサー統合・コンテストデモ版**
- **センサー**: IMU + ToF4M + ENV.4 + GPS
- **機能**: 
  - 転倒検知（IMU）
  - 離床・接近検知（ToF: 〜4m）
  - 温湿度・気圧監視（ENV.4）
  - 位置追跡・徘徊検知（GPS）
- **用途**: M5Stackコンテストデモ、総合的な見守りシステム
- **接続**: 
  - Port A: ToF4M + ENV.4（I2Cハブ使用）
  - Port C: GPSモジュール
- **コスト**: 約¥20,000（全センサー含む）

### 📝 その他
- **`backend-server.js`** - Node.jsバックエンドサーバー
- **`MVP_SETUP.md`** - MVPセットアップ手順

## 🚀 使用手順

### 1. Basic版で動作確認
```cpp
// m5stack-fire-basic/m5stack-fire-basic.ino
```
- M5Stack Fireのみで動作
- 転倒検知、WiFi/MQTT接続テスト
- センサー購入前のプロトタイプに最適

### 2. Unified All版で全機能動作
```cpp
// m5stack-fire-unified-all/m5stack-fire-unified-all.ino
```
- 全センサー同時動作
- I2Cアドレス: ToF(0x29), SHT4X(0x44), BMP280(0x76)
- GPS自動ボーレート検出対応

## 📊 必要なライブラリ

### 共通
- **M5Unified** (必須)
- **PubSubClient** (MQTT用)
- **ArduinoJson** (JSON処理用)

### Unified All版追加
- **M5Unit-ENV** (ENV.4用)
- **VL53L1X** (ToF4M用)
- **TinyGPSPlus** (GPS用)

## ⚙️ Arduino IDE設定

- ボードマネージャーURL:
  ```
  https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json
  ```
- ボード: **M5Stack-Fire**
- Upload Speed: **921600**
- Flash Frequency: **80MHz**

## 📡 接続方法

### Unified All版の配線
- **Port A（赤コネクタ）**: I2Cハブ経由でToF4MとENV.4を同時接続
- **Port C（青コネクタ）**: GPSモジュール

※ Port BはI2C互換性問題のため使用しません

## 📊 動作確認

### シリアルモニタ出力例
```
=== Setup Complete ===
Sensors status:
- ToF: OK
- ENV.4: OK
- GPS: Detected

Distance: 1991mm [NORMAL]
Temp: 31.3°C, Humidity: 72.4%
GPS: Lat=33.59348, Lng=130.40200, Sat=13
```

## ✅ 動作確認済み状態（2025/08/19）

### Unified All版テスト結果
- **ToF**: ✅ 正常動作（〜4m測定可能）
- **ENV.4**: ✅ 温湿度・気圧正常取得
- **GPS**: ✅ 13個の衛星で位置取得成功
- **IMU**: ✅ 転倒検知正常
- **MQTT**: ✅ test.mosquitto.org への送信確認

## 📡 MQTT確認

```bash
mosquitto_sub -h test.mosquitto.org -t "care/#" -v
```

## ⚠️ トラブルシューティング

### GPS取得できない場合
- 屋外または窓際でテスト（初回Fix: 1-3分）
- TX/RXの配線を確認（クロス接続）
- 自動ボーレート検出を待つ

### I2Cエラーが出る場合
- I2Cハブの接続確認
- Port A（GPIO21/22）使用を確認
- Port Bは使用不可（GPIO36問題）

### WiFi接続失敗
- 2.4GHz帯のSSIDを使用（5GHz非対応）
- パスワード確認

## 🏆 M5Stack Contest 2025

本プロジェクトはM5Stack Global Innovation Contest 2025に向けて開発中です。
- **締切**: 2025/08/22
- **デモ**: Unified All版でフル機能を実証
- **特徴**: カメラ不使用のプライバシー重視設計