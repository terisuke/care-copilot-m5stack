# 📱 LINE Messaging API 実装ガイド

**作成日**: 2025年8月19日  
**所要時間**: 30分  
**難易度**: ⭐⭐☆☆☆

## 🎯 概要

LINE Notifyの2025年3月終了に伴い、LINE Messaging APIへ移行します。  
このガイドに従えば30分で実装完了できます。

## 📋 必要なもの

- LINE Developersアカウント
- Node.js環境
- ngrok（ローカルテスト用）

## 🚀 実装手順

### Step 1: LINE Developersの設定（10分）

#### 1.1 チャネル作成

1. [LINE Developers Console](https://developers.line.biz/console/)にログイン
2. 「新規プロバイダー作成」→ プロバイダー名: `Care Copilot`
3. 「Messaging APIチャネル作成」
   - チャネル名: `Care Copilot Bot`
   - チャネル説明: `高齢者見守りシステム`
   - 大業種: `その他`
   - 小業種: `その他`

#### 1.2 必要な情報を取得

Messaging API設定タブから以下を取得：

- **Channel ID**: `(自動生成される)`
- **Channel Secret**: `チャネル基本設定`タブで確認
- **Channel Access Token**: `Messaging API設定`タブで発行

```bash
# .envファイルに記載
LINE_CHANNEL_ACCESS_TOKEN=xxx...
LINE_CHANNEL_SECRET=xxx...
```

### Step 2: バックエンドセットアップ（10分）

#### 2.1 依存パッケージインストール

```bash
cd 04-implementation
npm install @line/bot-sdk express mqtt dotenv
```

#### 2.2 環境変数設定

```bash
# .envファイル作成
cp ../.env.example .env

# 以下を編集
LINE_CHANNEL_ACCESS_TOKEN=<取得したトークン>
LINE_CHANNEL_SECRET=<取得したシークレット>
MQTT_BROKER=mqtt://test.mosquitto.org
PORT=3000
```

#### 2.3 ngrokでトンネル作成

```bash
# ngrokインストール（初回のみ）
brew install ngrok

# トンネル開始
ngrok http 3000

# 表示されるHTTPS URLをコピー
# 例: https://abc123.ngrok.io
```

#### 2.4 Webhook設定

1. LINE Developers Consoleに戻る
2. Messaging API設定タブ
3. Webhook設定:
   - Webhook URL: `https://abc123.ngrok.io/webhook`
   - Webhookの利用: ON
   - 応答メッセージ: OFF

### Step 3: 起動と動作確認（10分）

#### 3.1 バックエンド起動

```bash
node backend-line-messaging.js
```

期待される出力:
```
🚀 Care Copilot LINE Messaging API Starting...
📡 MQTT Broker: mqtt://test.mosquitto.org
🌐 Server Port: 3000
✅ Server running on port 3000
📱 Webhook URL: https://your-domain.com/webhook
   (Use ngrok for local testing: ngrok http 3000)
✅ MQTT Connected
✅ Subscribed to care/sensor/data
✅ Subscribed to care/alert
✅ Subscribed to care/status
✅ Subscribed to care/location
```

#### 3.2 LINE友だち追加

1. LINE Developers Console → Messaging API設定
2. QRコードを表示
3. スマートフォンのLINEアプリでスキャン
4. 友だち追加

#### 3.3 動作テスト

LINEアプリでメッセージ送信:

| 送信メッセージ | 期待される応答 |
|--------------|--------------|
| ヘルプ | コマンド一覧表示 |
| ステータス | センサー情報表示 |
| 位置 | GPS位置情報表示 |
| テスト | テストアラート送信 |

### Step 4: M5Stackとの連携確認

#### 4.1 M5Stack起動確認

M5Stack画面で以下を確認:
- WiFi: OK
- MQTT: OK

#### 4.2 アラートテスト

M5StackのボタンAを押してテストアラート送信

期待される動作:
1. M5Stack → MQTT送信
2. バックエンド受信
3. LINE通知受信

## 📊 実装確認チェックリスト

- [ ] Channel Access Token取得
- [ ] Channel Secret取得  
- [ ] .envファイル設定
- [ ] ngrok起動
- [ ] Webhook URL設定
- [ ] バックエンド起動
- [ ] MQTT接続確認
- [ ] LINE友だち追加
- [ ] コマンド応答確認
- [ ] M5Stack連携確認

## 🔧 トラブルシューティング

### Webhook検証エラー

```bash
# エラー: Webhook URL validation failed
# 解決法:
1. ngrok URLが正しいか確認
2. /webhookパスが含まれているか確認
3. バックエンドが起動しているか確認
```

### MQTT接続エラー

```bash
# エラー: MQTT connection failed
# 解決法:
1. インターネット接続確認
2. ファイアウォール設定確認
3. MQTTブローカーアドレス確認
```

### LINE通知が来ない

```bash
# 確認事項:
1. 友だち追加済みか確認
2. Channel Access Token正しいか確認
3. console.logでユーザーID確認
```

## 📱 デモシナリオ

### シナリオ1: 転倒検知

1. M5Stackを急に振る
2. LINE通知受信（位置情報付き）
3. クイックリプライで「状態確認」

### シナリオ2: 離床検知

1. ToFセンサーから離れる（2m以上）
2. LINE通知「離床を検知しました」
3. 「位置」コマンドで位置確認

### シナリオ3: 環境異常

1. 温度センサーに熱風を当てる
2. LINE通知「高温警告」
3. 「ステータス」で詳細確認

## 🎊 実装完了！

これでLINE Messaging API統合が完了です。  
M5Stack Contest 2025のデモ準備は万全です！

## 📚 参考資料

- [LINE Messaging API公式ドキュメント](https://developers.line.biz/ja/docs/messaging-api/)
- [LINE Bot SDK for Node.js](https://line.github.io/line-bot-sdk-nodejs/)
- [ngrok公式サイト](https://ngrok.com/)