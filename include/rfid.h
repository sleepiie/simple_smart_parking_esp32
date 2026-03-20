#ifndef RFID_H
#define RFID_H

#include <Arduino.h>

// MFRC522 Registers
#define CommandReg     0x01
#define ComIEnReg      0x02
#define ComIrqReg      0x04
#define ErrorReg       0x06
#define Status2Reg     0x08
#define FIFODataReg    0x09
#define FIFOLevelReg   0x0A
#define ControlReg     0x0C
#define BitFramingReg  0x0D
#define ModeReg        0x11
#define TxControlReg   0x14
#define TxASKReg       0x15

// MFRC522 Commands
#define PCD_IDLE       0x00
#define PCD_TRANSCEIVE 0x0C
#define PCD_RESETPHASE 0x0F
#define PICC_REQIDL    0x26
#define PICC_ANTICOLL  0x93

void rfid_init(uint8_t ss_pin, uint8_t rst_pin);
bool rfid_check_card();
bool rfid_read_uid(uint8_t* uid);
bool rfid_test_connection();

#endif