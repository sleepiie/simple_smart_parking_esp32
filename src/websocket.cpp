#include "websocket.h"
#include <WiFi.h>
#include <WebSocketsClient.h>

static WebSocketsClient webSocket;
volatile int ws_auth_status = 0;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("[WS] Disconnected!");
            break;
        case WStype_CONNECTED:
            Serial.println("[WS] Connected to server!");
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

void ws_send_lux(float lux1, float lux2) {
    char json[128];
    snprintf(json, sizeof(json), "{\"type\":\"lux\",\"sensor1\":%.2f,\"sensor2\":%.2f}", lux1, lux2);
    webSocket.sendTXT(json);
}