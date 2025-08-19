#!/bin/bash

# Care Copilot - LINE Messaging API Setup Script
# 実行: bash setup-line-api.sh

echo "🚀 Care Copilot LINE Messaging API セットアップ開始"
echo "================================================"

# 色定義
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 現在のディレクトリチェック
if [ ! -f "backend-line-messaging.js" ]; then
    echo -e "${RED}❌ エラー: 04-implementationディレクトリで実行してください${NC}"
    exit 1
fi

# Step 1: 依存パッケージインストール
echo ""
echo "📦 Step 1: 依存パッケージをインストール中..."
if npm install @line/bot-sdk express mqtt dotenv; then
    echo -e "${GREEN}✅ パッケージインストール完了${NC}"
else
    echo -e "${RED}❌ パッケージインストールに失敗しました${NC}"
    exit 1
fi

# Step 2: .envファイルセットアップ
echo ""
echo "📝 Step 2: 環境変数ファイルを設定中..."

if [ ! -f ".env" ]; then
    if [ -f "../.env.example" ]; then
        cp ../.env.example .env
        echo -e "${GREEN}✅ .envファイルを作成しました${NC}"
    else
        echo -e "${YELLOW}⚠️  .env.exampleが見つかりません。新規作成します${NC}"
        cat > .env << EOF
# LINE Messaging API設定
LINE_CHANNEL_ACCESS_TOKEN=your_channel_access_token_here
LINE_CHANNEL_SECRET=your_channel_secret_here

# MQTT設定
MQTT_BROKER=mqtt://test.mosquitto.org

# サーバー設定
PORT=3000
NODE_ENV=development

# ログ設定
LOG_LEVEL=info
EOF
        echo -e "${GREEN}✅ .envファイルを新規作成しました${NC}"
    fi
else
    echo -e "${YELLOW}⚠️  .envファイルが既に存在します${NC}"
fi

# Step 3: 設定値確認
echo ""
echo "📋 Step 3: 設定値を確認してください"
echo "----------------------------------------"

# .envファイルを読み込み
if [ -f ".env" ]; then
    source .env
fi

# LINE設定チェック
if [ "$LINE_CHANNEL_ACCESS_TOKEN" = "your_channel_access_token_here" ] || [ -z "$LINE_CHANNEL_ACCESS_TOKEN" ]; then
    echo -e "${RED}❌ LINE_CHANNEL_ACCESS_TOKEN が設定されていません${NC}"
    echo ""
    echo "🔗 LINE Developers Consoleにアクセスしてください:"
    echo "   https://developers.line.biz/console/"
    echo ""
    echo "1. Messaging API設定タブを開く"
    echo "2. Channel Access Tokenを発行"
    echo "3. .envファイルに貼り付け"
    echo ""
    read -p "設定が完了したらEnterキーを押してください..."
fi

if [ "$LINE_CHANNEL_SECRET" = "your_channel_secret_here" ] || [ -z "$LINE_CHANNEL_SECRET" ]; then
    echo -e "${RED}❌ LINE_CHANNEL_SECRET が設定されていません${NC}"
    echo ""
    echo "🔗 LINE Developers Console:"
    echo "1. チャネル基本設定タブを開く"
    echo "2. Channel Secretをコピー"
    echo "3. .envファイルに貼り付け"
    echo ""
    read -p "設定が完了したらEnterキーを押してください..."
fi

# Step 4: ngrokインストール確認
echo ""
echo "🌐 Step 4: ngrokをチェック中..."

if ! command -v ngrok &> /dev/null; then
    echo -e "${YELLOW}⚠️  ngrokがインストールされていません${NC}"
    echo ""
    echo "ngrokをインストールしますか？(y/n)"
    read -p "> " install_ngrok
    
    if [ "$install_ngrok" = "y" ]; then
        if command -v brew &> /dev/null; then
            brew install ngrok
            echo -e "${GREEN}✅ ngrokをインストールしました${NC}"
        else
            echo "手動でngrokをインストールしてください:"
            echo "https://ngrok.com/download"
        fi
    fi
else
    echo -e "${GREEN}✅ ngrokがインストール済みです${NC}"
fi

# Step 5: 起動オプション
echo ""
echo "========================================="
echo -e "${GREEN}🎉 セットアップ完了！${NC}"
echo "========================================="
echo ""
echo "次のステップ:"
echo ""
echo "1. ngrokトンネルを開始（別ターミナル）:"
echo -e "   ${YELLOW}ngrok http 3000${NC}"
echo ""
echo "2. バックエンドを起動:"
echo -e "   ${YELLOW}node backend-line-messaging.js${NC}"
echo ""
echo "3. LINE DevelopersでWebhook URLを設定:"
echo "   https://xxxx.ngrok.io/webhook"
echo ""
echo "----------------------------------------"
echo ""
read -p "今すぐバックエンドを起動しますか？(y/n) > " start_backend

if [ "$start_backend" = "y" ]; then
    echo ""
    echo "🚀 バックエンドを起動中..."
    echo "終了するには Ctrl+C を押してください"
    echo ""
    node backend-line-messaging.js
fi

echo ""
echo "セットアップスクリプト終了"