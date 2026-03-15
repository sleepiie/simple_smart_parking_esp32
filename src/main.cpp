#include <Arduino.h>
#include <SPI.h>
#include "rfid.h"

#define SS_PIN  5
#define RST_PIN 22

void setup() {
    Serial.begin(115200);
    SPI.begin(); 
    
    rfid_init(SS_PIN, RST_PIN);
    Serial.println("Ready to read RFID cards...");
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
    delay(50);
}