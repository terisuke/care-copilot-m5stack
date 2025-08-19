# MQTT接続トラブルシューティング

## 症状
M5Stack画面で「M:NG」と表示され、MQTT接続できない

## 原因の可能性

### 1. ネットワーク問題
- **ファイアウォール**: ポート1883がブロックされている
- **プロキシ**: 企業ネットワークなど
- **DNS**: test.mosquitto.orgが解決できない

### 2. WiFi設定問題
- **2.4GHz帯**: M5Stackは5GHz非対応
- **SSID/パスワード**: 誤入力
- **IPアドレス**: DHCPが機能していない

### 3. MQTTブローカー問題
- **サーバーダウン**: test.mosquitto.orgが停止
- **接続制限**: 同時接続数超過

## 診断方法

### シリアルモニター確認
```
Arduino IDE → ツール → シリアルモニタ（115200bps）
```

期待される出力:
```
Attempting MQTT connection... Connected!
Client ID: M5Fire-xxxx
```

エラーの場合:
```
Attempting MQTT connection... Failed, rc=-2
MQTT_CONNECT_FAILED
```

### エラーコード一覧

| コード | 意味 | 対処法 |
|-------|------|--------|
| -4 | CONNECTION_TIMEOUT | ネットワーク確認 |
| -2 | CONNECT_FAILED | ブローカーアドレス確認 |
| -1 | DISCONNECTED | WiFi接続確認 |
| 1 | BAD_PROTOCOL | MQTTバージョン確認 |
| 2 | BAD_CLIENT_ID | クライアントID変更 |
| 3 | UNAVAILABLE | ブローカー停止中 |
| 4 | BAD_CREDENTIALS | 認証情報確認 |
| 5 | UNAUTHORIZED | アクセス権限なし |

## 解決方法

### 方法1: 別のブローカーを試す

config.hを編集:
```cpp
// 代替ブローカー
#define MQTT_SERVER "broker.hivemq.com"  // HiveMQ
// または
#define MQTT_SERVER "broker.emqx.io"     // EMQX
// または  
#define MQTT_SERVER "mqtt.eclipse.org"   // Eclipse
```

### 方法2: ローカルブローカー使用

Mosquittoをインストール:
```bash
# Mac
brew install mosquitto
mosquitto -v

# config.h
#define MQTT_SERVER "192.168.1.100"  // あなたのPCのIP
```

### 方法3: ネットワーク診断

```cpp
// テストコード追加（setup()内）
IPAddress result;
if (WiFi.hostByName("test.mosquitto.org", result)) {
    Serial.print("MQTT broker IP: ");
    Serial.println(result);
} else {
    Serial.println("DNS resolution failed!");
}
```

### 方法4: ポート変更

WebSocket経由（ポート8080）:
```cpp
#define MQTT_PORT 8080  // または8883（SSL）
```

## 確認コマンド

### PC側でMQTTブローカー確認
```bash
# 接続テスト
telnet test.mosquitto.org 1883

# メッセージ購読
mosquitto_sub -h test.mosquitto.org -t "care/#" -v

# テストメッセージ送信
mosquitto_pub -h test.mosquitto.org -t "care/test" -m "Hello"
```

### WiFiルーター確認
- ポート1883が開いているか
- MACアドレスフィルタリング無効か
- DHCP有効か

## 代替案: MQTT不要版

MQTTが使えない環境では、HTTP POSTで直接バックエンドに送信:

```cpp
// WiFiClient使用
HTTPClient http;
http.begin("http://your-server:3000/data");
http.addHeader("Content-Type", "application/json");
http.POST(jsonString);
http.end();
```

## それでも解決しない場合

1. **ログ収集**
   - シリアルモニター全ログ
   - WiFi RSSI値
   - エラーコード

2. **環境情報**
   - ルーター機種
   - ISP
   - ファイアウォール設定

3. **代替通信**
   - WebSocket
   - HTTP Long Polling
   - Server-Sent Events

## 成功確認

画面表示:
```
M:OK  // MQTT接続成功
```

シリアル出力:
```
MQTT Connected!
Client ID: M5Fire-xxxx
Subscribed to care/command/+
```

## お問い合わせ

解決しない場合はGitHub Issueで報告してください。
必要情報:
- エラーコード
- シリアルログ
- ネットワーク環境