# 🎬 デモシナリオ詳細

## 📱 デモ動画コンセプト

**タイトル**: "Care Copilot - Where Technology Meets Dignity"
**時間**: 3分（最大）
**言語**: 英語（字幕付き）
**トーン**: 温かみがあり、希望に満ちた

## 🎭 登場人物設定

### 田中さん（82歳）
- 軽度認知症
- 日中は活発、夜間徘徊リスクあり
- 散歩が日課

### 佐藤さん（75歳）
- 車椅子使用
- 転倒リスク高
- 自立意識が高い

### 介護スタッフ 山田さん
- 夜勤担当
- アラート疲れに悩んでいた

## 📹 シーン別撮影計画

### オープニング（0:00-0:15）

**ナレーション**:
"In Japan, 3.6 million elderly need care support. But current monitoring systems create more problems than they solve."

**映像**:
- 施設の外観（朝）
- スタッフがスマホを見て疲れた表情
- アラート音の連続（音声のみ）

### Scene 1: アラート疲れの問題（0:15-0:45）

**実演内容**:
```
[従来システムのシミュレーション]
22:00 - "田中さんがベッドから離れました"
22:05 - "田中さんがトイレに入りました"
22:10 - "佐藤さんが寝返りしました"
22:15 - "田中さんが部屋に戻りました"
（アラートが鳴り続ける）
```

**山田さんのセリフ**:
"Every night, dozens of meaningless alerts. We can't distinguish real emergencies anymore."

**映像のポイント**:
- スマホ画面に通知が殺到
- スタッフが通知をオフにする様子
- 疲れ切った表情のクローズアップ

### Scene 2: Care Copilotの導入（0:45-1:15）

**ナレーション**:
"Care Copilot changes everything with AI-powered smart filtering."

**実演内容**:
```
[M5Stack設置シーン]
- M5Stack Basicを管理室に設置
- M5StickC Plus2を田中さんの部屋に設置
- センサー類の配置（カメラなし強調）
```

**テクニカル説明**:
```
画面表示:
"No Cameras = 100% Privacy"
"AI learns normal patterns"
"4-level urgency system"
```

### Scene 3: 通常の夜（1:15-1:45）

**実演内容**:
```
22:00 - 田中さん通常就寝
→ M5Stack: 緑色LED点灯（記録のみ）
→ スタッフ端末: 通知なし

23:30 - 田中さんトイレへ
→ M5Stack: パターン認識「通常行動」
→ スタッフ端末: 通知なし

山田さん: "Finally, a peaceful night!"
```

**データ表示**:
```
ダッシュボード画面:
- アラート削減率: 85%
- 誤検知: 0件
- スタッフ満足度: ⬆
```

### Scene 4: 緊急事態の適切な検知（1:45-2:15）

**実演内容**:
```
02:30 - 佐藤さん転倒シミュレーション
→ 加速度センサー検知
→ M5Stack: 赤色LED点滅
→ 即座にLINE通知
→ 音声アラート発動

山田さん: すぐに駆けつける
"Real emergency, real alert. Perfect!"
```

**技術デモ**:
```
画面分割表示:
- センサーデータ（グラフ）
- AI判定プロセス
- 通知タイミング
```

### Scene 5: 革新的な外出支援（2:15-2:45）

**実演内容**:
```
09:00 - 田中さん散歩準備
→ M5StickC Plus2を首から装着
→ "散歩モード"起動

09:30 - 公園で休憩
→ GPS追跡（地図表示）
→ 歩数: 2,847歩
→ 「いつもの休憩スポット」認識

10:00 - 無事帰宅
→ QOLレポート生成
→ "Great walk! 3,000 steps achieved!"
```

**差別化ポイント強調**:
"Not surveillance, but support for independent living"

### クロージング：インパクトと将来性（2:45-3:00）

**統計表示**:
```
対象市場:
- 日本: 20,000施設
- 世界: 500,000施設

コスト削減:
- 初期費用: 1/10
- 運用費: 1/5

QOL向上:
- 外出機会: +40%
- スタッフ満足度: +60%
```

**最後のメッセージ**:
"Care Copilot - Transforming eldercare with dignity and technology"

**M5Stackロゴとプロジェクト情報表示**

## 🎯 撮影のコツ

### 照明
- 自然光を活用（朝の撮影推奨）
- M5Stack画面は少し暗めの環境で撮影
- センサーLEDが見えるように

### カメラアングル
- M5Stack: 斜め45度から（画面とLEDが見える）
- 人物: 表情がわかる距離
- 全体: 施設の雰囲気が伝わる広角

### 音声
- ナレーション: 後から録音
- 環境音: 適度に残す（リアリティ）
- アラート音: はっきりと録音

## 📝 Hackster.io 記事構成

### タイトル
"Care Copilot: AI-Powered Dignity-First Care System Using M5Stack"

### サマリー（150文字）
"Revolutionary care monitoring system that reduces false alerts by 85% while ensuring privacy and supporting independent living. Built with M5Stack for elderly and disabled care facilities."

### セクション構成

#### 1. The Problem We Solve
- Alert fatigue statistics
- Privacy invasion concerns
- Current market limitations

#### 2. Our Solution
- Smart filtering system
- Privacy-first design
- Outdoor support features

#### 3. Technical Implementation
- Hardware architecture diagram
- Software flow chart
- AI decision tree

#### 4. Components and Supplies
- Detailed parts list with links
- Total cost calculation
- Alternative options

#### 5. Code
```cpp
// Key algorithm showcase
AlertLevel smartFilter(SensorData data) {
    // Innovative filtering logic
    ...
}
```

#### 6. Schematics
- Connection diagram
- System architecture
- Data flow visualization

#### 7. Story
- Development journey
- Challenges overcome
- User testimonials (simulated)

#### 8. Impact
- Market size analysis
- Cost reduction potential
- Social significance

## 🎪 デモブース設営（もし現地デモの場合）

### 必要機材
```
展示台 配置:
┌─────────────────────────┐
│   モニター（ダッシュボード）    │
├─────────┬───────────────┤
│ M5Stack │  センサー展示   │
│  Basic  │  (PIR/超音波)  │
├─────────┼───────────────┤
│   iPad  │ M5StickC      │
│(LINE表示)│  Plus2展示     │
└─────────┴───────────────┘
```

### 説明フロー（1人3分）
1. 課題の共有（30秒）
2. システム概要（30秒）
3. 実機デモ（1分30秒）
4. 質疑応答（30秒）

### よくある質問への回答準備

**Q: カメラなしで本当に安全？**
A: "PIRとdistance sensorsの組み合わせで、転倒検知率95%を実現。プライバシーと安全の両立が可能です。"

**Q: 既存システムとの連携は？**
A: "REST APIで主要な介護記録システムと連携可能。段階的な導入も対応します。"

**Q: バッテリー持続時間は？**
A: "M5StickC Plus2で連続8時間。充電ステーション設置で24時間運用可能です。"

## 📊 効果測定メトリクス

### デモ成功指標
- [ ] 3つのシナリオを2分30秒で完遂
- [ ] アラート削減を視覚的に実証
- [ ] 外出支援機能の革新性が伝わる
- [ ] 審査員から質問が出る

### 動画成功指標
- [ ] 最初の10秒で関心を引く
- [ ] 技術と人間性のバランス
- [ ] 明確なbefore/after
- [ ] Call to actionが明確

## 🚨 緊急時対応

### Plan B（機材トラブル）
- 録画済み動画を使用
- スマホでバックアップデモ
- プレゼン資料で説明

### Plan C（通信トラブル）
- ローカルモードで動作
- オフラインデータ使用
- 後から結果を表示

## ✨ 演出の工夫

### 感情に訴える要素
- 介護者の笑顔
- 利用者の自立した様子
- 数値だけでなくストーリー

### 技術力を示す要素
- リアルタイムデータ表示
- AI判定の可視化
- システム応答速度

### 革新性を示す要素
- "業界初"の機能強調
- 特許出願可能な技術
- 将来の拡張性

---

## 🎬 Action!

**撮影チェックリスト**:
- [ ] 機材充電100%
- [ ] 予備バッテリー準備
- [ ] 撮影場所の下見
- [ ] リハーサル2回実施
- [ ] 複数アングルで撮影
- [ ] 音声別撮り準備

**Remember**: 
"It's not about perfect technology, it's about perfect empathy."

---

*Break a leg! 🎭*