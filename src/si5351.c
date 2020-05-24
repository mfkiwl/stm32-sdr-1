#include "si5351.h"

void si5351_write_reg(uint8_t reg, uint8_t value) {
    i2c_start(DEV_WRITE_ADDRESS);
    i2c_write(reg);
    i2c_write(value);
    i2c_stop();
}

void si5351_powerdown() {
    // disable outputs
    si5351_write_reg(OUTPUT_ENABLE_CONTROL, 0xff);
    // power down all output drivers
    i2c_start(DEV_WRITE_ADDRESS);
    i2c_write(CLK0_CONTROL);
    for (uint8_t i = 0; i < 8; i ++) {
        i2c_write(0x80);
    }
    i2c_stop();
}

// write configuration to MSNA or MSNB registers
void si5351_write_fbms_config(uint8_t msn, FBMS_Config config) {
    uint8_t buf[9] = {0};
    buf[0] = msn;
    buf[1] = (config.P3 >> 8) & 0xff;
    buf[2] = (config.P3 & 0xff);
    buf[3] = (config.P1 >> 16) & 0x03;
    buf[4] = (config.P1 >> 8) & 0xff;
    buf[5] = (config.P1 & 0xff);
    buf[6] = ((config.P3 >> 12) & 0xf0) | ((config.P2 >> 16) & 0x0f);
    buf[7] = (config.P2 >> 8) & 0xff;
    buf[8] = (config.P2 & 0xff);
    i2c_write_buf(DEV_WRITE_ADDRESS, buf, 9);
}

void si5351_write_oms05_config(uint8_t ms, OMS05_Config config) {
    uint8_t buf[9] = {0};
    buf[0] = ms;
    buf[1] = (config.P3 >> 8) & 0xff;
    buf[2] = (config.P3 & 0xff);
    buf[3] = (config.DIV<<4) | (config.DIVBY4<<2) | ((config.P1>>16)& 0x03);
    buf[4] = (config.P1 >> 8) & 0xff;
    buf[5] = (config.P1 & 0xff);
    buf[6] = ((config.P3 >> 12) & 0xf0) | ((config.P2 >> 16) & 0x0f);
    buf[7] = (config.P2 >> 8) & 0xff;
    buf[8] = (config.P2 & 0xff);
    i2c_write_buf(DEV_WRITE_ADDRESS, buf, 9);
}

void si5351_write_oms67_config(OMS67_Config config) {
    uint8_t buf[4] = {0};
    buf[0] = MS6;
    buf[1] = config.MS6_P1;
    buf[2] = config.MS7_P1;
    buf[3] = (config.R7_DIV<<4) | (config.R6_DIV);
    i2c_write_buf(DEV_WRITE_ADDRESS, buf, 4);
}

void si5351_write_ssc_config(SSC_Config config) {
    uint8_t buf[14] = {0};
    buf[0] = SSC;
    buf[1] = (config.SSC_EN<<7) | ((config.DN_P2 >> 8) & 0x7f);
    buf[2] = (config.DN_P2 & 0xff);
    buf[3] = (config.SSC_MODE<<7) | ((config.DN_P3 >> 8) & 0x7f);
    buf[4] = (config.DN_P3 & 0xff);
    buf[5] = (config.DN_P1 & 0xff);
    buf[6] = ((config.UDP >> 4) & 0xf0) | ((config.DN_P1 >> 8) & 0x0f);
    buf[7] = (config.UDP & 0xff);
    buf[8] = (config.UP_P2 >> 8) & 0x7f;
    buf[9] = (config.UP_P2 & 0xff);
    buf[10] = (config.UP_P3 >> 8) & 0x7f;
    buf[11] = (config.UP_P3 & 0xff);
    buf[12] = (config.UP_P1 & 0xff);
    buf[13] = (config.NCLK<<4) | ((config.UP_P1 >> 16) & 0x0f);
    i2c_write_buf(DEV_WRITE_ADDRESS, buf, 14);
}

void si5351_write_vcxo_param(uint32_t param) {
    uint8_t buf[4] = {0};
    buf[0] = VCXO_PARAM;
    buf[1] = param & 0xff;
    buf[2] = (param>>8) & 0xff;
    buf[4] = (param>>16) & 0x3f;
    i2c_write_buf(DEV_WRITE_ADDRESS, buf, 4);
}
