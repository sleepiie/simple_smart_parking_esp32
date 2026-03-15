#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "rfid.h"
#include "bh1750.h"


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
    
    Serial.println("Ready!");
    Serial.println("Waiting for RFID cards...");
}

void loop() {
    if (rfid_check_card()) {
        uint8_t uid[5]; 
        
        if (rfid_read_uid(uid)) {
            Serial.print("Card UID: ");
            for (int i = 0; i < 4; i++) {
                Serial.print(uid[i] < 0x10 ? " 0" : " ");
                Serial.print(uid[i], HEX);
            }
            Serial.println();
            
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
    }
    
    delay(100);
}