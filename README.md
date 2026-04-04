# Simple Smart Parking ESP32

A Smart Parking system project that works collaboratively between an **ESP32** board (acting to read sensor values and send data) and a **Node.js Web Application / Discord Bot** (receiving data via WebSockets and displaying/notifying).

---

## 1. Wiring



**BH1750_1**

| Module Pin | ESP32 Pin |
| :--- | :--- |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

**BH1750_2**

| Module Pin | ESP32 Pin |
| :--- | :--- |
| SDA | GPIO 32 |
| SCL | GPIO 33 |

**LED**
| Module Pin | ESP32 Pin |
| :--- | :--- |
| LED | GPIO 26 |

**SPI RFID**
| Module Pin | ESP32 Pin |
| :--- | :--- |
| SDA (CS) | GPIO 5 |
| SCK | GPIO 18 |
| MOSI | GPIO 23 |
| MISO | GPIO 19 |
| RST | GPIO 27 |

---

## 2. Setup `secrets.h` for ESP32

This file is used to store hardware-side secrets, such as Wi-Fi credentials and the Server IP.

**Where to create:** 
Create a file named `secrets.h` in the `include/` folder of the ESP32 project.

**Code to insert:**
```cpp
#ifndef SECRETS_H
#define SECRETS_H

const char* WIFI_SSID = "your_wifi_name";
const char* WIFI_PASSWORD = "your_wifi_password";

const char* WEBSOCKET_HOST = "192.168.1.xxx"; //your webserver ip
const uint16_t WEBSOCKET_PORT = 3000; //or your webserver port

#endif
```

---

## 3. Setup `.env` for Discord notification

This file is used to store environment variables for the Node.js Server.

**Where to create:** 
Create a file named `.env` in the `webapp/` folder.

**Code to insert:**
```env
DISCORD_BOT_TOKEN=your_bot_token
MY_DISCORD_USER_ID=your_user_id
MY_DISCORD_CHANNEL_ID=your_channel_id
```

---

## How to Run

Running this project is divided into 2 parts: uploading code to the board and starting the server.

### Step 1: Upload code to ESP32
1. Open this project with **VS Code** that has the **PlatformIO** extension installed.
2. Plug the USB cable into the ESP32 board.
3. Click the **Build** button (checkmark ✔️ on the bottom bar) to compile the code.
4. Click the **Upload** button (right arrow ➡️) to flash the code to the board.
5. (Optional) Click the **Serial Monitor** button (plug icon 🔌) to check if the ESP32 successfully connected to Wi-Fi.

### Step 2: Run Web Application (Node.js)
1. Open a new Terminal and navigate to the `webapp/` folder.
   ```bash
   cd webapp
   ```
2. Install all necessary packages (only required the first time).
   ```bash
   npm install
   ```
3. Start the server.
   ```bash
   npm start
   ```

4. Once the Server is running and the ESP32 successfully connects to Wi-Fi, the ESP32 board will immediately connect to the WebSocket, and you can view parking status updates via the web page.

---

## Need Help?

If you encounter any problems or have any questions about this project, feel free to ask Generative AI tools like **Gemini**, **ChatGPT**, or whatever you prefer! :)