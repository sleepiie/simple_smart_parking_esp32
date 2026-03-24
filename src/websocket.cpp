#include "websocket.h"
#include <WiFi.h>
#include <WebSocketsClient.h>

static WebSocketsClient webSocket;
volatile int ws_auth_status = 0;
volatile bool ws_connected = false;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("[WS] Disconnected!");
            ws_connected = false;
            break;
        case WStype_CONNECTED:
            Serial.println("[WS] Connected to server!");
            ws_connected = true;
            break;
        case WStype_TEXT:
            if (strstr((char*)payload, "\"type\":\"auth\"")) {
                if (strstr((char*)payload, "\"valid\":true")) {
                    ws_auth_status = 1;
                } else if (strstr((char*)payload, "\"valid\":false")) {
                    ws_auth_status = 2;
                }
            }
            break;
    }
}

void ws_init(const char* ssid, const char* password, const char* server_ip, uint16_t server_port) {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected! IP: ");
    Serial.println(WiFi.localIP());

    webSocket.begin(server_ip, server_port, "/");
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
}

void ws_loop() {
    webSocket.loop();
}

void ws_send_rfid(const char* uid) {
    char json[64];
    snprintf(json, sizeof(json), "{\"type\":\"rfid\",\"uid\":\"%s\"}", uid);
    webSocket.sendTXT(json);
}

void ws_send_slot_state(int slotId, bool isParked, float lux) {
    char json[128];
    snprintf(json, sizeof(json), "{\"type\":\"slot_state\",\"slot\":%d,\"is_parked\":%s,\"lux\":%.2f}", 
             slotId, isParked ? "true" : "false", lux);
    webSocket.sendTXT(json);
}

void ws_send_error(const char* msg) {
    char json[512];
    snprintf(json, sizeof(json), "{\"type\":\"error\",\"message\":\"%s\"}", msg);
    webSocket.sendTXT(json);
}