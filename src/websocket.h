#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <Arduino.h>
extern volatile int ws_auth_status;

void ws_init(const char* ssid, const char* password, const char* server_ip, uint16_t server_port);
void ws_loop();
void ws_send_rfid(const char* uid);
void ws_send_slot_state(int slotId, bool isParked, float lux);

#endif