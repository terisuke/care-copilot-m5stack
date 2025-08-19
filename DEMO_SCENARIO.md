# Care Copilot デモシナリオ

## 🎯 デモ概要（5分間）

M5Stack Fireを使った高齢者見守りシステムの実演

## 📊 現在利用可能な機能

### ✅ 実装済み機能
1. **転倒検知**（IMU）
2. **環境モニタリング**（温湿度・気圧）
3. **位置追跡**（GPS）
4. **リアルタイム通信**（MQTT）
5. **アラート通知**

## 🎬 デモシナリオ

### 1️⃣ **システム紹介**（1分）
- M5Stack Fireの紹介
- センサー構成の説明
  - ENV.4：温湿度・気圧センサー
  - GPS：位置追跡
  - IMU：転倒検知

### 2️⃣ **通常モニタリング**（1分）
- 画面表示の説明
  - リアルタイム温湿度表示
  - GPS位置情報
  - 接続状態
- MQTTデータ受信デモ
  ```bash
  mosquitto_sub -h test.mosquitto.org -t "care/#" -v
  ```

### 3️⃣ **環境アラート実演**（1分）
- 温度異常のシミュレーション
  - センサーに手を当てて温度上昇
  - 35°C超過でアラート発生
- 画面に警告表示
- MQTTでアラート送信

### 4️⃣ **転倒検知実演**（1分）
- デバイスを振って転倒をシミュレート
- 即座にレベル3緊急アラート
- 音声警告
- MQTT緊急通知

### 5️⃣ **GPS追跡デモ**（1分）
- 現在地表示
- 外出時の見守り機能説明
- ジオフェンス機能の説明（実装予定）

## 📱 デモ環境セットアップ

### 必要機材
- M5Stack Fire（センサー接続済み）
- ノートPC（MQTT受信表示用）
- WiFiテザリング（スマートフォン）
- プロジェクター（オプション）

### 事前準備
1. WiFi設定確認
2. MQTTブローカー接続確認
3. GPS衛星捕捉（窓際で5分）
4. バッテリー充電

## 🖥️ データ表示方法

### オプション1：ターミナル表示
```bash
# MQTTデータをリアルタイム表示
mosquitto_sub -h test.mosquitto.org -t "care/#" -v | jq .
```

### オプション2：簡易Webダッシュボード
```html
<!DOCTYPE html>
<html>
<head>
    <title>Care Copilot Dashboard</title>
    <script src="https://unpkg.com/mqtt/dist/mqtt.min.js"></script>
    <style>
        body { font-family: Arial; padding: 20px; background: #f0f0f0; }
        .card { background: white; padding: 20px; margin: 10px; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
        .value { font-size: 2em; color: #2196F3; }
        .alert { background: #ff5252; color: white; }
        .normal { background: #4CAF50; color: white; }
    </style>
</head>
<body>
    <h1>Care Copilot リアルタイムモニタリング</h1>
    
    <div class="card">
        <h2>環境データ</h2>
        <p>温度: <span id="temp" class="value">--</span> °C</p>
        <p>湿度: <span id="humidity" class="value">--</span> %</p>
        <p>気圧: <span id="pressure" class="value">--</span> hPa</p>
    </div>
    
    <div class="card">
        <h2>位置情報</h2>
        <p>緯度: <span id="lat" class="value">--</span></p>
        <p>経度: <span id="lng" class="value">--</span></p>
        <p>衛星数: <span id="sat" class="value">--</span></p>
    </div>
    
    <div id="alert" class="card normal">
        <h2>アラート状態</h2>
        <p id="alertText">正常</p>
    </div>
    
    <script>
        const client = mqtt.connect('wss://test.mosquitto.org:8081');
        
        client.on('connect', function () {
            console.log('Connected to MQTT');
            client.subscribe('care/#');
        });
        
        client.on('message', function (topic, message) {
            const data = JSON.parse(message.toString());
            
            if (topic === 'care/sensor/data') {
                // 環境データ更新
                if (data.environment) {
                    document.getElementById('temp').textContent = data.environment.temperature.toFixed(1);
                    document.getElementById('humidity').textContent = data.environment.humidity.toFixed(1);
                    document.getElementById('pressure').textContent = (data.environment.pressure/100).toFixed(1);
                }
                
                // GPS更新
                if (data.gps) {
                    document.getElementById('lat').textContent = data.gps.latitude.toFixed(6);
                    document.getElementById('lng').textContent = data.gps.longitude.toFixed(6);
                    document.getElementById('sat').textContent = data.gps.satellites;
                }
            }
            
            if (topic === 'care/alert') {
                const alertDiv = document.getElementById('alert');
                const alertText = document.getElementById('alertText');
                
                if (data.level >= 2) {
                    alertDiv.className = 'card alert';
                    alertText.textContent = 'アラート: ' + data.message;
                } else {
                    alertDiv.className = 'card normal';
                    alertText.textContent = '正常';
                }
            }
        });
    </script>
</body>
</html>
```

## 💡 デモのポイント

### 強調すべき点
1. **プライバシー保護**：カメラ不使用
2. **低コスト**：2万円以下で実現
3. **リアルタイム性**：即座にアラート
4. **拡張性**：センサー追加可能

### 質問への準備
- **Q: バッテリー持続時間は？**
  - A: 約8時間（連続稼働時）
  
- **Q: 通信範囲は？**
  - A: WiFi圏内、将来的にLTE対応も可能
  
- **Q: 誤検知対策は？**
  - A: 時間帯別しきい値調整、複数センサーでの確認

## 📈 追加デモ（時間があれば）

### データ分析
- 1日の温湿度変化グラフ
- 活動パターン分析
- アラート頻度統計

### 将来展望
- LINE Bot連携
- 機械学習による異常検知
- 複数施設管理

## 🎯 成功指標

- [ ] 転倒検知の実演成功
- [ ] リアルタイムデータ表示
- [ ] アラート通知の確認
- [ ] GPS位置表示
- [ ] 5分以内に完了

---
**準備完了チェック**
- [ ] M5Stack Fire充電済み
- [ ] センサー接続確認
- [ ] WiFi設定完了
- [ ] MQTT接続テスト済み
- [ ] デモスクリプト準備