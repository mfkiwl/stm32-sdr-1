#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stm32f4xx.h>

#include "uart.h"
#include "i2c.h"
#include "i2s.h"
#include "si5351.h"
#include "wm8731.h"

#include "am.h"
#include "ssb.h"

typedef enum {
    AS_IDLE,
    AS_DSP,
    AS_SETVFO,
    AS_SETVOL,
    AS_PARSE
} ApplicationStateMachine;

typedef enum {
    MS_CW,
    MS_LSB,
    MS_USB,
    MS_AM
} ModulationScheme;

typedef struct {
    uint32_t dial_frequency;
    uint32_t vfo_frequency;
    ModulationScheme modulation_scheme;
    int8_t headphone_volume;
} ApplicationSettings;

typedef struct {
    char name[10];
    char type[10];
    uint32_t value;
} Command;

void busy(uint32_t delay) {
    for (uint32_t i = 0; i < delay; i++) __asm("mov r0,r0");
}

void clock_config() {
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_VOS_1;
    FLASH->ACR |= (2<<FLASH_ACR_LATENCY_Pos);
    // PLL setup, use HSI
    // wait for HSI to stabilize
    while(!(RCC->CR & RCC_CR_HSIRDY));
    // configure PLL
    // PLL clock source HSI 16 MHz
    // M = 8 --> f_PLLin = 2 MHz
    // N = 168 --> f_VCOout = 336 MHz
    // P = 4 --> f_SYSCLK = 84 MHz
    // Q = 7 --> f_USBOTG = 48 MHz
    RCC->PLLCFGR = (7<<RCC_PLLCFGR_PLLQ_Pos) |
                   RCC_PLLCFGR_PLLSRC_HSI |
                   (1<<RCC_PLLCFGR_PLLP_Pos) |
                   (168<<RCC_PLLCFGR_PLLN_Pos) |
                   (8<<RCC_PLLCFGR_PLLM_Pos);
    // Prescaler configuration
    // AHB prescaler HPRE = 1 
    // APB1 prescaler  PPRE1 = 2
    // APB2 prescaler PPRE2 = 1
    RCC->CFGR = RCC_CFGR_HPRE_DIV1 |
                RCC_CFGR_PPRE2_DIV1 |
                RCC_CFGR_PPRE1_DIV2;
    // turn on PLL
    RCC->CR |= RCC_CR_PLLON;
    // wait for the PLL to stabilize
    while (!(RCC->CR & RCC_CR_PLLRDY));
    // switch system clock source to PLL output
    RCC->CFGR |= RCC_CFGR_SW_PLL;
}

void nucleo_led_init() {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    // set up GPIO A 5 to blink the LED yay
    GPIOA->MODER |= GPIO_MODER_MODER5_0;
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT5;
    GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED5_0;
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR5;
    GPIOA->BSRR |= GPIO_BSRR_BR5;
}

void nucleo_led_on() {
    GPIOA->BSRR |= GPIO_BSRR_BS5;
}

void nucleo_led_off() {
    GPIOA->BSRR |= GPIO_BSRR_BR5;
}

void tayloe_gpio_init() {
    // set up ~TXEN and ~RXEN
    // PA8 = ~TXEN
    // PA9 = ~RXEN
    GPIOA->MODER |= GPIO_MODER_MODER9_0 |
                    GPIO_MODER_MODER8_0;
    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT9 |
                       GPIO_OTYPER_OT8);
    GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED9_0 |
                      GPIO_OSPEEDR_OSPEED8_0;
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR9 |
                      GPIO_PUPDR_PUPDR8);
    // enable RX 
    // both pins need to be set low because multiplexer does
    // not allow independent control of both halves
    GPIOA->BSRR |= GPIO_BSRR_BR9 | GPIO_BSRR_BR8;
    //GPIOA->BSRR |= GPIO_BSRR_BS9 | GPIO_BSRR_BS8;
}

void parse_command(char* buffer, Command* cmd) {
    char *name, *type, *value;
    name = strtok(buffer, " ");
    type = strtok(NULL, " ");
    value = strtok(NULL, " ");
    strncpy(cmd->name,name,10);
    strncpy(cmd->type,type,10);
    cmd->value = atoi(value);
}

int main(void) {
    // App state
    ApplicationStateMachine app_state = AS_IDLE;
    // App settings
    ApplicationSettings app_settings;
    app_settings.dial_frequency = 7069350;
    app_settings.vfo_frequency = app_settings.dial_frequency;
    app_settings.modulation_scheme = MS_LSB;
    app_settings.headphone_volume = -16;
    // For parsing uart interface commands
    Command cmd;
    // Demodulators
    AM_Demodulator am;
    SSB_Demodulator ssb;
    SSB_Demodulator cw;

    SystemInit();
    
    clock_config();
    nucleo_led_init();
    tayloe_gpio_init();

    AM_Demodulator_init(&am);
    SSB_Demodulator_init(&ssb, LSB);
    SSB_Demodulator_init(&cw, USB);

    uart_init();

    i2c_init();
    si5351_init();
    si5351_set_frequency((float)(app_settings.vfo_frequency<<2));
    wm8731_init();
    wm8731_set_hp_volume(app_settings.headphone_volume, BOTH);
    i2s_init();

    while (1) {
        switch (app_state) {
            case AS_IDLE:
                nucleo_led_off();
                if (uart_state == END) {
                    app_state = AS_PARSE;
                } else if (i2s_receive_buffer_full == true) {
                    i2s_receive_buffer_full = false;
                    app_state = AS_DSP;
                } else {
                    app_state = AS_IDLE;
                }
                break;

            case AS_DSP:
                nucleo_led_on();
                switch (app_settings.modulation_scheme) {
                    case MS_CW:
                        demod_ssb(&cw);
                        break;

                    case MS_LSB:
                        set_ssb_sideband(&ssb, LSB);
                        demod_ssb(&ssb);
                        break;

                    case MS_USB:
                        set_ssb_sideband(&ssb, USB);
                        demod_ssb(&ssb);
                        break;

                    case MS_AM:
                        demod_am(&am);
                        break;
                }
                app_state = AS_IDLE;
                break;

            case AS_SETVFO:
                app_settings.vfo_frequency = app_settings.dial_frequency;
                switch (app_settings.modulation_scheme) {
                    case MS_CW:
                        app_settings.vfo_frequency -= 4250;
                        break;

                    case MS_LSB:
                        app_settings.vfo_frequency -= 6650;
                        break;

                    case MS_USB:
                        app_settings.vfo_frequency -= 3350;
                        break;

                    case MS_AM:
                        app_settings.vfo_frequency = app_settings.dial_frequency;
                        break;
                }
 
                wm8731_mute_line_in(BOTH);
                si5351_set_frequency((float)(app_settings.vfo_frequency<<2));
                wm8731_set_line_in_volume(55, BOTH);
                app_state = AS_IDLE;
                break;

            case AS_SETVOL:
                wm8731_set_hp_volume(app_settings.headphone_volume, BOTH);
                app_state = AS_IDLE;
                break;

            case AS_PARSE:
                parse_command((char*)uart_rxbuf, &cmd);
                memset((char*)uart_rxbuf,0,UART_BUFSIZE);
                uart_state = IDLE;
                if (strncmp(cmd.name, "freq", 4) == 0) {
                    if (strncmp(cmd.type, "am", 2) == 0) {
                        app_settings.modulation_scheme = MS_AM;
                    } else if (strncmp(cmd.type, "cw", 2) == 0) {
                        app_settings.modulation_scheme = MS_CW;
                    } else if (strncmp(cmd.type, "lsb", 3) == 0) {
                        app_settings.modulation_scheme = MS_LSB;
                    } else if (strncmp(cmd.type, "usb", 3) == 0) {
                        app_settings.modulation_scheme = MS_USB;
                    }
                    app_settings.dial_frequency = cmd.value;
                    app_state = AS_SETVFO;
                } else if (strncmp(cmd.name, "hp", 2) == 0) {
                    if (strncmp(cmd.type, "vol", 3) == 0) {
                        if ((cmd.value >= 0) && (cmd.value <= 100)) {
                            app_settings.headphone_volume = -44 + (int8_t)(cmd.value>>1);
                        }
                    }
                    app_state = AS_SETVOL;
                } else {
                    app_state = AS_IDLE;
                }
                break;
        }
    }
}
