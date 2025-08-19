/**
 * Care Copilot - LINE Messaging API Backend
 * M5Stack Contest 2025対応版
 */

const express = require('express');
const line = require('@line/bot-sdk');
const mqtt = require('mqtt');
require('dotenv').config();

// LINE設定
const config = {
  channelAccessToken: process.env.LINE_CHANNEL_ACCESS_TOKEN,
  channelSecret: process.env.LINE_CHANNEL_SECRET,
};

// Expressアプリ初期化
const app = express();
const PORT = process.env.PORT || 3000;

// ボディパーサー設定
// webhookエンドポイントはLINE SDKが処理するため除外
app.use((req, res, next) => {
  if (req.path !== '/webhook') {
    express.json()(req, res, next);
  } else {
    next();
  }
});

// LINEクライアント初期化
const client = new line.Client(config);

// MQTT設定
const MQTT_BROKER = process.env.MQTT_BROKER || 'mqtt://test.mosquitto.org';
const mqttClient = mqtt.connect(MQTT_BROKER);

// グローバル状態管理
const systemStatus = {
  lastSensorData: null,
  alerts: [],
  devices: new Map(),
  lineUsers: new Set()
};

// MQTTトピック
const TOPICS = {
  SENSOR_DATA: 'care/sensor/data',
  ALERT: 'care/alert',
  STATUS: 'care/status',
  LOCATION: 'care/location'
};

console.log('🚀 Care Copilot LINE Messaging API Starting...');
console.log(`📡 MQTT Broker: ${MQTT_BROKER}`);
console.log(`🌐 Server Port: ${PORT}`);

// ============================================
// MQTT接続とメッセージ処理
// ============================================

mqttClient.on('connect', () => {
  console.log('✅ MQTT Connected');
  
  // トピック購読
  Object.values(TOPICS).forEach(topic => {
    mqttClient.subscribe(topic, (err) => {
      if (!err) {
        console.log(`✅ Subscribed to ${topic}`);
      }
    });
  });
});

mqttClient.on('message', async (topic, message) => {
  try {
    const data = JSON.parse(message.toString());
    console.log(`📨 MQTT [${topic}]:`, data);
    
    switch(topic) {
      case TOPICS.SENSOR_DATA:
        await handleSensorData(data);
        break;
      case TOPICS.ALERT:
        await handleAlert(data);
        break;
      case TOPICS.STATUS:
        await handleDeviceStatus(data);
        break;
      case TOPICS.LOCATION:
        await handleLocation(data);
        break;
    }
  } catch (error) {
    console.error('MQTT message processing error:', error);
  }
});

// ============================================
// センサーデータ処理
// ============================================

async function handleSensorData(data) {
  systemStatus.lastSensorData = data;
  const alerts = [];
  
  // 転倒検知
  if (data.imu && data.imu.fall_detected) {
    alerts.push({
      level: 3,
      type: 'fall',
      message: '🚨 転倒を検知しました！',
      data: data
    });
  }
  
  // 離床検知
  if (data.tof && data.tof.zone === 'FAR') {
    alerts.push({
      level: 2,
      type: 'bed_exit',
      message: '🛏️ 離床を検知しました',
      distance: data.tof.distance
    });
  }
  
  // 環境異常
  if (data.environment) {
    const { temperature, humidity } = data.environment;
    
    if (temperature > 35) {
      alerts.push({
        level: 2,
        type: 'high_temp',
        message: `🌡️ 高温警告: ${temperature.toFixed(1)}°C`
      });
    } else if (temperature < 15 && temperature > 0) {
      alerts.push({
        level: 1,
        type: 'low_temp',
        message: `❄️ 低温注意: ${temperature.toFixed(1)}°C`
      });
    }
    
    if (humidity > 80) {
      alerts.push({
        level: 1,
        type: 'high_humidity',
        message: `💧 高湿度: ${humidity.toFixed(1)}%`
      });
    }
  }
  
  // 重要なアラートをLINE送信
  for (const alert of alerts) {
    if (alert.level >= 2) {
      await broadcastAlert(alert, data);
    }
  }
}

async function handleAlert(data) {
  const { device_id, level, message, latitude, longitude } = data;
  
  console.log(`🚨 Alert received - Level ${level}: ${message}`);
  console.log(`   Device: ${device_id}`);
  if (latitude && longitude) {
    console.log(`   Location: ${latitude}, ${longitude}`);
  }
  
  // テスト用：レベル1以上で送信（本番ではレベル2以上に戻す）
  if (level >= 1) {
    const alert = {
      level,
      message,
      device_id,
      timestamp: new Date().toISOString()
    };
    
    if (latitude && longitude) {
      alert.location = { latitude, longitude };
    }
    
    console.log('📤 Sending alert to LINE users...');
    await broadcastAlert(alert);
  } else {
    console.log(`   Alert level ${level} is below threshold, not sending to LINE`);
  }
}

async function handleDeviceStatus(data) {
  const { device_id, status } = data;
  systemStatus.devices.set(device_id, {
    ...data,
    lastUpdate: new Date().toISOString()
  });
}

async function handleLocation(data) {
  const { device_id, latitude, longitude, satellites } = data;
  
  if (systemStatus.devices.has(device_id)) {
    systemStatus.devices.get(device_id).location = {
      latitude,
      longitude,
      satellites,
      lastUpdate: new Date().toISOString()
    };
  }
}

// ============================================
// LINE送信処理
// ============================================

async function broadcastAlert(alert, sensorData = null) {
  console.log(`📢 Broadcasting alert to ${systemStatus.lineUsers.size} users`);
  
  if (systemStatus.lineUsers.size === 0) {
    console.log('⚠️ No LINE users registered! Please add the bot as a friend first.');
    console.log('   1. Open LINE app');
    console.log('   2. Scan QR code from LINE Developers Console');
    console.log('   3. Add as friend');
    return;
  }
  
  const messages = [];
  
  // メインメッセージ
  const mainMessage = {
    type: 'text',
    text: `${alert.message}\n` +
          `レベル: ${alert.level}\n` +
          `時刻: ${new Date().toLocaleString('ja-JP')}`
  };
  messages.push(mainMessage);
  
  // 位置情報があれば地図を追加
  if (alert.location || (sensorData && sensorData.gps)) {
    const loc = alert.location || sensorData.gps;
    messages.push({
      type: 'location',
      title: '📍 現在位置',
      address: `緯度: ${loc.latitude}, 経度: ${loc.longitude}`,
      latitude: loc.latitude,
      longitude: loc.longitude
    });
  }
  
  // クイックリプライ追加（緊急時）
  if (alert.level >= 3) {
    mainMessage.quickReply = {
      items: [
        {
          type: 'action',
          action: {
            type: 'message',
            label: '状態確認',
            text: 'ステータス'
          }
        },
        {
          type: 'action',
          action: {
            type: 'message',
            label: '位置確認',
            text: '位置'
          }
        },
        {
          type: 'action',
          action: {
            type: 'uri',
            label: '緊急連絡',
            uri: 'tel:119'
          }
        }
      ]
    };
  }
  
  // 全ユーザーに送信
  for (const userId of systemStatus.lineUsers) {
    try {
      await client.pushMessage(userId, messages);
      console.log(`✅ Alert sent to ${userId}`);
    } catch (error) {
      console.error(`Failed to send to ${userId}:`, error.message);
    }
  }
}

// ============================================
// LINE Webhookエンドポイント
// ============================================

// Webhook処理（LINE SDKのミドルウェアを使用）
app.post('/webhook', line.middleware(config), async (req, res) => {
  try {
    console.log(`📨 Webhook received with ${req.body.events.length} events`);
    
    const results = await Promise.all(
      req.body.events.map(handleLineEvent)
    );
    
    res.json(results);
  } catch (err) {
    console.error('❌ Webhook error:', err);
    if (err.statusCode === 401) {
      console.error('⚠️ Channel Access Tokenが無効です！');
      console.error('LINE Developers Consoleで新しいトークンを発行してください');
    }
    res.status(500).end();
  }
});

async function handleLineEvent(event) {
  // ユーザーIDを記録
  if (event.source && event.source.userId) {
    const userId = event.source.userId;
    if (!systemStatus.lineUsers.has(userId)) {
      console.log(`👤 New user registered: ${userId}`);
      systemStatus.lineUsers.add(userId);
    }
  }
  
  // メッセージイベント処理
  if (event.type === 'message' && event.message.type === 'text') {
    const userMessage = event.message.text.toLowerCase();
    
    switch(userMessage) {
      case 'ステータス':
      case 'status':
        return handleStatusRequest(event);
      
      case '位置':
      case 'location':
        return handleLocationRequest(event);
      
      case 'ヘルプ':
      case 'help':
        return handleHelpRequest(event);
      
      case 'テスト':
      case 'test':
        return handleTestAlert(event);
      
      default:
        return handleDefaultMessage(event);
    }
  }
  
  // フォローイベント（友だち追加）
  if (event.type === 'follow') {
    return handleFollow(event);
  }
  
  return null;
}

// コマンド処理
async function handleStatusRequest(event) {
  const status = systemStatus.lastSensorData;
  let message = '📊 システムステータス\n';
  message += '━━━━━━━━━━━━━━\n';
  
  if (status) {
    if (status.environment) {
      message += `🌡️ 温度: ${status.environment.temperature?.toFixed(1)}°C\n`;
      message += `💧 湿度: ${status.environment.humidity?.toFixed(1)}%\n`;
    }
    if (status.tof) {
      message += `📏 距離: ${status.tof.distance}mm\n`;
      message += `📍 ゾーン: ${status.tof.zone}\n`;
    }
    if (status.battery) {
      message += `🔋 バッテリー: ${status.battery}%\n`;
    }
    message += `⏰ 更新: ${new Date().toLocaleString('ja-JP')}`;
  } else {
    message += '⚠️ データ未受信';
  }
  
  return client.replyMessage(event.replyToken, {
    type: 'text',
    text: message
  });
}

async function handleLocationRequest(event) {
  const lastDevice = Array.from(systemStatus.devices.values()).pop();
  
  if (lastDevice && lastDevice.location) {
    const loc = lastDevice.location;
    return client.replyMessage(event.replyToken, [
      {
        type: 'text',
        text: `📍 最新の位置情報\n衛星: ${loc.satellites}個`
      },
      {
        type: 'location',
        title: 'デバイスの位置',
        address: `更新: ${loc.lastUpdate}`,
        latitude: loc.latitude,
        longitude: loc.longitude
      }
    ]);
  } else {
    return client.replyMessage(event.replyToken, {
      type: 'text',
      text: '❌ 位置情報がありません\nGPSが取得できていない可能性があります'
    });
  }
}

async function handleHelpRequest(event) {
  const helpMessage = `🤖 Care Copilot ヘルプ
━━━━━━━━━━━━━━
使用可能なコマンド:

📊 ステータス
→ センサー情報を表示

📍 位置
→ GPS位置を表示

🧪 テスト
→ テストアラート送信

❓ ヘルプ
→ このメッセージを表示

━━━━━━━━━━━━━━
緊急時は自動で通知されます`;
  
  return client.replyMessage(event.replyToken, {
    type: 'text',
    text: helpMessage
  });
}

async function handleTestAlert(event) {
  // テストアラート送信
  const testAlert = {
    level: 2,
    message: '🧪 これはテストアラートです',
    location: {
      latitude: 35.6762,
      longitude: 139.6503
    }
  };
  
  await broadcastAlert(testAlert);
  
  return client.replyMessage(event.replyToken, {
    type: 'text',
    text: '✅ テストアラートを送信しました'
  });
}

async function handleDefaultMessage(event) {
  return client.replyMessage(event.replyToken, {
    type: 'text',
    text: '「ヘルプ」と送信すると使い方を表示します'
  });
}

async function handleFollow(event) {
  console.log(`🎉 New follower! User ID: ${event.source.userId}`);
  
  const welcomeMessage = `🎉 Care Copilotへようこそ！

高齢者の見守りシステムです。
以下の機能があります：

🚨 転倒検知
🛏️ 離床検知
🌡️ 環境モニタリング
📍 GPS位置追跡

「ヘルプ」でコマンド一覧を表示`;
  
  return client.replyMessage(event.replyToken, {
    type: 'text',
    text: welcomeMessage
  });
}

// ============================================
// HTTPサーバー起動
// ============================================

// ヘルスチェック
app.get('/health', (req, res) => {
  res.json({
    status: 'OK',
    mqtt: mqttClient.connected,
    users: systemStatus.lineUsers.size,
    devices: systemStatus.devices.size
  });
});

// サーバー起動
app.listen(PORT, () => {
  console.log(`✅ Server running on port ${PORT}`);
  console.log(`📱 Webhook URL: https://your-domain.com/webhook`);
  console.log(`   (Use ngrok for local testing: ngrok http ${PORT})`);
});

// 定期ステータス通知（1時間ごと）
setInterval(async () => {
  if (systemStatus.lineUsers.size > 0) {
    const statusMessage = `📊 定期レポート
━━━━━━━━━━━━━━
✅ システム正常稼働中
📡 MQTT: ${mqttClient.connected ? '接続' : '切断'}
👥 登録ユーザー: ${systemStatus.lineUsers.size}
🔌 デバイス: ${systemStatus.devices.size}
⏰ ${new Date().toLocaleString('ja-JP')}`;
    
    for (const userId of systemStatus.lineUsers) {
      try {
        await client.pushMessage(userId, {
          type: 'text',
          text: statusMessage
        });
      } catch (error) {
        console.error('定期通知エラー:', error.message);
      }
    }
  }
}, 3600000); // 1時間

// グレースフルシャットダウン
process.on('SIGINT', () => {
  console.log('\n👋 Shutting down gracefully...');
  mqttClient.end();
  process.exit();
});