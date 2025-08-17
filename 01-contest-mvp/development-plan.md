# 🚀 詳細開発計画書

## 📋 開発環境セットアップ

### 必要なソフトウェア

#### Arduino IDE設定
```bash
# Arduino IDE 2.0以上をインストール
# ボードマネージャーでESP32を追加
https://dl.espressif.com/dl/package_esp32_index.json

# 必要なライブラリ
- M5Stack (by M5Stack)
- M5StickCPlus2 (by M5Stack)
- TinyGPSPlus (GPS処理)
- ArduinoJson (データ処理)
- WiFiManager (WiFi設定)
- HTTPClient (API通信)
```

#### Node.js バックエンド
```bash
# プロジェクト初期化
npm init -y

# 必要なパッケージ
npm install express socket.io
npm install @line/bot-sdk
npm install dotenv cors
npm install node-cron  # 定期処理用

# 開発用
npm install -D nodemon
```

#### Python AI部分
```bash
# 仮想環境作成
python -m venv venv
source venv/bin/activate  # Mac/Linux

# 必要なライブラリ
pip install pandas numpy
pip install scikit-learn
pip install matplotlib
pip install requests
```

## 🔧 Day 1: ハードウェア基礎実装

### センサー接続図

```
M5Stack Fire (Basic V2.7の代替)
├── I2C (Port A)
│   ├── ENV IV Unit (0x44/0x76) - ENV III Unitの代替
│   └── ToF Distance Unit (0x52) - Ultrasonic Unitの代替
├── GPIO (Port B)
│   └── PIR Unit (GPIO 36) - 明後日到着予定
└── UART (Port C)
    └── GPS Module (RX:16, TX:17) - 未購入

M5StickC Plus2 (携帯用) - 明後日到着予定
├── Built-in
│   ├── IMU (加速度/ジャイロ)
│   └── RTC (時刻管理)
└── Hat端子
    └── GPS Module (外出時装着) - 未購入
```

### テストコード例

```cpp
// センサー動作確認
#include <M5Stack.h>  // M5Stack Fireでも同じライブラリを使用
#include <Wire.h>

void setup() {
    M5.begin();
    Wire.begin();
    
    // PIRセンサー初期化
    pinMode(36, INPUT);
    
    M5.Lcd.println("Sensor Test Starting...");
}

void loop() {
    // PIR読み取り
    int motion = digitalRead(36);
    if(motion == HIGH) {
        M5.Lcd.fillScreen(RED);
        M5.Lcd.println("Motion Detected!");
        delay(1000);
    }
    
    M5.update();
    delay(100);
}
```

## 🔔 Day 2: 通知システム実装

### LINE Bot設定手順

1. **LINE Developers登録**
```
1. https://developers.line.biz/
2. プロバイダー作成
3. Messaging APIチャネル作成
4. Channel Access Token取得
5. Webhook URL設定
```

2. **Node.jsサーバー実装**
```javascript
const line = require('@line/bot-sdk');
const express = require('express');

const config = {
    channelAccessToken: process.env.LINE_TOKEN,
    channelSecret: process.env.LINE_SECRET
};

const client = new line.Client(config);
const app = express();

// アラート送信関数
async function sendAlert(level, message, userId) {
    const emoji = ['ℹ️', '⚠️', '🚨', '🆘'];
    const text = `${emoji[level]} ${message}`;
    
    await client.pushMessage(userId, {
        type: 'text',
        text: text
    });
}

app.listen(3000);
```

### 段階的通知ロジック

```cpp
// M5Stack側の判定ロジック
enum AlertLevel {
    INFO = 0,      // 記録のみ
    CAUTION = 1,   // ダッシュボード表示
    WARNING = 2,   // LINE通知
    EMERGENCY = 3  // 即時対応要請
};

AlertLevel evaluateEvent(SensorData data) {
    // 時間帯による重み付け
    int timeWeight = (hour >= 22 || hour <= 6) ? 2 : 1;
    
    // イベントタイプによる基本スコア
    int baseScore = 0;
    if (data.fallDetected) baseScore = 10;
    else if (data.longInactivity) baseScore = 5;
    else if (data.unusualMovement) baseScore = 3;
    
    int finalScore = baseScore * timeWeight;
    
    if (finalScore >= 10) return EMERGENCY;
    if (finalScore >= 6) return WARNING;
    if (finalScore >= 3) return CAUTION;
    return INFO;
}
```

## 🗺️ Day 3: GPS外出支援

### ジオフェンシング実装

```cpp
// 安全エリア定義
struct SafeZone {
    float centerLat;
    float centerLon;
    float radiusMeters;
};

SafeZone homeArea = {35.6812, 139.7671, 500.0};  // 自宅から500m
SafeZone parkArea = {35.6850, 139.7700, 100.0};  // 公園100m

bool isInSafeZone(float lat, float lon) {
    float distance = TinyGPSPlus::distanceBetween(
        lat, lon,
        homeArea.centerLat, homeArea.centerLon
    );
    
    return distance <= homeArea.radiusMeters;
}

// 活動記録
struct WalkingActivity {
    unsigned long startTime;
    unsigned long duration;
    float totalDistance;
    int stepCount;
    int restCount;
};
```

### 歩数カウント（加速度センサー）

```cpp
// M5StickC Plus2の加速度センサー使用
#include <M5StickCPlus2.h>

class StepCounter {
private:
    float lastMagnitude = 0;
    int stepCount = 0;
    unsigned long lastStepTime = 0;
    
public:
    void update() {
        float ax, ay, az;
        M5.Imu.getAccel(&ax, &ay, &az);
        
        float magnitude = sqrt(ax*ax + ay*ay + az*az);
        float delta = magnitude - lastMagnitude;
        
        // 歩行検知閾値
        if (delta > 0.1 && millis() - lastStepTime > 300) {
            stepCount++;
            lastStepTime = millis();
        }
        
        lastMagnitude = magnitude;
    }
    
    int getSteps() { return stepCount; }
};
```

## 📊 Day 4: データ分析とレポート

### QOLメトリクス計算

```python
# Python分析スクリプト
import pandas as pd
import numpy as np
from datetime import datetime, timedelta

class QOLAnalyzer:
    def __init__(self, sensor_data):
        self.data = pd.DataFrame(sensor_data)
        
    def calculate_sleep_score(self):
        """睡眠の質スコア計算"""
        night_data = self.data[
            (self.data['hour'] >= 22) | 
            (self.data['hour'] <= 6)
        ]
        
        # 動きの少なさ = 良い睡眠
        movement_score = 100 - (night_data['motion_count'].mean() * 10)
        
        # 適切な睡眠時間（7-9時間）
        sleep_duration = self._estimate_sleep_duration()
        duration_score = 100 if 7 <= sleep_duration <= 9 else 50
        
        return (movement_score + duration_score) / 2
    
    def generate_daily_report(self):
        """日次レポート生成"""
        return {
            "date": datetime.now().strftime("%Y-%m-%d"),
            "sleep_score": self.calculate_sleep_score(),
            "activity_level": self._calculate_activity(),
            "social_interaction": self._estimate_social(),
            "recommendations": self._generate_recommendations()
        }
    
    def _generate_recommendations(self):
        """改善提案の生成"""
        recommendations = []
        
        if self.calculate_sleep_score() < 60:
            recommendations.append("就寝前の活動を控えめに")
            
        if self._calculate_activity() < 30:
            recommendations.append("午後の散歩時間を増やしましょう")
            
        return recommendations
```

### ダッシュボード表示（M5Stack）

```cpp
// M5Stack画面レイアウト
void drawDashboard() {
    M5.Lcd.fillScreen(BLACK);
    
    // ヘッダー
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.println("Care Copilot Dashboard");
    
    // 居住者ステータス
    drawResidentStatus(10, 50, "田中様", currentStatus);
    drawResidentStatus(10, 100, "佐藤様", currentStatus);
    
    // アラート表示エリア
    drawAlertArea(10, 150);
    
    // 統計サマリー
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 200);
    M5.Lcd.printf("本日のアラート: %d件", alertCount);
}

void drawResidentStatus(int x, int y, String name, Status status) {
    // ステータスに応じた色
    uint16_t color = GREEN;
    if (status == WARNING) color = YELLOW;
    if (status == EMERGENCY) color = RED;
    
    M5.Lcd.fillCircle(x, y, 5, color);
    M5.Lcd.setCursor(x + 15, y - 5);
    M5.Lcd.print(name);
}
```

## 🎬 Day 5: 統合とUI仕上げ

### システム統合フロー

```
センサーデータ収集（100ms間隔）
    ↓
イベント検出
    ↓
緊急度判定（AI/ルールベース）
    ↓
通知振り分け
    ├→ INFO: ログ記録のみ
    ├→ CAUTION: ダッシュボード更新
    ├→ WARNING: LINE通知
    └→ EMERGENCY: 全チャネル通知
```

### WebSocket リアルタイム通信

```javascript
// サーバー側
const io = require('socket.io')(server);

io.on('connection', (socket) => {
    console.log('M5Stack connected');
    
    socket.on('sensor_data', (data) => {
        // データ処理
        const processed = processData(data);
        
        // ダッシュボードに配信
        io.emit('dashboard_update', processed);
        
        // 必要に応じてLINE通知
        if (processed.alertLevel >= 2) {
            sendLineAlert(processed);
        }
    });
});

// M5Stack側（WiFiClient使用）
void sendSensorData(String jsonData) {
    if (client.connect(serverIP, serverPort)) {
        client.println("POST /api/sensor HTTP/1.1");
        client.println("Content-Type: application/json");
        client.println("Content-Length: " + String(jsonData.length()));
        client.println();
        client.println(jsonData);
    }
}
```

## 📹 Day 6: デモ準備

### デモ動画構成（3分）

```
0:00-0:30 問題提起
- アラート疲れの実例
- プライバシー問題
- 外出制限の現状

0:30-1:30 ソリューション紹介
- システム構成説明
- 3つのコア機能
- 差別化ポイント

1:30-2:30 実演
- 通常時：静かな見守り
- 異常時：的確な通知
- 外出時：自立支援

2:30-3:00 インパクトと将来
- 対象市場規模
- コスト削減効果
- 拡張可能性
```

### トラブルシューティング

| 問題 | 対処法 |
|------|--------|
| WiFi接続失敗 | スマホテザリング使用 |
| センサー誤動作 | 閾値を現場で調整 |
| GPS精度不足 | 屋外でデモ実施 |
| バッテリー不足 | モバイルバッテリー常備 |
| LINE通知遅延 | ローカルアラート併用 |

## 📝 最終チェックリスト

### 技術面
- [ ] 全センサー動作確認
- [ ] 通信安定性テスト
- [ ] エラーハンドリング実装
- [ ] バッテリー持続時間測定

### デモ面
- [ ] 3つのシナリオ完走
- [ ] 動画撮影（複数テイク）
- [ ] 英語説明準備
- [ ] バックアップ動画準備

### 投稿面
- [ ] Hackster.io アカウント作成
- [ ] プロジェクト画像準備
- [ ] 回路図作成
- [ ] コード整理とコメント

---

## 🎯 成功指標

**技術的完成度より「伝わる」ことを重視**

1. 課題が明確に伝わる
2. 解決策が革新的
3. 実現可能性が見える
4. 社会的価値が大きい

---

*Let's make it happen! 🚀*