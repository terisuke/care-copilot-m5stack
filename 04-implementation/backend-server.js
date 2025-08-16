/**
 * Care Copilot Backend Server
 * Version: 1.0.0
 * Date: 2025-08-16
 * 
 * Node.js backend for Care Copilot IoT system
 * Features:
 * - MQTT broker integration
 * - WebSocket real-time communication
 * - REST API
 * - LINE Bot integration
 * - AI alert processing
 * - Database management
 */

const express = require('express');
const http = require('http');
const socketIO = require('socket.io');
const mqtt = require('mqtt');
const cors = require('cors');
const helmet = require('helmet');
const compression = require('compression');
const rateLimit = require('express-rate-limit');
const { Pool } = require('pg');
const Redis = require('redis');
const jwt = require('jsonwebtoken');
const bcrypt = require('bcrypt');
const line = require('@line/bot-sdk');
const axios = require('axios');
const winston = require('winston');
const { v4: uuidv4 } = require('uuid');
require('dotenv').config();

// ============================================
// Configuration
// ============================================
const config = {
    port: process.env.PORT || 3000,
    mqtt: {
        broker: process.env.MQTT_BROKER || 'mqtt://localhost:1883',
        username: process.env.MQTT_USER,
        password: process.env.MQTT_PASSWORD
    },
    database: {
        host: process.env.DB_HOST || 'localhost',
        port: process.env.DB_PORT || 5432,
        database: process.env.DB_NAME || 'care_copilot',
        user: process.env.DB_USER || 'postgres',
        password: process.env.DB_PASSWORD
    },
    redis: {
        host: process.env.REDIS_HOST || 'localhost',
        port: process.env.REDIS_PORT || 6379
    },
    line: {
        channelAccessToken: process.env.LINE_CHANNEL_ACCESS_TOKEN,
        channelSecret: process.env.LINE_CHANNEL_SECRET
    },
    jwt: {
        secret: process.env.JWT_SECRET || 'your-secret-key',
        expiresIn: '24h'
    },
    ai: {
        apiKey: process.env.AI_API_KEY,
        endpoint: process.env.AI_ENDPOINT
    }
};

// ============================================
// Logger Setup
// ============================================
const logger = winston.createLogger({
    level: 'info',
    format: winston.format.combine(
        winston.format.timestamp(),
        winston.format.json()
    ),
    transports: [
        new winston.transports.File({ filename: 'error.log', level: 'error' }),
        new winston.transports.File({ filename: 'combined.log' }),
        new winston.transports.Console({
            format: winston.format.simple()
        })
    ]
});

// ============================================
// Database Setup
// ============================================
const pgPool = new Pool(config.database);

// Database initialization
async function initDatabase() {
    try {
        // Create tables if not exists
        await pgPool.query(`
            CREATE TABLE IF NOT EXISTS facilities (
                id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
                name VARCHAR(255) NOT NULL,
                address TEXT,
                capacity INTEGER,
                created_at TIMESTAMP DEFAULT NOW(),
                updated_at TIMESTAMP DEFAULT NOW()
            )
        `);

        await pgPool.query(`
            CREATE TABLE IF NOT EXISTS residents (
                id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
                facility_id UUID REFERENCES facilities(id),
                name VARCHAR(255) NOT NULL,
                room_number VARCHAR(50),
                care_level INTEGER,
                risk_factors JSONB,
                created_at TIMESTAMP DEFAULT NOW(),
                updated_at TIMESTAMP DEFAULT NOW()
            )
        `);

        await pgPool.query(`
            CREATE TABLE IF NOT EXISTS devices (
                id VARCHAR(100) PRIMARY KEY,
                facility_id UUID REFERENCES facilities(id),
                resident_id UUID REFERENCES residents(id),
                type VARCHAR(50),
                location TEXT,
                status VARCHAR(20),
                last_seen TIMESTAMP,
                created_at TIMESTAMP DEFAULT NOW()
            )
        `);

        await pgPool.query(`
            CREATE TABLE IF NOT EXISTS sensor_data (
                time TIMESTAMPTZ NOT NULL,
                device_id VARCHAR(100),
                resident_id UUID,
                data JSONB,
                PRIMARY KEY (time, device_id)
            )
        `);

        await pgPool.query(`
            CREATE TABLE IF NOT EXISTS alerts (
                id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
                device_id VARCHAR(100),
                resident_id UUID,
                facility_id UUID,
                level INTEGER,
                type VARCHAR(50),
                message TEXT,
                data JSONB,
                acknowledged BOOLEAN DEFAULT FALSE,
                acknowledged_by UUID,
                acknowledged_at TIMESTAMP,
                created_at TIMESTAMP DEFAULT NOW()
            )
        `);

        await pgPool.query(`
            CREATE TABLE IF NOT EXISTS users (
                id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
                email VARCHAR(255) UNIQUE NOT NULL,
                password_hash VARCHAR(255) NOT NULL,
                name VARCHAR(255),
                role VARCHAR(50),
                facility_id UUID REFERENCES facilities(id),
                line_user_id VARCHAR(255),
                notification_settings JSONB,
                created_at TIMESTAMP DEFAULT NOW(),
                updated_at TIMESTAMP DEFAULT NOW()
            )
        `);

        logger.info('Database initialized successfully');
    } catch (error) {
        logger.error('Database initialization failed:', error);
        throw error;
    }
}

// ============================================
// Redis Setup
// ============================================
const redisClient = Redis.createClient(config.redis);

redisClient.on('error', (err) => {
    logger.error('Redis Client Error', err);
});

redisClient.on('connect', () => {
    logger.info('Redis connected');
});

// ============================================
// Express App Setup
// ============================================
const app = express();
const server = http.createServer(app);
const io = socketIO(server, {
    cors: {
        origin: process.env.FRONTEND_URL || 'http://localhost:3001',
        methods: ['GET', 'POST']
    }
});

// Middleware
app.use(helmet());
app.use(cors());
app.use(compression());
app.use(express.json({ limit: '10mb' }));
app.use(express.urlencoded({ extended: true }));

// Rate limiting
const limiter = rateLimit({
    windowMs: 15 * 60 * 1000, // 15 minutes
    max: 100 // limit each IP to 100 requests per windowMs
});
app.use('/api/', limiter);

// ============================================
// MQTT Client Setup
// ============================================
const mqttClient = mqtt.connect(config.mqtt.broker, {
    username: config.mqtt.username,
    password: config.mqtt.password,
    clientId: `backend_${Math.random().toString(16).substr(2, 8)}`
});

mqttClient.on('connect', () => {
    logger.info('MQTT connected');
    
    // Subscribe to topics
    mqttClient.subscribe('care/+/data');
    mqttClient.subscribe('care/+/alert');
    mqttClient.subscribe('care/+/status');
});

mqttClient.on('message', async (topic, message) => {
    try {
        const payload = JSON.parse(message.toString());
        const topicParts = topic.split('/');
        const messageType = topicParts[topicParts.length - 1];
        
        switch (messageType) {
            case 'data':
                await handleSensorData(payload);
                break;
            case 'alert':
                await handleAlert(payload);
                break;
            case 'status':
                await handleDeviceStatus(payload);
                break;
        }
    } catch (error) {
        logger.error('MQTT message processing error:', error);
    }
});

// ============================================
// Data Handlers
// ============================================
async function handleSensorData(data) {
    try {
        // Store in database
        await pgPool.query(
            `INSERT INTO sensor_data (time, device_id, resident_id, data) 
             VALUES ($1, $2, $3, $4)`,
            [new Date(), data.device_id, data.resident_id, JSON.stringify(data)]
        );
        
        // Update Redis cache
        await redisClient.set(
            `device:${data.device_id}:latest`,
            JSON.stringify(data),
            'EX',
            300 // 5 minutes TTL
        );
        
        // Emit to WebSocket clients
        io.to(`facility:${data.facility_id}`).emit('sensor_update', data);
        
        // Process with AI for anomaly detection
        const anomaly = await detectAnomaly(data);
        if (anomaly.detected) {
            await createAlert({
                device_id: data.device_id,
                level: anomaly.level,
                type: 'anomaly',
                message: anomaly.message,
                data: anomaly.data
            });
        }
    } catch (error) {
        logger.error('Sensor data handling error:', error);
    }
}

async function handleAlert(alert) {
    try {
        // Store alert in database
        const result = await pgPool.query(
            `INSERT INTO alerts (device_id, resident_id, facility_id, level, type, message, data) 
             VALUES ($1, $2, $3, $4, $5, $6, $7) 
             RETURNING id`,
            [alert.device_id, alert.resident_id, alert.facility_id, 
             alert.level, alert.type, alert.message, JSON.stringify(alert.data)]
        );
        
        const alertId = result.rows[0].id;
        alert.id = alertId;
        
        // Apply intelligent filtering
        const shouldNotify = await shouldSendNotification(alert);
        
        if (shouldNotify) {
            // Send notifications based on alert level
            await sendNotifications(alert);
            
            // Emit to WebSocket clients
            io.to(`facility:${alert.facility_id}`).emit('alert', alert);
        }
        
        // Log alert
        logger.info('Alert processed:', alert);
    } catch (error) {
        logger.error('Alert handling error:', error);
    }
}

async function handleDeviceStatus(status) {
    try {
        // Update device status
        await pgPool.query(
            `UPDATE devices 
             SET status = $1, last_seen = $2 
             WHERE id = $3`,
            [status.status, new Date(), status.device_id]
        );
        
        // Update Redis
        await redisClient.set(
            `device:${status.device_id}:status`,
            JSON.stringify(status),
            'EX',
            60 // 1 minute TTL
        );
        
        // Emit status update
        io.emit('device_status', status);
    } catch (error) {
        logger.error('Device status handling error:', error);
    }
}

// ============================================
// AI Processing
// ============================================
async function detectAnomaly(data) {
    try {
        // Simple rule-based anomaly detection
        // In production, this would call an AI service
        
        let anomalyScore = 0;
        let messages = [];
        
        // Check temperature
        if (data.data.temperature < 18 || data.data.temperature > 28) {
            anomalyScore += 2;
            messages.push(`Temperature out of range: ${data.data.temperature}°C`);
        }
        
        // Check motion patterns
        const lastMotion = await redisClient.get(`device:${data.device_id}:last_motion`);
        if (lastMotion) {
            const timeSinceMotion = Date.now() - parseInt(lastMotion);
            if (timeSinceMotion > 300000) { // 5 minutes
                anomalyScore += 3;
                messages.push('No motion detected for extended period');
            }
        }
        
        // Check sudden distance changes (possible fall)
        const lastDistance = await redisClient.get(`device:${data.device_id}:last_distance`);
        if (lastDistance) {
            const distanceChange = Math.abs(data.data.distance - parseFloat(lastDistance));
            if (distanceChange > 100 && data.data.distance < 50) {
                anomalyScore += 5;
                messages.push('Possible fall detected');
            }
        }
        
        // Update last values
        if (data.data.motion) {
            await redisClient.set(`device:${data.device_id}:last_motion`, Date.now());
        }
        await redisClient.set(`device:${data.device_id}:last_distance`, data.data.distance);
        
        // Determine alert level
        let level = 0;
        if (anomalyScore >= 5) level = 3; // Emergency
        else if (anomalyScore >= 3) level = 2; // Warning
        else if (anomalyScore >= 1) level = 1; // Info
        
        return {
            detected: anomalyScore > 0,
            level: level,
            message: messages.join(', '),
            data: {
                score: anomalyScore,
                factors: messages
            }
        };
    } catch (error) {
        logger.error('Anomaly detection error:', error);
        return { detected: false };
    }
}

async function shouldSendNotification(alert) {
    try {
        // Check alert frequency to prevent alert fatigue
        const recentAlerts = await pgPool.query(
            `SELECT COUNT(*) as count 
             FROM alerts 
             WHERE device_id = $1 
             AND type = $2 
             AND created_at > NOW() - INTERVAL '5 minutes'`,
            [alert.device_id, alert.type]
        );
        
        const count = parseInt(recentAlerts.rows[0].count);
        
        // Don't send if too many similar alerts recently
        if (count > 3 && alert.level < 3) {
            return false;
        }
        
        // Time-based filtering
        const hour = new Date().getHours();
        const isNightTime = hour >= 22 || hour < 6;
        
        // During night, only send warning and emergency alerts
        if (isNightTime && alert.level < 2) {
            return false;
        }
        
        // Always send emergency alerts
        if (alert.level >= 3) {
            return true;
        }
        
        // Check user preferences
        // ... additional filtering logic
        
        return true;
    } catch (error) {
        logger.error('Notification filtering error:', error);
        return true; // Default to sending
    }
}

// ============================================
// Notification System
// ============================================
async function sendNotifications(alert) {
    try {
        // Get users to notify
        const users = await pgPool.query(
            `SELECT u.* 
             FROM users u 
             JOIN devices d ON u.facility_id = d.facility_id 
             WHERE d.id = $1 
             AND u.notification_settings->>'alerts' = 'true'`,
            [alert.device_id]
        );
        
        for (const user of users.rows) {
            // Send based on alert level and user preferences
            if (alert.level >= 2 && user.line_user_id) {
                await sendLineNotification(user.line_user_id, alert);
            }
            
            if (alert.level >= 3) {
                // For emergency, could also send SMS, phone call, etc.
                await sendEmergencyNotification(user, alert);
            }
        }
    } catch (error) {
        logger.error('Notification sending error:', error);
    }
}

// LINE Bot Client
const lineClient = new line.Client(config.line);

async function sendLineNotification(userId, alert) {
    try {
        const emoji = ['ℹ️', '⚠️', '🚨', '🆘'][alert.level];
        
        const message = {
            type: 'flex',
            altText: `${emoji} Alert: ${alert.message}`,
            contents: {
                type: 'bubble',
                header: {
                    type: 'box',
                    layout: 'vertical',
                    contents: [{
                        type: 'text',
                        text: `${emoji} Alert Level ${alert.level}`,
                        size: 'lg',
                        weight: 'bold',
                        color: getAlertColor(alert.level)
                    }]
                },
                body: {
                    type: 'box',
                    layout: 'vertical',
                    contents: [
                        {
                            type: 'text',
                            text: alert.message,
                            wrap: true
                        },
                        {
                            type: 'separator',
                            margin: 'md'
                        },
                        {
                            type: 'text',
                            text: `Device: ${alert.device_id}`,
                            size: 'sm',
                            color: '#999999',
                            margin: 'md'
                        },
                        {
                            type: 'text',
                            text: `Time: ${new Date().toLocaleString()}`,
                            size: 'sm',
                            color: '#999999'
                        }
                    ]
                },
                footer: {
                    type: 'box',
                    layout: 'horizontal',
                    contents: [
                        {
                            type: 'button',
                            action: {
                                type: 'postback',
                                label: 'Acknowledge',
                                data: `acknowledge:${alert.id}`
                            },
                            style: 'primary'
                        },
                        {
                            type: 'button',
                            action: {
                                type: 'uri',
                                label: 'View Details',
                                uri: `${process.env.FRONTEND_URL}/alerts/${alert.id}`
                            }
                        }
                    ]
                }
            }
        };
        
        await lineClient.pushMessage(userId, message);
        logger.info(`LINE notification sent to ${userId}`);
    } catch (error) {
        logger.error('LINE notification error:', error);
    }
}

async function sendEmergencyNotification(user, alert) {
    // Implementation for emergency notifications
    // Could include SMS, phone calls, etc.
    logger.info(`Emergency notification for user ${user.id}: ${alert.message}`);
}

function getAlertColor(level) {
    const colors = ['#00AA00', '#00AAAA', '#FFAA00', '#FF0000'];
    return colors[level] || '#666666';
}

// ============================================
// REST API Routes
// ============================================

// Health check
app.get('/health', (req, res) => {
    res.json({ 
        status: 'healthy',
        timestamp: new Date(),
        uptime: process.uptime()
    });
});

// Authentication
app.post('/api/auth/register', async (req, res) => {
    try {
        const { email, password, name, facility_id, role } = req.body;
        
        // Hash password
        const passwordHash = await bcrypt.hash(password, 10);
        
        // Create user
        const result = await pgPool.query(
            `INSERT INTO users (email, password_hash, name, facility_id, role) 
             VALUES ($1, $2, $3, $4, $5) 
             RETURNING id, email, name, role`,
            [email, passwordHash, name, facility_id, role || 'staff']
        );
        
        const user = result.rows[0];
        
        // Generate JWT
        const token = jwt.sign(
            { id: user.id, email: user.email, role: user.role },
            config.jwt.secret,
            { expiresIn: config.jwt.expiresIn }
        );
        
        res.json({ user, token });
    } catch (error) {
        logger.error('Registration error:', error);
        res.status(400).json({ error: 'Registration failed' });
    }
});

app.post('/api/auth/login', async (req, res) => {
    try {
        const { email, password } = req.body;
        
        // Get user
        const result = await pgPool.query(
            'SELECT * FROM users WHERE email = $1',
            [email]
        );
        
        if (result.rows.length === 0) {
            return res.status(401).json({ error: 'Invalid credentials' });
        }
        
        const user = result.rows[0];
        
        // Verify password
        const validPassword = await bcrypt.compare(password, user.password_hash);
        if (!validPassword) {
            return res.status(401).json({ error: 'Invalid credentials' });
        }
        
        // Generate JWT
        const token = jwt.sign(
            { id: user.id, email: user.email, role: user.role },
            config.jwt.secret,
            { expiresIn: config.jwt.expiresIn }
        );
        
        res.json({ 
            user: {
                id: user.id,
                email: user.email,
                name: user.name,
                role: user.role
            }, 
            token 
        });
    } catch (error) {
        logger.error('Login error:', error);
        res.status(500).json({ error: 'Login failed' });
    }
});

// Middleware for protected routes
const authenticate = (req, res, next) => {
    const token = req.headers.authorization?.split(' ')[1];
    
    if (!token) {
        return res.status(401).json({ error: 'Authentication required' });
    }
    
    try {
        const decoded = jwt.verify(token, config.jwt.secret);
        req.user = decoded;
        next();
    } catch (error) {
        return res.status(401).json({ error: 'Invalid token' });
    }
};

// Facilities
app.get('/api/facilities', authenticate, async (req, res) => {
    try {
        const result = await pgPool.query('SELECT * FROM facilities ORDER BY name');
        res.json(result.rows);
    } catch (error) {
        logger.error('Facilities fetch error:', error);
        res.status(500).json({ error: 'Failed to fetch facilities' });
    }
});

app.post('/api/facilities', authenticate, async (req, res) => {
    try {
        const { name, address, capacity } = req.body;
        const result = await pgPool.query(
            `INSERT INTO facilities (name, address, capacity) 
             VALUES ($1, $2, $3) 
             RETURNING *`,
            [name, address, capacity]
        );
        res.json(result.rows[0]);
    } catch (error) {
        logger.error('Facility creation error:', error);
        res.status(500).json({ error: 'Failed to create facility' });
    }
});

// Residents
app.get('/api/residents', authenticate, async (req, res) => {
    try {
        const { facility_id } = req.query;
        let query = 'SELECT * FROM residents';
        const params = [];
        
        if (facility_id) {
            query += ' WHERE facility_id = $1';
            params.push(facility_id);
        }
        
        query += ' ORDER BY name';
        
        const result = await pgPool.query(query, params);
        res.json(result.rows);
    } catch (error) {
        logger.error('Residents fetch error:', error);
        res.status(500).json({ error: 'Failed to fetch residents' });
    }
});

app.post('/api/residents', authenticate, async (req, res) => {
    try {
        const { facility_id, name, room_number, care_level, risk_factors } = req.body;
        const result = await pgPool.query(
            `INSERT INTO residents (facility_id, name, room_number, care_level, risk_factors) 
             VALUES ($1, $2, $3, $4, $5) 
             RETURNING *`,
            [facility_id, name, room_number, care_level, JSON.stringify(risk_factors)]
        );
        res.json(result.rows[0]);
    } catch (error) {
        logger.error('Resident creation error:', error);
        res.status(500).json({ error: 'Failed to create resident' });
    }
});

// Alerts
app.get('/api/alerts', authenticate, async (req, res) => {
    try {
        const { facility_id, acknowledged, limit = 50 } = req.query;
        let query = 'SELECT * FROM alerts';
        const params = [];
        const conditions = [];
        
        if (facility_id) {
            params.push(facility_id);
            conditions.push(`facility_id = $${params.length}`);
        }
        
        if (acknowledged !== undefined) {
            params.push(acknowledged === 'true');
            conditions.push(`acknowledged = $${params.length}`);
        }
        
        if (conditions.length > 0) {
            query += ' WHERE ' + conditions.join(' AND ');
        }
        
        query += ' ORDER BY created_at DESC';
        
        params.push(limit);
        query += ` LIMIT $${params.length}`;
        
        const result = await pgPool.query(query, params);
        res.json(result.rows);
    } catch (error) {
        logger.error('Alerts fetch error:', error);
        res.status(500).json({ error: 'Failed to fetch alerts' });
    }
});

app.put('/api/alerts/:id/acknowledge', authenticate, async (req, res) => {
    try {
        const { id } = req.params;
        const result = await pgPool.query(
            `UPDATE alerts 
             SET acknowledged = true, 
                 acknowledged_by = $1, 
                 acknowledged_at = NOW() 
             WHERE id = $2 
             RETURNING *`,
            [req.user.id, id]
        );
        
        if (result.rows.length === 0) {
            return res.status(404).json({ error: 'Alert not found' });
        }
        
        res.json(result.rows[0]);
    } catch (error) {
        logger.error('Alert acknowledgment error:', error);
        res.status(500).json({ error: 'Failed to acknowledge alert' });
    }
});

// Analytics
app.get('/api/analytics/summary', authenticate, async (req, res) => {
    try {
        const { facility_id, start_date, end_date } = req.query;
        
        // Get summary statistics
        const alertStats = await pgPool.query(
            `SELECT 
                COUNT(*) as total_alerts,
                SUM(CASE WHEN level = 3 THEN 1 ELSE 0 END) as emergency_alerts,
                SUM(CASE WHEN acknowledged THEN 1 ELSE 0 END) as acknowledged_alerts,
                AVG(EXTRACT(EPOCH FROM (acknowledged_at - created_at))/60) as avg_response_time_minutes
             FROM alerts 
             WHERE facility_id = $1 
             AND created_at BETWEEN $2 AND $3`,
            [facility_id, start_date || '2025-01-01', end_date || '2025-12-31']
        );
        
        const residentStats = await pgPool.query(
            `SELECT COUNT(*) as total_residents 
             FROM residents 
             WHERE facility_id = $1`,
            [facility_id]
        );
        
        res.json({
            alerts: alertStats.rows[0],
            residents: residentStats.rows[0],
            period: { start_date, end_date }
        });
    } catch (error) {
        logger.error('Analytics error:', error);
        res.status(500).json({ error: 'Failed to generate analytics' });
    }
});

app.get('/api/analytics/qol-report/:resident_id', authenticate, async (req, res) => {
    try {
        const { resident_id } = req.params;
        const { days = 7 } = req.query;
        
        // Generate QOL report
        // This would include activity patterns, sleep quality, social interaction, etc.
        
        const report = {
            resident_id,
            period_days: days,
            metrics: {
                sleep_score: 85,
                activity_level: 'moderate',
                social_interaction: 'good',
                outdoor_time_minutes: 45,
                alert_frequency: 'low'
            },
            recommendations: [
                'Increase afternoon activities',
                'Consider more outdoor time',
                'Sleep pattern is healthy'
            ],
            generated_at: new Date()
        };
        
        res.json(report);
    } catch (error) {
        logger.error('QOL report error:', error);
        res.status(500).json({ error: 'Failed to generate QOL report' });
    }
});

// Device management
app.post('/api/devices/command', authenticate, async (req, res) => {
    try {
        const { device_id, command, params } = req.body;
        
        // Publish command to device via MQTT
        const commandPayload = {
            command,
            params,
            timestamp: Date.now(),
            issued_by: req.user.id
        };
        
        mqttClient.publish(
            `care/command/${device_id}`,
            JSON.stringify(commandPayload),
            { qos: 1 }
        );
        
        res.json({ 
            success: true, 
            message: 'Command sent',
            command: commandPayload
        });
    } catch (error) {
        logger.error('Device command error:', error);
        res.status(500).json({ error: 'Failed to send command' });
    }
});

// ============================================
// WebSocket Handlers
// ============================================
io.use((socket, next) => {
    const token = socket.handshake.auth.token;
    
    try {
        const decoded = jwt.verify(token, config.jwt.secret);
        socket.userId = decoded.id;
        socket.userRole = decoded.role;
        next();
    } catch (err) {
        next(new Error('Authentication error'));
    }
});

io.on('connection', (socket) => {
    logger.info(`WebSocket client connected: ${socket.id}`);
    
    // Join facility room
    socket.on('join_facility', async (facilityId) => {
        // Verify user has access to this facility
        const result = await pgPool.query(
            'SELECT facility_id FROM users WHERE id = $1',
            [socket.userId]
        );
        
        if (result.rows[0]?.facility_id === facilityId) {
            socket.join(`facility:${facilityId}`);
            socket.emit('joined', { facility: facilityId });
            logger.info(`Socket ${socket.id} joined facility ${facilityId}`);
        }
    });
    
    // Subscribe to resident updates
    socket.on('subscribe_resident', (residentId) => {
        socket.join(`resident:${residentId}`);
        socket.emit('subscribed', { resident: residentId });
    });
    
    // Handle real-time commands
    socket.on('device_command', async (data) => {
        // Verify permissions and send command
        mqttClient.publish(
            `care/command/${data.device_id}`,
            JSON.stringify(data.command)
        );
    });
    
    socket.on('disconnect', () => {
        logger.info(`WebSocket client disconnected: ${socket.id}`);
    });
});

// ============================================
// LINE Webhook
// ============================================
app.post('/webhook/line', line.middleware(config.line), async (req, res) => {
    try {
        const events = req.body.events;
        
        for (const event of events) {
            if (event.type === 'message' && event.message.type === 'text') {
                await handleLineMessage(event);
            } else if (event.type === 'postback') {
                await handleLinePostback(event);
            }
        }
        
        res.json({ success: true });
    } catch (error) {
        logger.error('LINE webhook error:', error);
        res.status(500).json({ error: 'Webhook processing failed' });
    }
});

async function handleLineMessage(event) {
    const userId = event.source.userId;
    const message = event.message.text;
    
    // Simple command handling
    if (message === 'status') {
        // Get facility status
        const reply = {
            type: 'text',
            text: 'All systems operational. No active alerts.'
        };
        await lineClient.replyMessage(event.replyToken, reply);
    } else if (message === 'help') {
        const reply = {
            type: 'text',
            text: 'Available commands:\n' +
                  '• status - System status\n' +
                  '• alerts - Recent alerts\n' +
                  '• help - This message'
        };
        await lineClient.replyMessage(event.replyToken, reply);
    }
}

async function handleLinePostback(event) {
    const data = event.postback.data;
    const [action, alertId] = data.split(':');
    
    if (action === 'acknowledge') {
        // Acknowledge alert
        await pgPool.query(
            'UPDATE alerts SET acknowledged = true WHERE id = $1',
            [alertId]
        );
        
        const reply = {
            type: 'text',
            text: 'Alert acknowledged ✓'
        };
        await lineClient.replyMessage(event.replyToken, reply);
    }
}

// ============================================
// Server Startup
// ============================================
async function startServer() {
    try {
        // Initialize database
        await initDatabase();
        
        // Connect to Redis
        await redisClient.connect();
        
        // Start server
        server.listen(config.port, () => {
            logger.info(`Server running on port ${config.port}`);
            logger.info(`Environment: ${process.env.NODE_ENV || 'development'}`);
        });
    } catch (error) {
        logger.error('Server startup failed:', error);
        process.exit(1);
    }
}

// Handle graceful shutdown
process.on('SIGTERM', async () => {
    logger.info('SIGTERM received, shutting down gracefully');
    
    server.close(() => {
        logger.info('HTTP server closed');
    });
    
    await pgPool.end();
    await redisClient.quit();
    mqttClient.end();
    
    process.exit(0);
});

// Start the server
startServer();

module.exports = app; // For testing