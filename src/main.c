#include <stdint.h>
#include <stdlib.h>
#include <stm32f4xx.h>

#include "uart.h"
#include "i2c.h"
#include "i2s.h"
#include "si5351.h"
#include "wm8731.h"

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
}

int main(void) {
    SystemInit();
    
    clock_config();
    nucleo_led_init();
    tayloe_gpio_init();

    uart_init();
    uart_puts("Hello World!\r\n");

    i2c_init();
    si5351_init();
    si5351_set_frequency(28000000);
    wm8731_init();
    i2s_init();

    while (1) {
        while (uart_state != END) {
        /*
            nucleo_led_on();
            busy(0x000fffff);
            nucleo_led_off();
            busy(0x000fffff);
        */
        }
        
        uint32_t freq = atoi((const char*)uart_rxbuf);
        si5351_set_frequency(freq*4.0f);
        for (uint8_t i = 0; i < UART_BUFSIZE; i++) uart_rxbuf[i] = 0;
        uart_state = IDLE;
    }
}
