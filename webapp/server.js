require('dotenv').config();

const express = require('express');
const http = require('http');
const WebSocket = require('ws');
const path = require('path');
const sqlite3 = require('sqlite3').verbose();

//discord connect
const { Client, GatewayIntentBits, EmbedBuilder } = require('discord.js');
const discordClient = new Client({ intents: [GatewayIntentBits.Guilds] });

const DISCORD_TOKEN = process.env.DISCORD_BOT_TOKEN;
const MY_DISCORD_USER_ID = process.env.MY_DISCORD_USER_ID;

discordClient.once('ready', () => {
    console.log(`Discord bot logged in as ${discordClient.user.tag}`);
});
discordClient.login(DISCORD_TOKEN);

//discord alert
function sendDiscordAlert(message) {
    discordClient.users.fetch(MY_DISCORD_USER_ID).then(user => {
        const alertEmbed = new EmbedBuilder()
            .setColor('#ff0000')
            .setTitle('🚨 Smart Parking Alert')
            .setDescription(message)
            .setTimestamp();

        user.send({ embeds: [alertEmbed] })
            .catch(err => console.error("⚠️ Discord DM Error:", err.message));
            
    }).catch(err => console.error("⚠️ หา Discord User ไม่เจอ:", err.message));
}

const db = new sqlite3.Database('./parking.db', (err) => {
    if (err) console.error("Database connection error:", err.message);
    else console.log('Connected to SQLite database (parking.db)');
});

db.serialize(() => {
    db.run(`CREATE TABLE IF NOT EXISTS cards (
        uid TEXT PRIMARY KEY,
        is_parked BOOLEAN DEFAULT 0
    )`);

    db.run(`CREATE TABLE IF NOT EXISTS logs (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        uid TEXT,
        action TEXT,
        timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
    )`);

    const insertMock = db.prepare(`INSERT OR IGNORE INTO cards (uid, is_parked) VALUES (?, ?)`);
    insertMock.run('99 1C 46 31', 0);
    insertMock.run('79 DE B8 59', 0);
    insertMock.finalize();
});

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

let latestSlotState = {
    1: { is_parked: false, lux: 0 },
    2: { is_parked: false, lux: 0 }
};

app.use(express.static(path.join(__dirname, 'pages')));

wss.on('connection', (ws) => {
    console.log('New client connected');

    ws.on('error', (err) => console.error('⚠️ [WS] Connection Error:', err.message));

    ws.send(JSON.stringify({ type: 'slot_state', slot: 1, ...latestSlotState[1] }));
    ws.send(JSON.stringify({ type: 'slot_state', slot: 2, ...latestSlotState[2] }));

    ws.on('message', (message) => {
        const data = JSON.parse(message.toString());

        if (data.type === 'rfid') {
            const uid = data.uid;
            
            db.get(`SELECT * FROM cards WHERE uid = ?`, [uid], (err, row) => {
                if (err) {
                    console.error("DB Error:", err.message);
                    return;
                }

                if (row) {
                    const newStatus = row.is_parked ? 0 : 1; 
                    const actionText = newStatus ? 'IN' : 'OUT';

                    db.run(`UPDATE cards SET is_parked = ? WHERE uid = ?`, [newStatus, uid]);
                    db.run(`INSERT INTO logs (uid, action) VALUES (?, ?)`, [uid, actionText]);
                    console.log(`[AUTH] Card: ${uid} | Action: ${actionText}`);

                    ws.send(JSON.stringify({ type: 'auth', valid: true }));

                    wss.clients.forEach(client => {
                        if (client !== ws && client.readyState === WebSocket.OPEN) {
                            client.send(JSON.stringify({ 
                                type: 'rfid_log', 
                                uid: uid, 
                                action: actionText 
                            }));
                        }
                    });

                } else {
                    console.log(`[AUTH] Invalid Card Rejected: ${uid}`);
                    ws.send(JSON.stringify({ type: 'auth', valid: false }));
                }
            });
        } 
        else if (data.type === 'slot_state') {
            latestSlotState[data.slot] = { is_parked: data.is_parked, lux: data.lux };

            wss.clients.forEach(client => {
                if (client !== ws && client.readyState === WebSocket.OPEN) {
                    client.send(message.toString());
                }
            });
        }
        else if (data.type === 'error') {
            console.error(`[HARDWARE ERROR] ${data.message}`);
            sendDiscordAlert(data.message);
        }
    });

    ws.on('close', () => console.log('Client disconnected'));
});

const PORT = 3000;
server.listen(PORT, '0.0.0.0', () => {
    console.log(`Server running on http://localhost:${PORT}`);
});