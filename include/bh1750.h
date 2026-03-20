#ifndef BH1750_H
#define BH1750_H

#include <Arduino.h>
#include <Wire.h>

#define BH1750_ADDR 0x23 

#define BH1750_POWER_ON        0x01
#define BH1750_RESET           0x07
#define BH1750_CONT_H_RES_MODE 0x10

void bh1750_init(TwoWire& wireObj);
float bh1750_read_lux(TwoWire& wireObj);

#endif