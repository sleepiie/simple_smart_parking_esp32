#include "rfid.h"
#include <SPI.h>

static uint8_t _ss_pin;
static uint8_t _rst_pin;

//read write register in SPI protocol

static void write_reg(uint8_t addr, uint8_t val) {
    digitalWrite(_ss_pin, LOW);
    SPI.transfer((addr << 1) & 0x7E); // lowest bit is 0 for write
    SPI.transfer(val);
    digitalWrite(_ss_pin, HIGH);
}

static uint8_t read_reg(uint8_t addr) {
    digitalWrite(_ss_pin, LOW);
    SPI.transfer(((addr << 1) & 0x7E) | 0x80); // highest bit is 1 for read
    uint8_t val = SPI.transfer(0x00);
    digitalWrite(_ss_pin, HIGH);
    return val;
}

bool rfid_test_connection() {
    uint8_t v = read_reg(0x37); 
    return (v == 0x91 || v == 0x92); 
}

static void set_bit_mask(uint8_t addr, uint8_t mask) {
    write_reg(addr, read_reg(addr) | mask);
}

static void clear_bit_mask(uint8_t addr, uint8_t mask) {
    write_reg(addr, read_reg(addr) & (~mask));
}

// turn on antenna
static void antenna_on() {
    uint8_t temp = read_reg(TxControlReg);
    if (!(temp & 0x03)) {
        set_bit_mask(TxControlReg, 0x03);
    }
}

// communicate with card
static bool to_card(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint16_t *backLen) {
    uint8_t irqEn = 0x77;
    uint8_t waitIRq = 0x30;
    
    write_reg(ComIEnReg, irqEn | 0x80);
    clear_bit_mask(ComIrqReg, 0x80);
    set_bit_mask(FIFOLevelReg, 0x80); // FlushBuffer
    write_reg(CommandReg, PCD_IDLE);

    for (uint8_t i = 0; i < sendLen; i++) {
        write_reg(FIFODataReg, sendData[i]);
    }

    write_reg(CommandReg, command);
    if (command == PCD_TRANSCEIVE) {
        set_bit_mask(BitFramingReg, 0x80); // StartSend
    }

    uint16_t i = 2000;
    uint8_t n;
    while (true) {
        n = read_reg(ComIrqReg);
        i--;
        if ((i == 0) || (n & 0x01) || (n & waitIRq)) break;
    }

    clear_bit_mask(BitFramingReg, 0x80);

    if (i != 0 && !(read_reg(ErrorReg) & 0x1B)) {
        if (command == PCD_TRANSCEIVE) {
            n = read_reg(FIFOLevelReg);
            uint8_t lastBits = read_reg(ControlReg) & 0x07;
            if (lastBits) *backLen = (n - 1) * 8 + lastBits;
            else *backLen = n * 8;
            
            if (n == 0) n = 1;
            if (n > 16) n = 16;
            for (i = 0; i < n; i++) backData[i] = read_reg(FIFODataReg);
            return true;
        }
    }
    return false;
}

// for call in main.c
void rfid_init(uint8_t ss_pin, uint8_t rst_pin) {
    _ss_pin = ss_pin;
    _rst_pin = rst_pin;
    pinMode(_ss_pin, OUTPUT);
    pinMode(_rst_pin, OUTPUT);
    
    digitalWrite(_ss_pin, HIGH);
    digitalWrite(_rst_pin, HIGH);
    delay(50);

    write_reg(CommandReg, PCD_RESETPHASE); // Soft reset
    delay(50);

    write_reg(ModeReg, 0x3D);
    write_reg(TxASKReg, 0x40);
    antenna_on();
}

bool rfid_check_card() {
    uint8_t reqMode = PICC_REQIDL;
    uint8_t backData[2];
    uint16_t backLen;
    write_reg(BitFramingReg, 0x07);
    return to_card(PCD_TRANSCEIVE, &reqMode, 1, backData, &backLen);
}

bool rfid_read_uid(uint8_t* uid) {
    uint8_t cmd[2] = {PICC_ANTICOLL, 0x20};
    uint16_t backLen;
    write_reg(BitFramingReg, 0x00);
    
    if (to_card(PCD_TRANSCEIVE, cmd, 2, uid, &backLen)) {
        uint8_t checksum = 0;
        for (uint8_t i = 0; i < 4; i++) {
            checksum ^= uid[i];
        }
        if (checksum == uid[4]) return true; 
    }
    return false;
}