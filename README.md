# 🏠 ケア・コパイロット (Care Copilot)
### M5Stack Fire × M5Unified 高齢者見守りIoTシステム

![Status](https://img.shields.io/badge/Status-Development-yellow)
![Contest](https://img.shields.io/badge/Contest-M5Stack_Global_2025-blue)
![Platform](https://img.shields.io/badge/Platform-M5Stack_Fire-red)
![Library](https://img.shields.io/badge/Library-M5Unified-orange)
![License](https://img.shields.io/badge/License-MIT-green)

[English Version](README_EN.md)

## 📋 プロジェクト概要

高齢者・障害者向けグループホームおよび個別住宅における、尊厳を重視した見守りと自立支援を実現する革新的なIoTシステムです。

### 🎯 解決する課題
- **アラート疲れ**: 無意味な通知を80%削減
- **プライバシー侵害**: カメラを使わない非接触センシング
- **外出制限**: 自立した散歩を支援
- **高額な初期投資**: 2万円以下で導入可能

### 💡 革新的な特徴
- **インテリジェント通知**: AIが緊急度を判定し、本当に必要な時だけ通知
- **外出支援モード**: 業界初の「自立散歩」支援機能
- **QOL向上分析**: データから生活改善提案を自動生成

## 📁 プロジェクト構成

```
care-copilot-m5stack/
├── 01-contest-mvp/                  # コンテスト向けMVP開発
│   ├── README.md                    # 6日間開発計画
│   ├── hardware-list.md             # 購入リスト
│   ├── development-plan.md          # 詳細スケジュール
│   └── demo-scenario.md             # デモシナリオ
│
├── 02-commercialization/            # 商用化戦略
│   ├── business-plan.md             # 事業計画
│   ├── market-analysis.md           # 市場分析
│   └── roadmap.md                   # 商用化ロードマップ
│
├── 03-technical-social-impact/      # 技術と社会的意義
│   ├── technical-spec.md            # 技術仕様書
│   ├── social-impact.md             # 社会的インパクト
│   └── competitors.md               # 競合分析
│
├── 04-implementation/                # 実装コード
│   ├── m5stack-fire-unified-all/    # 統合版（全センサー対応）
│   │   ├── m5stack-fire-unified-all.ino  # メインコード
│   │   ├── config.h                 # 設定ファイル
│   │   └── config_local.h.example   # ローカル設定テンプレート
│   ├── backend-line-messaging.js    # LINE連携バックエンド
│   ├── LINE_MESSAGING_API_GUIDE.md  # LINE API設定ガイド
│   ├── MQTT_TROUBLESHOOTING.md      # MQTTトラブルシューティング
│   ├── QUICK_START_GUIDE.md         # クイックスタートガイド
│   └── setup-line-api.sh            # LINE API設定スクリプト
│
├── 05-investor-materials/           # 投資家向け資料
│   └── pitch-deck.md                # ピッチデッキ
│
├── CLAUDE.md                        # Claude Code向け指示書
├── PROJECT_STATUS.md                # プロジェクト進捗状況
├── CONTRIBUTING.md                  # コントリビューションガイド
├── package.json                     # Node.js依存関係
└── README.md                        # このファイル
```

## 🚀 実装パターン（2種類）

| パターン | センサー構成 | 主な用途 | コスト | 実装状況 |
|---------|-------------|---------|--------|----------|
| **Unified All版** | IMU+ToF4M+ENV.4+GPS | フル機能・コンテスト対応 | ¥20,000 | ✅ 実装完了 |

### 🔄 実装状況（2025/08/20 03:00時点）

#### ✅ 完了（60%）
- **MQTT通信**: broker.hivemq.com経由でリアルタイム送信
- **LINE通知**: Messaging API統合完了、アラート送信成功
- **全センサー**: IMU、ToF、ENV.4、GPS全て正常動作

#### ❌ 未実装（40%）
- **PostgreSQL**: データベース未構築
- **Webダッシュボード**: 未実装
- **M5StickC連携**: 未実装
- **睡眠分析**: 未実装
- **日次レポート**: 未実装

## 🛠️ 開発環境

### Arduino IDE設定
1. **ボードマネージャーURL追加**:
   ```
   https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json
   ```
2. **M5Stackボード**をインストール
3. **M5Unified**ライブラリをインストール

## 🚀 クイックスタート

### ハードウェア要件
- **M5Stack Fire** (IMU搭載、LED Bar付き)
- **必須センサー**:
  - ToF4M (距離測定)
  - ENV.4 (SHT4X温湿度 + BMP280気圧)
  - GPS Module (位置追跡用)
- **接続**:
  - Port A: I2Cハブ経由でToF4M + ENV.4
  - Port C: GPS Module

詳細は[ハードウェアリスト](01-contest-mvp/hardware-list.md)を参照

### 前提条件
- Arduino IDE 2.0+ 
- Node.js 18+ と npm
- Python 3.9+ (AI分析部分)
- PostgreSQL 14+
- Redis 6+

### セットアップ手順

#### 1. リポジトリのクローン
```bash
git clone https://github.com/terisuke/care-copilot-m5stack.git
cd care-copilot-m5stack
```

#### 2. バックエンドサーバーのセットアップ
```bash
# 依存関係のインストール
cd 04-implementation
npm install

# 環境変数の設定
cp .env.example .env
# .envファイルを編集してLINE APIの認証情報を設定

# サーバー起動
node backend-line-messaging.js

# Webhook用（ローカルテスト）
ngrok http 3000
# LINE Developers ConsoleにWebhook URLを設定
```

#### 3. M5Stackファームウェアの書き込み
1. Arduino IDEを開く
2. ボード設定: M5Stack → M5Stack-Fire
3. `04-implementation/m5stack-fire-unified-all/m5stack-fire-unified-all.ino`を開く
4. `config_local.h.example`を`config_local.h`にコピー
5. WiFi情報を`config_local.h`に設定
6. アップロード (速度: 921600)

#### 4. Python環境のセットアップ（オプション）
```bash
# 仮想環境の作成
python -m venv venv
source venv/bin/activate  # Mac/Linux
# または
venv\Scripts\activate  # Windows

# 依存関係のインストール
pip install pandas numpy scikit-learn matplotlib requests
```

## 🛠 技術スタック

### ハードウェア
- **M5Stack Fire**: メインコントローラー (ESP32-D0WDQ6-V3, IMU搭載)
- **センサー群**: ToF4M、ENV.4 (SHT4X + BMP280)、GPS
- **ライブラリ**: M5Unified (統一API)

### ソフトウェア
- **ファームウェア**: Arduino/C++ (ESP32)
- **バックエンド**: Node.js, Express, Socket.io
- **データベース**: PostgreSQL (時系列データ), Redis (キャッシュ)
- **通信**: MQTT, WebSocket, REST API
- **通知**: LINE Messaging API
- **AI/ML**: Python (scikit-learn, pandas)

## 📊 システムアーキテクチャ

```
[M5Stack デバイス] --MQTT--> [MQTTブローカー]
                                    |
                                    v
                            [バックエンドサーバー]
                                    |
                        +-----------+-----------+
                        |           |           |
                    [PostgreSQL] [Redis]  [LINE API]
                        |
                    [Webダッシュボード]
```

## 📅 緊急実装スケジュール

### 8月20日（残り21時間）
| 時間 | タスク | 目標 |
|------|--------|------|
| 03:00-09:00 | PostgreSQL構築 | DB動作確認 |
| 09:00-12:00 | データ永続化 | センサーデータ保存 |
| 12:00-18:00 | ダッシュボード | 基本画面完成 |
| 18:00-24:00 | M5StickC連携 | 外出モード実装 |

### 8月21日（24時間）
| 時間 | タスク | 目標 |
|------|--------|------|
| 00:00-06:00 | 睡眠分析 | パターン解析 |
| 06:00-12:00 | レポート機能 | 日次レポート生成 |
| 12:00-18:00 | 統合テスト | 全機能動作確認 |
| 18:00-24:00 | デモ準備 | シナリオ完成 |

## 🏆 目標

### 短期（コンテスト）
- M5Stack Global Innovation Contest 2025 入賞
- 社会的インパクトの実証
- 技術的実現可能性の証明

### 長期（事業化）
- 中小規模グループホーム市場でシェア10%
- 月額3,000円/人のSaaSモデル確立
- 介護記録ソフトとのAPI連携実現

## 🤝 コントリビューション

プロジェクトへの貢献を歓迎します！

1. このリポジトリをフォーク
2. フィーチャーブランチを作成 (`git checkout -b feature/AmazingFeature`)
3. 変更をコミット (`git commit -m 'Add some AmazingFeature'`)
4. ブランチにプッシュ (`git push origin feature/AmazingFeature`)
5. プルリクエストを作成

### GitHub Actions設定

#### Claude Code Review (オプション)
プルリクエストの自動レビューを有効にするには：
1. [Claude Code](https://claude.ai/code)でOAuthトークンを取得
2. リポジトリの Settings → Secrets and variables → Actions に移動
3. `CLAUDE_CODE_OAUTH_TOKEN`として保存

注: 初回PRでは認証エラーが発生することがありますが、これは正常な動作です。

## 📄 ライセンス

このプロジェクトはMITライセンスの下で公開されています。詳細は[LICENSE](LICENSE)ファイルを参照してください。

## 📞 コンタクト

- プロジェクトリード: Terada Kousuke
- 開始日: 2025年8月16日
- コンテスト締切: 2025年8月22日

## 🙏 謝辞

- M5Stack Global Innovation Contest 2025
- すべてのオープンソースコントリビューター
- テストに協力していただいた介護施設の皆様

---

*"見守る"のではなく"共に生きる" - ケア・コパイロットの約束*