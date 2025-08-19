# 🚀 Care Copilot クイックスタートガイド

**最終更新**: 2025年8月20日  
**所要時間**: 30分  
**対象**: LINE Messaging API統合版

## 📋 事前準備チェックリスト

- [ ] M5Stack Fire本体
- [ ] センサー（ToF4M、ENV.4、GPS）※オプション
- [ ] WiFi環境（2.4GHz帯）
- [ ] LINE Developersアカウント
- [ ] Node.js環境（v18以上）
- [ ] スマートフォン（LINE アプリ）

## 🎯 今すぐ動かす手順

### 1️⃣ M5Stack設定（5分）

```bash
# 設定ファイル作成
cd 04-implementation/m5stack-fire-unified-all/
cp config_local.h.example config_local.h

# config_local.hを編集
# WiFi情報を入力:
#define WIFI_SSID "あなたのWiFi名"
#define WIFI_PASSWORD "あなたのパスワード"
```

**Arduino IDEでアップロード:**
- ボード: M5Stack-Fire
- ライブラリ: M5Unified, VL53L1X, M5Unit-ENV, TinyGPSPlus, PubSubClient
- アップロード速度: 921600

### 2️⃣ LINE Messaging API設定（15分）

#### A. LINE Developers Console設定

1. **チャネル作成**
   ```
   https://developers.line.biz/console/
   → 新規プロバイダー作成
   → Messaging APIチャネル作成
   ```

2. **認証情報取得**
   - Channel Access Token（長期）を発行
   - Channel Secretをコピー

3. **Webhook設定**
   - Webhook URL: （後で設定）
   - Webhookの利用: オン
   - 応答メッセージ: オフ

#### B. バックエンドサーバー設定

```bash
# 依存関係インストール
cd 04-implementation/
npm install

# 環境変数設定
cp ../.env.example .env

# .envファイル編集
LINE_CHANNEL_ACCESS_TOKEN=<取得したトークン>
LINE_CHANNEL_SECRET=<取得したシークレット>

# サーバー起動
node backend-line-messaging.js
```

#### C. ngrokでWebhook公開

```bash
# 別ターミナルで実行
ngrok http 3000

# 表示されたHTTPS URLをコピー
# 例: https://abc123.ngrok-free.app

# LINE Developers ConsoleでWebhook URL設定
# https://abc123.ngrok-free.app/webhook
```

### 3️⃣ LINE友だち追加とテスト（5分）

1. **QRコードで友だち追加**
   - LINE Developers Console → Messaging API設定
   - QRコードをスキャン

2. **動作確認**
   ```
   LINEで送信: ヘルプ
   → コマンド一覧が返信される
   
   M5Stackボタン操作:
   - A: テストアラート送信
   - B: アラートクリア
   - C: 詳細情報表示
   ```

## ✅ 動作確認チェックリスト

### M5Stack側
- [ ] WiFi接続: 画面に「W:OK」表示
- [ ] MQTT接続: 画面に「M:OK」表示
- [ ] センサー値: 正常な数値表示

### LINE側
- [ ] 友だち追加: ウェルカムメッセージ受信
- [ ] コマンド応答: 「ヘルプ」に反応
- [ ] アラート受信: ボタンAでLINE通知

## 🚨 トラブルシューティング

### MQTT接続「M:NG」の場合
```bash
# MQTTブローカー変更
# config.hまたはconfig_local.h
#define MQTT_SERVER "broker.hivemq.com"  # 推奨
```

### LINE通知が来ない場合
1. サーバーログ確認
2. Channel Access Token再発行
3. ngrok URL更新

### GPS「G:..」表示の場合
- 屋外または窓際に移動
- 30秒～2分待機

## 📊 正常動作時の表示例

**M5Stack画面:**
```
Care Copilot
W:OK M:OK T:OK E:OK G:OK

IMU: Normal

Dist: 1490mm
Zone: NORMAL

T:24.8C H:60.4%

GPS: Fixed
Sat:16

Bat:75% MQTT:broker.hivemq.com
[A]Alert [B]Clear [C]Info
```

**LINE通知例:**
```
Test alert - Button A (Level 2)
レベル: 2
時刻: 2025/08/20 15:00:00

📍 現在位置
緯度: 33.581303, 経度: 130.342376
```

## 🎯 次のステップ

1. **センサー調整**
   - 転倒検知感度: FALL_THRESHOLD
   - 離床距離: DISTANCE_FAR
   - 温度警告: TEMP_HIGH/LOW

2. **追加機能実装**
   - 定期レポート設定
   - 複数デバイス管理
   - データ履歴記録

3. **デモ準備**
   - 転倒シミュレーション
   - 離床検知テスト
   - GPS外出追跡

## 📝 重要な注意事項

- WiFiは2.4GHz帯のみ対応（5GHz非対応）
- GPSは屋外でのみ動作
- LINE通知にはインターネット接続必須
- ngrok無料版は8時間でセッション切れ

---

**サポート**: GitHub Issueで報告
**コンテスト期限**: 2025年8月22日