#include "bh1750.h"

void bh1750_init(TwoWire& wireObj) {
    wireObj.beginTransmission(BH1750_ADDR);
    wireObj.write(BH1750_POWER_ON);
    wireObj.endTransmission();

    wireObj.beginTransmission(BH1750_ADDR);
    wireObj.write(BH1750_CONT_H_RES_MODE);
    wireObj.endTransmission();
    
    delay(150); 
}

float bh1750_read_lux(TwoWire& wireObj) {
    uint16_t level = 0;
    
    wireObj.requestFrom(BH1750_ADDR, 2); 
    
    if (wireObj.available() == 2) {
        uint8_t msb = wireObj.read(); 
        uint8_t lsb = wireObj.read(); 
        level = (msb << 8) | lsb;  
        return level / 1.2f;
    }
    
    return -1.0f;
}