# 🔧 技術仕様書 v1.0

## 📋 システム概要

### アーキテクチャ概要

```
┌─────────────────────────────────────────────────┐
│                  利用者環境                       │
├─────────────┬───────────┬───────────────────────┤
│  居室A      │   居室B    │      共用スペース       │
│ M5StickC+   │ M5StickC+  │    M5Stack Basic     │
│ +センサー    │ +センサー   │    （管理端末）        │
└──────┬──────┴─────┬──────┴───────────┬──────────┘
       │            │                   │
       └────────────┼───────────────────┘
                    │ WiFi/MQTT
                    ▼
┌─────────────────────────────────────────────────┐
│              エッジサーバー（施設内）               │
│  ・データ集約  ・一次処理  ・ローカルアラート       │
└─────────────────────┬───────────────────────────┘
                      │ HTTPS/WebSocket
                      ▼
┌─────────────────────────────────────────────────┐
│                クラウドサービス                    │
│  ・AI分析  ・データ蓄積  ・API  ・通知配信         │
└─────────────────────────────────────────────────┘
                      │
       ┌──────────────┼──────────────┐
       ▼              ▼              ▼
   LINE Bot    Web Dashboard   Mobile App
```

## 🎮 ハードウェア仕様

### M5Stack Basic（管理端末）

**基本スペック**
```
CPU: ESP32 240MHz dual core
RAM: 520KB SRAM
Flash: 16MB
Display: 2.0" 320x240 TFT LCD
Connectivity: WiFi 802.11b/g/n, Bluetooth 4.2
Power: 110mAh battery + USB-C
```

**接続センサー**
```
I2C (Port A):
- Address 0x44: SHT30 (温湿度)
- Address 0x76: BMP280 (気圧)

GPIO (Port B):
- Pin 36: PIR Motion Sensor
- Pin 26: LED Indicator

UART (Port C):
- RX16/TX17: GPS Module
```

### M5StickC Plus2（モバイル端末）

**基本スペック**
```
CPU: ESP32-PICO-D4
RAM: 520KB
Flash: 4MB
Display: 1.14" 135x240 TFT
Battery: 120mAh
IMU: MPU6886 (6軸)
RTC: BM8563
Buzzer: Built-in
```

**用途別設定**
```
居室モード:
- 加速度による転倒検知
- 定期的な環境測定
- 省電力動作（8時間）

外出モード:
- GPS追跡（外付け）
- 歩数カウント
- SOS機能
```

### センサー仕様

#### PIR Unit（人感センサー）
```
検知方式: 焦電型赤外線
検知範囲: 100° × 90°
検知距離: 最大5m
応答時間: 0.5秒
消費電力: 0.1mA（待機時）
```

#### Ultrasonic Distance Unit
```
測定範囲: 20cm - 450cm
精度: ±1cm
分解能: 1mm
応答時間: 60ms
用途: 離床検知、通過検知
```

#### ENV III Unit
```
温度センサー (SHT30):
- 範囲: -40°C to +125°C
- 精度: ±0.2°C

湿度センサー:
- 範囲: 0-100% RH
- 精度: ±2% RH

気圧センサー (QMP6988):
- 範囲: 300-1100 hPa
- 精度: ±0.06 hPa
```

## 💾 ソフトウェアアーキテクチャ

### エッジ側（M5Stack/ESP32）

#### ファームウェア構成
```cpp
// main.ino
#include <M5Stack.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

class CareCopaiotDevice {
private:
    // センサー管理
    SensorManager sensors;
    
    // 通信管理
    NetworkManager network;
    
    // イベント処理
    EventProcessor processor;
    
    // ローカルストレージ
    DataLogger logger;
    
public:
    void setup();
    void loop();
    void handleEmergency();
};
```

#### データフォーマット
```json
{
  "device_id": "M5_ROOM_001",
  "timestamp": 1629350400,
  "type": "sensor_data",
  "data": {
    "motion": true,
    "distance": 120,
    "temperature": 25.3,
    "humidity": 60.5,
    "pressure": 1013.25,
    "battery": 85
  },
  "alert_level": 0
}
```

### クラウド側アーキテクチャ

#### 技術スタック
```
Backend:
- Runtime: Node.js 18 LTS
- Framework: Express.js
- WebSocket: Socket.io
- Database: PostgreSQL + TimescaleDB
- Cache: Redis
- Queue: Bull (Redis-based)

AI/ML:
- Language: Python 3.9
- Framework: FastAPI
- ML: scikit-learn, TensorFlow
- Analysis: Pandas, NumPy

Infrastructure:
- Cloud: AWS / Azure
- Container: Docker
- Orchestration: Kubernetes
- CI/CD: GitHub Actions
```

#### API設計

**RESTful API**
```yaml
# Facility Management
GET    /api/v1/facilities
POST   /api/v1/facilities
GET    /api/v1/facilities/{id}
PUT    /api/v1/facilities/{id}
DELETE /api/v1/facilities/{id}

# Resident Management
GET    /api/v1/residents
POST   /api/v1/residents
GET    /api/v1/residents/{id}
GET    /api/v1/residents/{id}/status
GET    /api/v1/residents/{id}/history

# Alert Management
GET    /api/v1/alerts
POST   /api/v1/alerts
GET    /api/v1/alerts/{id}
PUT    /api/v1/alerts/{id}/acknowledge

# Analytics
GET    /api/v1/analytics/summary
GET    /api/v1/analytics/qol-report
GET    /api/v1/analytics/predictions
```

**WebSocket Events**
```javascript
// Server → Client
socket.emit('sensor_update', data);
socket.emit('alert_triggered', alert);
socket.emit('status_change', status);

// Client → Server
socket.on('subscribe_room', roomId);
socket.on('acknowledge_alert', alertId);
socket.on('request_history', params);
```

### データベース設計

#### PostgreSQL Schema
```sql
-- 施設テーブル
CREATE TABLE facilities (
    id UUID PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    address TEXT,
    capacity INTEGER,
    created_at TIMESTAMP DEFAULT NOW()
);

-- 居住者テーブル
CREATE TABLE residents (
    id UUID PRIMARY KEY,
    facility_id UUID REFERENCES facilities(id),
    name VARCHAR(255) NOT NULL,
    room_number VARCHAR(50),
    care_level INTEGER,
    risk_factors JSONB,
    created_at TIMESTAMP DEFAULT NOW()
);

-- センサーデータ（時系列）
CREATE TABLE sensor_data (
    time TIMESTAMPTZ NOT NULL,
    device_id VARCHAR(100),
    resident_id UUID,
    data JSONB,
    PRIMARY KEY (time, device_id)
);

-- TimescaleDB設定
SELECT create_hypertable('sensor_data', 'time');
```

#### Redis データ構造
```
# リアルタイムステータス
resident:{id}:status → JSON
facility:{id}:alerts → List
device:{id}:last_seen → Timestamp

# キャッシュ
cache:qol_report:{id} → JSON (TTL: 3600)
cache:analytics:{date} → JSON (TTL: 86400)

# セッション
session:{token} → User Data (TTL: 7200)
```

## 🤖 AI/機械学習

### 異常検知アルゴリズム

#### アプローチ1: ルールベース
```python
class RuleBasedDetector:
    def detect_anomaly(self, data):
        score = 0
        
        # 時間帯による重み付け
        hour = data['timestamp'].hour
        if 22 <= hour or hour <= 6:
            time_weight = 2.0
        else:
            time_weight = 1.0
        
        # 転倒検知
        if data['acceleration_spike'] > 3.0:
            score += 10
        
        # 長時間不動
        if data['no_motion_duration'] > 3600:
            score += 5
        
        # 環境異常
        if data['temperature'] > 35 or data['temperature'] < 15:
            score += 3
        
        return score * time_weight
```

#### アプローチ2: 機械学習
```python
from sklearn.ensemble import IsolationForest
import numpy as np

class MLAnomalyDetector:
    def __init__(self):
        self.model = IsolationForest(
            contamination=0.1,
            random_state=42
        )
        
    def train(self, historical_data):
        features = self.extract_features(historical_data)
        self.model.fit(features)
        
    def predict(self, current_data):
        features = self.extract_features([current_data])
        anomaly_score = self.model.decision_function(features)[0]
        is_anomaly = self.model.predict(features)[0] == -1
        return is_anomaly, anomaly_score
        
    def extract_features(self, data):
        # 特徴量抽出
        return np.array([
            d['motion_frequency'],
            d['average_distance'],
            d['temperature_variance'],
            d['time_since_last_motion']
        ] for d in data)
```

### 行動パターン学習

```python
class BehaviorPatternLearner:
    def __init__(self):
        self.patterns = {}
        
    def learn_daily_pattern(self, resident_id, days=30):
        """日常パターンの学習"""
        data = self.fetch_historical_data(resident_id, days)
        
        pattern = {
            'wake_time': self.extract_wake_time(data),
            'sleep_time': self.extract_sleep_time(data),
            'activity_peaks': self.find_activity_peaks(data),
            'bathroom_frequency': self.calc_bathroom_freq(data),
            'meal_times': self.extract_meal_times(data)
        }
        
        self.patterns[resident_id] = pattern
        return pattern
        
    def detect_deviation(self, resident_id, current_behavior):
        """パターンからの逸脱検知"""
        if resident_id not in self.patterns:
            return False, 0
            
        pattern = self.patterns[resident_id]
        deviation_score = 0
        
        # 起床時間の逸脱
        wake_diff = abs(current_behavior['wake_time'] - 
                       pattern['wake_time'].total_seconds())
        if wake_diff > 3600:  # 1時間以上
            deviation_score += wake_diff / 3600
            
        return deviation_score > 2, deviation_score
```

## 🔒 セキュリティ仕様

### データ保護

#### 暗号化
```
通信: TLS 1.3
保存: AES-256-GCM
鍵管理: AWS KMS / Azure Key Vault
```

#### 認証・認可
```javascript
// JWT認証
const jwt = require('jsonwebtoken');

function generateToken(user) {
    return jwt.sign(
        {
            id: user.id,
            facility_id: user.facility_id,
            role: user.role
        },
        process.env.JWT_SECRET,
        { expiresIn: '24h' }
    );
}

// ロールベースアクセス制御
const permissions = {
    admin: ['read', 'write', 'delete', 'manage'],
    staff: ['read', 'write', 'acknowledge'],
    viewer: ['read']
};
```

### プライバシー設計

#### データ最小化
```
収集データ:
✓ 必要最小限のセンサー情報
✓ 統計化された行動パターン
✗ 個人を特定する画像・音声
✗ 不必要な位置情報
```

#### データ保持ポリシー
```
リアルタイムデータ: 24時間
集計データ: 90日
統計データ: 1年
個人識別情報: 削除可能
```

## 📊 パフォーマンス仕様

### システム要件

#### レスポンスタイム
```
センサー → エッジ: < 100ms
エッジ → クラウド: < 500ms
アラート通知: < 3秒
ダッシュボード更新: < 1秒
```

#### スケーラビリティ
```
同時接続デバイス: 10,000台
データ処理: 100,000 events/秒
ストレージ: 10TB/年
API呼び出し: 1,000 req/秒
```

#### 可用性
```
システム稼働率: 99.9%
計画停止: 月1回、深夜2時間
災害復旧時間: < 4時間
データバックアップ: 日次
```

## 🔄 インテグレーション

### 外部システム連携

#### 介護記録システム
```javascript
// CAREKARTE連携例
class CarekarteIntegration {
    async syncEvent(event) {
        const payload = {
            timestamp: event.timestamp,
            resident_id: this.mapResidentId(event.resident_id),
            event_type: this.mapEventType(event.type),
            details: event.data
        };
        
        return await axios.post(
            'https://api.carekarte.jp/v1/events',
            payload,
            { headers: this.getAuthHeaders() }
        );
    }
}
```

#### LINE Messaging API
```javascript
const line = require('@line/bot-sdk');

class LineNotification {
    constructor() {
        this.client = new line.Client({
            channelAccessToken: process.env.LINE_TOKEN
        });
    }
    
    async sendAlert(userId, alert) {
        const message = {
            type: 'flex',
            altText: `緊急度${alert.level}: ${alert.message}`,
            contents: this.createFlexMessage(alert)
        };
        
        return await this.client.pushMessage(userId, message);
    }
}
```

## 🧪 テスト仕様

### テスト戦略

#### ユニットテスト
```javascript
// Jest使用例
describe('AlertProcessor', () => {
    test('緊急アラートが正しく判定される', () => {
        const processor = new AlertProcessor();
        const data = {
            motion: false,
            duration: 7200,
            time: '02:30'
        };
        
        const result = processor.evaluate(data);
        expect(result.level).toBe(4);
        expect(result.notify).toBe(true);
    });
});
```

#### 統合テスト
```python
# pytest使用例
def test_end_to_end_alert_flow():
    # センサーデータ送信
    response = client.post('/api/sensor', json=test_data)
    assert response.status_code == 200
    
    # アラート生成確認
    alert = Alert.query.filter_by(
        device_id=test_data['device_id']
    ).first()
    assert alert is not None
    
    # 通知送信確認
    assert mock_line_api.called
```

### 品質基準

```
コードカバレッジ: > 80%
バグ密度: < 1 bug/KLOC
MTBF: > 720時間
MTTR: < 1時間
```

## 📦 デプロイメント

### Docker構成
```dockerfile
# Backend Dockerfile
FROM node:18-alpine
WORKDIR /app
COPY package*.json ./
RUN npm ci --only=production
COPY . .
EXPOSE 3000
CMD ["node", "server.js"]
```

### Kubernetes設定
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: care-copilot-api
spec:
  replicas: 3
  selector:
    matchLabels:
      app: care-copilot-api
  template:
    metadata:
      labels:
        app: care-copilot-api
    spec:
      containers:
      - name: api
        image: care-copilot/api:latest
        ports:
        - containerPort: 3000
        env:
        - name: NODE_ENV
          value: "production"
```

## 📈 モニタリング

### メトリクス収集
```
Prometheus設定:
- システムメトリクス
- アプリケーションメトリクス
- カスタムメトリクス

Grafanaダッシュボード:
- リアルタイムステータス
- アラート統計
- パフォーマンス指標
```

### ログ管理
```
ELKスタック:
- Elasticsearch: ログ保存
- Logstash: ログ収集
- Kibana: ログ分析

ログレベル:
- ERROR: システムエラー
- WARN: 警告事項
- INFO: 通常操作
- DEBUG: デバッグ情報
```

---

## 🔄 更新履歴

| バージョン | 日付 | 変更内容 |
|-----------|------|---------|
| 1.0.0 | 2025-08-16 | 初版作成 |
| 1.1.0 | (予定) | GPS機能追加 |
| 2.0.0 | (予定) | AI機能強化 |

---

*Technical Excellence for Human Dignity*