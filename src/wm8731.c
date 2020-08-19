#include "wm8731.h"

void wm8731_init() {
    wm8731_reset();
    // power down:
    // microphone input
    // crystal oscillator
    // MCLK output
    wm8731_write_reg(WM8731_POWER_DOWN_CONTROL,
                     WM8731_MICPD |
                     WM8731_OSCPD |
                     WM8731_CLKOUTPD);
    // disable bypass
    wm8731_write_reg(WM8731_ANALOG_AUDIO_PATH_CONTROL,
                     WM8731_MUTEMIC |
                     WM8731_DACSEL);
    // unmute DAC, enable high pass filter
    wm8731_write_reg(WM8731_DIGITAL_AUDIO_PATH_CONTROL, 0);
    // audio interface I2S
    // 32 bit word length
    wm8731_write_reg(WM8731_DIGITAL_AUDIO_INTERFACE_FORMAT,
                     WM8731_FORMAT_1 |
                     WM8731_IWL_1 |
                     WM8731_IWL_0 |
                     WM8731_LRP);

    wm8731_write_reg(WM8731_SAMPLING_CONTROL, 0);
    wm8731_set_hp_volume(0, BOTH);
    wm8731_set_line_in_volume(55, BOTH);
    wm8731_active();
}

void wm8731_write_reg(uint8_t reg, uint16_t value) {
    i2c_start(WM8731_WRITE_ADDRESS);
    i2c_write((uint8_t)((reg<<1) |
              (uint8_t)((value & 0x100)>>8)));
    i2c_write((uint8_t)(value & 0xFF));
    i2c_stop();
}

void wm8731_reset() {
    wm8731_write_reg(WM8731_RESET, 0);
}

// activate digital audio interface
void wm8731_active() {
    wm8731_write_reg(WM8731_ACTIVE_CONTROL, WM8731_ACTIVE);
}

// set the line input volume to vol
// vol: from 0 (=-34.5 dB) to 63 (=+12 dB) in 1.5 dB steps
void wm8731_set_line_in_volume(uint8_t vol, WM8731_Channel channel) {
    switch (channel) {
        case LEFT:
            wm8731_write_reg(WM8731_LEFT_LINE_IN, vol);
            break;

        case RIGHT:
            wm8731_write_reg(WM8731_RIGHT_LINE_IN, vol);
            break;

        case BOTH:
            wm8731_write_reg(WM8731_LEFT_LINE_IN,
                             vol | WM8731_LRINBOTH);
            break;
    }
}

void wm8731_mute_line_in(WM8731_Channel channel) {
    switch (channel) {
        case LEFT:
            wm8731_write_reg(WM8731_LEFT_LINE_IN,
                             WM8731_LINMUTE);
            break;

        case RIGHT:
            wm8731_write_reg(WM8731_RIGHT_LINE_IN,
                             WM8731_RINMUTE);
            break;

        case BOTH:
            wm8731_write_reg(WM8731_LEFT_LINE_IN,
                             WM8731_LINMUTE |
                             WM8731_LRINBOTH);
            break;
    }
}

// set the headphone output level to vol dB
// vol: ranging from -73 to 6
void wm8731_set_hp_volume(int8_t vol, WM8731_Channel channel) {
    uint16_t val = (uint16_t)(vol + 121);
    switch (channel) {
        case LEFT:
            wm8731_write_reg(WM8731_LEFT_HP_OUT,
                             val | WM8731_LZCEN);
            break;

        case RIGHT:
            wm8731_write_reg(WM8731_RIGHT_HP_OUT,
                             val | WM8731_RZCEN);
            break;

        case BOTH:
            wm8731_write_reg(WM8731_LEFT_HP_OUT,
                             val | WM8731_LZCEN |
                             WM8731_LRHPBOTH);
            break;
    }
}
