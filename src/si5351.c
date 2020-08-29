#include "si5351.h"

void si5351_init() {
    FBMS_Config fbmsa;
    OMS05_Config oms;

    fbmsa.P1 = 2570;
    fbmsa.P2 = 251658;
    fbmsa.P3 = 1048575;

    oms.P1 = 10496;
    oms.P2 = 0;
    oms.P3 = 1;
    oms.DIV = 0;
    oms.DIVBY4 = 0;

    si5351_powerdown();
    si5351_write_fbms_config(MSNA, fbmsa);
    si5351_write_oms05_config(MS0, oms);
    si5351_write_reg(XTAL_LOAD_CAPACITANCE, XTAL_CL1 | XTAL_CL0 | 0x12);
    si5351_write_reg(CLK0_CONTROL, 0x0C);
    si5351_write_reg(PLL_RESET, 0xA0);
    si5351_write_reg(OUTPUT_ENABLE_CONTROL, ~CLK0_OEB);
}

void si5351_freq2coeff(uint32_t freq, uint32_t* abc, uint32_t* def) {
    const uint32_t fxtal = 25000000;
    // start with 600 MHz VCO test frequency
    uint32_t fvco = 600000000;
    // round up the needed output divider ratio to next highest integer
    uint32_t omd = (uint32_t)ceilf((float)fvco/(float)freq);
    // calculate resulting VCO frequency
    //fvco = omd*freq;
    // calculate PLL feedback ratio
    float fmd = (float)omd*(float)freq/(float)fxtal;
    abc[0] = (uint32_t)floorf(fmd);
    abc[2] = 0x000fffff;
    abc[1] = (uint32_t)roundf((fmd - (float)abc[0]) * (float)abc[2]);
    // output divider
    def[0] = omd;
    def[1] = 0;
    def[2] = 1;
}

void si5351_coeff2param(uint32_t* coeff, uint32_t* param) {
    param[0] = (coeff[0]<<7) + (uint32_t)(128.0f*(float)coeff[1]/(float)coeff[2]) - 512;
    param[1] = (coeff[1]<<7) - coeff[2]*(uint32_t)(128.0f*(float)coeff[1]/(float)coeff[2]);
    param[2] = coeff[2];
}

void si5351_set_frequency(uint32_t freq) {
    FBMS_Config fbmsa;
    OMS05_Config oms;
    uint32_t abc[3] = {0};
    uint32_t def[3] = {0};
    uint32_t param[3] = {0};

    si5351_freq2coeff(freq, abc, def);
    // PLL feedback multisynth
    si5351_coeff2param(abc, param);
    fbmsa.P1 = param[0];
    fbmsa.P2 = param[1];
    fbmsa.P3 = param[2];
    si5351_write_fbms_config(MSNA, fbmsa);
    // output multisynth
    si5351_coeff2param(def, param);
    oms.P1 = param[0];
    oms.P2 = param[1];
    oms.P3 = param[2];
    oms.DIV = 0;
    oms.DIVBY4 = 0;
    si5351_write_oms05_config(MS0, oms);
    si5351_write_reg(PLL_RESET, PLLA_RST);
}

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
    buf[3] = (param>>16) & 0x3f;
    i2c_write_buf(DEV_WRITE_ADDRESS, buf, 4);
}
