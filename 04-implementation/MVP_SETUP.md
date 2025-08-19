# 🚀 M5Stack Fire MVP セットアップガイド

## 📋 前提条件
- M5Stack Fire（接続確認済み✅）
- Arduino IDE 2.3.6（インストール済み✅）
- USB-Cケーブル（データ転送対応）

## 🔧 セットアップ手順

### 1. Arduino IDEの準備

#### ボードマネージャの設定
1. **Arduino IDE** → **設定**（macOS: Arduino IDE → Preferences）
2. **追加のボードマネージャのURL**に以下を追加：
```
https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json
```

#### M5Stackボードのインストール
1. **ツール** → **ボード** → **ボードマネージャ**
2. 「M5Stack」を検索
3. **M5Stack by M5Stack official**をインストール

### 2. 必要なライブラリのインストール

**ツール** → **ライブラリを管理**で以下を検索してインストール：

| ライブラリ名 | 用途 | 必須 |
|------------|------|------|
| **M5Unified** | M5Stack統合ライブラリ | ✅ |
| **M5GFX** | グラフィックライブラリ（M5Unifiedに含まれる） | 自動 |
| **PubSubClient** | MQTT通信 | ✅ |
| **ArduinoJson** | JSON処理 | ✅ |
| **FastLED** | LED Bar制御（Fire専用） | オプション |

### 3. ファームウェアの設定

`m5stack-fire-unified.ino`を開いて、以下を変更：

```cpp
// WiFi設定 - 必ず変更してください！
const char* ssid = "YOUR_WIFI_SSID";        // あなたのWiFi名
const char* password = "YOUR_WIFI_PASSWORD"; // あなたのWiFiパスワード
```

### 4. Arduino IDE設定

**ツール**メニューで以下を設定：

| 項目 | 設定値 |
|------|--------|
| ボード | **M5Stack-FIRE** |
| Upload Speed | **921600**（エラーが出たら115200） |
| CPU Frequency | **240MHz (WiFi/BT)** |
| Flash Frequency | **80MHz** |
| Flash Mode | **QIO** |
| Flash Size | **16MB (3MB APP/9.9MB FATFS)** |
| Partition Scheme | **Default (2 x 6.5MB app, 3.6MB SPIFFS)** |
| Core Debug Level | **None** |
| PSRAM | **Enabled** |
| ポート | **/dev/cu.wchusbserial588D0332551** |

### 5. アップロード

1. **マイコンボードに書き込む**ボタン（→）をクリック
2. コンパイルとアップロードを待つ（初回は2-3分）
3. 「ボードへの書き込みが完了しました」を確認

## 🎮 動作確認

### 起動時の動作
1. 画面に「Care Copilot Fire」が表示
2. WiFi接続試行（約10秒）
3. 起動音が鳴る（ド・ミ・ソ）

### ボタン操作
- **ボタンA**（左）: テストアラート送信
- **ボタンB**（中）: アラートクリア
- **ボタンC**（右）: システム情報表示

### 画面表示
```
Care Copilot
WiFi: OK/NG
MQTT: OK/NG

--- Sensor ---
Accel: 1.00
Temp: 25.0 C
Battery: 100%

--- Alert ---
正常
```

## 🔍 トラブルシューティング

### アップロードエラーが出る場合

#### エラー: "A fatal error occurred: Failed to connect to ESP32"
- **Upload Speed**を**115200**に下げる
- USBケーブルを交換
- M5Stack Fireをリセット（電源ボタン2回押し）

#### エラー: "No module named 'serial'"
```bash
pip install pyserial
```

#### ポートが見つからない
1. M5Stack Fireを抜き差し
2. Arduino IDEを再起動
3. **ツール** → **ポート**を再確認

### WiFi接続できない場合
- SSID/パスワードの確認（大文字小文字に注意）
- 2.4GHz帯のWiFiを使用（5GHzは非対応）
- モバイルホットスポットで代用

### 画面が表示されない場合
- M5Stack Fireの電源を確認
- USBケーブルでの給電を確認
- リセットボタンを押す

## 📊 MQTTモニタリング（オプション）

### MQTT Explorerでのモニタリング
1. [MQTT Explorer](http://mqtt-explorer.com/)をダウンロード
2. 接続設定：
   - Host: `test.mosquitto.org`
   - Port: `1883`
3. トピックを監視：
   - `care/sensor/data` - センサーデータ
   - `care/alert` - アラート
   - `care/status` - ステータス

## 🎯 次のステップ

### MVP機能の実装優先順位

1. **基本動作確認**（今すぐ）
   - WiFi接続
   - MQTT通信
   - IMUセンサー読み取り

2. **転倒検知**（Day 1）
   - 加速度による転倒判定
   - アラート送信
   - LED/音での通知

3. **環境モニタリング**（Day 2）
   - 温度センサー追加
   - しきい値設定
   - 定期データ送信

4. **外部センサー連携**（Day 3）
   - PIRセンサー接続
   - 距離センサー接続
   - マルチセンサー統合

## 📱 スマートフォンでのテスト

### シリアルモニタの代替
スマートフォンからのモニタリング：
1. **Serial Bluetooth Terminal**アプリをインストール
2. M5Stack FireのBluetoothをON
3. ペアリングして接続

## ✅ チェックリスト

- [ ] M5Unifiedライブラリをインストール
- [ ] WiFi設定を変更
- [ ] ボード設定を「M5Stack-FIRE」に
- [ ] ポートを正しく選択
- [ ] ファームウェアをアップロード
- [ ] 起動音を確認
- [ ] WiFi接続を確認
- [ ] ボタン動作を確認

---

**問題が発生した場合は、エラーメッセージと共にお知らせください！**