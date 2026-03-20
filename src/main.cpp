#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "rfid.h"
#include "bh1750.h"
#include "websocket.h"
#include "secrets.h"

#define SS_PIN  5
#define RST_PIN 27

#define SDA1_PIN 21
#define SCL1_PIN 22 

#define SDA2_PIN 32 
#define SCL2_PIN 33

#define LED_GREEN_PIN 26

enum SystemState {
    S0_WAITING,     
    S1_CHECKING_WS, 
    S2_RESULT,    
    S3_GATE_OPEN 
};

SystemState currentState = S0_WAITING;
unsigned long stateTimer = 0; 
unsigned long previousMillis = 0;
const long interval = 1000;

void setup() {
    Serial.begin(115200);

    //led
    pinMode(LED_GREEN_PIN, OUTPUT);
    digitalWrite(LED_GREEN_PIN, LOW);
    
    //rfid
    SPI.begin(); 
    rfid_init(SS_PIN, RST_PIN);
    
    //bh1750
    Wire.begin(SDA1_PIN, SCL1_PIN);
    bh1750_init(Wire); 
    
    Wire1.begin(SDA2_PIN, SCL2_PIN);
    bh1750_init(Wire1);

    ws_init(WIFI_SSID, WIFI_PASS, SERVER_IP, SERVER_PORT);
    
    Serial.println("Ready!");
    Serial.println("Waiting for RFID cards...");
}

void loop() {
    ws_loop(); 

    //rfid state machine
    switch (currentState) {
        
        case S0_WAITING:
            if (rfid_check_card()) {
                uint8_t uidRaw[5]; 
                if (rfid_read_uid(uidRaw)) {
                    char uidStr[16] = "";
                    char hex[4];
                    for (int i = 0; i < 4; i++) {
                        sprintf(hex, "%02X ", uidRaw[i]);
                        strcat(uidStr, hex);
                    }
                    uidStr[strlen(uidStr) - 1] = '\0'; 
                    
                    Serial.print("Scanned: ");
                    Serial.println(uidStr);
                    
                    ws_auth_status = 0;
                    ws_send_rfid(uidStr);
                    
                    stateTimer = millis(); 
                    currentState = S1_CHECKING_WS; 
                }
            }
            break;

        case S1_CHECKING_WS:
            if (ws_auth_status == 1 || ws_auth_status == 2) {
                currentState = S2_RESULT;
            } else if (millis() - stateTimer > 5000) {
                Serial.println("Server Timeout! return to s0");
                currentState = S0_WAITING;
                delay(1000);
            }
            break;

        case S2_RESULT:
            if (ws_auth_status == 1) {
                Serial.println("STATUS: ACCEPTED");
                digitalWrite(LED_GREEN_PIN, HIGH);
                stateTimer = millis();
                currentState = S3_GATE_OPEN;
            } else {
                Serial.println("STATUS: INVALID CARD");
                delay(2000);
                currentState = S0_WAITING;
            }
            break;

        case S3_GATE_OPEN:
            if (millis() - stateTimer >= 10000) {
                Serial.println("Barrier Closing...");
                digitalWrite(LED_GREEN_PIN, LOW);
                currentState = S0_WAITING;
            } else {
                if ((millis() - stateTimer) % 2000 == 0) {
                    Serial.println("Barrier Opening...");
                }
            }
            break;
    }

    //bh1750
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        float lux1 = bh1750_read_lux(Wire);
        float lux2 = bh1750_read_lux(Wire1);
        ws_send_lux(lux1, lux2);
    }
    delay(10);
}