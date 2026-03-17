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

unsigned long previousMillis = 0;
const long interval = 1000;

void setup() {
    Serial.begin(115200);
    
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

    
    if (rfid_check_card()) {
        uint8_t uid[5]; 
        if (rfid_read_uid(uid)) {
            char uidStr[16] = "";
            char hex[4];
            
            Serial.print("Card UID: ");
            for (int i = 0; i < 4; i++) {
                sprintf(hex, "%02X ", uid[i]);
                strcat(uidStr, hex);
                Serial.print(hex);
            }
            Serial.println();
            
            ws_send_rfid(uidStr);
            delay(1000); 
        }
    }

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        
        float lux1 = bh1750_read_lux(Wire);
        float lux2 = bh1750_read_lux(Wire1);
        
        Serial.print("Sensor 1 : ");
        Serial.print(lux1);
        Serial.print(" Lux | Sensor 2 : ");
        Serial.print(lux2);
        Serial.println(" Lux");

        ws_send_lux(lux1, lux2);
    }
    
    delay(100);
}