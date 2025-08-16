# 🏠 ケア・コパイロット (Care Copilot)
### 施設内外トータルケアM5Stack IoTシステム

![Status](https://img.shields.io/badge/Status-Development-yellow)
![Contest](https://img.shields.io/badge/Contest-M5Stack_Global_2025-blue)
![License](https://img.shields.io/badge/License-MIT-green)

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
├── 01-contest-mvp/          # コンテスト向けMVP開発
│   ├── README.md            # 6日間開発計画
│   ├── hardware-list.md     # 購入リスト
│   ├── development-plan.md  # 詳細スケジュール
│   └── demo-scenario.md     # デモシナリオ
│
├── 02-commercialization/    # 商用化戦略
│   ├── business-plan.md     # 事業計画
│   ├── market-analysis.md   # 市場分析
│   └── roadmap.md          # 商用化ロードマップ
│
└── 03-technical-social-impact/  # 技術と社会的意義
    ├── technical-spec.md    # 技術仕様書
    ├── social-impact.md     # 社会的インパクト
    └── competitors.md       # 競合分析
```

## 🚀 クイックスタート

### ハードウェア要件（総額: 約18,000円）
- M5Stack Core2
- M5StickC Plus2  
- GPS Module (M5Stack GPS/BDS Unit)
- PIRセンサー (人感検知)
- 超音波センサー (距離測定)
- BME280 (環境センサー)

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
npm install

# 環境変数の設定
cp .env.example .env
# .envファイルを編集して必要な値を設定

# サーバー起動
node 04-implementation/backend-server.js
```

#### 3. M5Stackファームウェアの書き込み
1. Arduino IDEを開く
2. ボード設定: ESP32 Arduino → M5Stack-Core-ESP32
3. `04-implementation/m5stack-firmware.ino`を開く
4. WiFi情報とMQTTブローカー情報を設定
5. アップロード (速度: 921600)

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
- **M5Stack Core2**: メインコントローラー (ESP32-D0WDQ6-V3)
- **M5StickC Plus2**: ウェアラブル端末
- **センサー群**: PIR、超音波、BME280、GPS

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

## 📅 タイムライン

| フェーズ | 期間 | 目標 |
|---------|------|------|
| **MVP開発** | 6日間 | M5Stack Contest応募 |
| **実証実験** | 3ヶ月 | 5施設でテスト |
| **商用化** | 6ヶ月 | サービス開始 |

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