#include <stdint.h>
#include <stdlib.h>
#include <stm32f4xx.h>

#include "i2c.h"
#include "si5351.h"

#define UART_BUFSIZE 10
volatile uint8_t rxcnt = 0;
volatile char uart_rxbuf[UART_BUFSIZE] = {0};

typedef enum {
    IDLE,
    RECEIVE,
    END
} UART_StateMachine;

volatile UART_StateMachine uart_state;

void busy(uint32_t delay) {
    for (uint32_t i = 0; i < delay; i++) __asm("mov r0,r0");
}

void uart_init() {
    // configure PA2 (TX) and PA3 (RX)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;    
    GPIOA->MODER |= GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1;
    GPIOA->AFR[0] |= (7<<GPIO_AFRL_AFSEL2_Pos) | (7<<GPIO_AFRL_AFSEL3_Pos);
    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3);
    GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED2_0 | GPIO_OSPEEDR_OSPEED3_0;
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR2 | GPIO_PUPDR_PUPDR3);

    // configure USART2
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    USART2->BRR = 0x8B;
    USART2->CR1 |= USART_CR1_RXNEIE;
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE;
    NVIC_EnableIRQ(USART2_IRQn);
    USART2->CR1 |= USART_CR1_UE;
}

void uart_putc(char c) {
    while(!(USART2->SR & USART_SR_TXE));
    USART2->DR = c;
    while(!(USART2->SR & USART_SR_TC));
}

void uart_puts(char* s) {
    while (*s) {
        while (!(USART2->SR & USART_SR_TXE));
        USART2->DR = *s++;
    }
    while (!(USART2->SR & USART_SR_TC));
}

void USART2_IRQHandler() {
    if (USART2->SR & USART_SR_RXNE) {
        // we received something
        char rx = USART2->DR;

        switch (uart_state) {
            case IDLE:
                if (rx == '#') {
                    rxcnt = 0;
                    uart_state = RECEIVE;
                } else { 
                    uart_state = IDLE;
                }
                break;

            case RECEIVE:
                if (rxcnt < UART_BUFSIZE-1) {
                    if (rx == '\n') {
                        uart_rxbuf[rxcnt] = 0;
                        uart_state = END;
                    } else {
                        uart_rxbuf[rxcnt++] = rx;
                        uart_state = RECEIVE; 
                    }
                } else {
                    uart_rxbuf[rxcnt] = 0;
                    uart_state = IDLE;
                }
                break;

            case END:
                break;

            default:
                uart_state = IDLE;
                break;
        }
    }
}

int main(void) {
    SystemInit();
    /* 
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_VOS_1;

    // PLL setup, use HSI
    RCC->PLLCFGR = (7<<RCC_PLLCFGR_PLLQ_Pos) | RCC_PLLCFGR_PLLSRC_HSI | RCC_PLLCFGR_PLLP_0 | (336<<RCC_PLLCFGR_PLLN_Pos) | (16<<RCC_PLLCFGR_PLLM_Pos);
    // turn on HSI and PLL
    RCC->CR |= RCC_CR_HSION | RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));
    RCC->CFGR = RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_PPRE1_DIV2;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    // set up GPIO A 5 to blink the LED yay
    GPIOA->MODER |= GPIO_MODER_MODER5_0;
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT5;
    GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED5_0;
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR5;

    GPIOA->BSRR |= GPIO_BSRR_BS5;
    
    for (uint8_t i = 0; i < UART_BUFSIZE; i++) uart_rxbuf[i] = 0;
    uart_state = IDLE;

    uart_init();
    uart_puts("Hello World!\r\n");

    i2c_init();
    si5351_init();

    while(1) {
        while (uart_state != END) {
            GPIOA->BSRR |= GPIO_BSRR_BS5;
            busy(0x000fffff);
            GPIOA->BSRR |= GPIO_BSRR_BR5;
            busy(0x000fffff);
        }
        uart_puts(uart_rxbuf);
        uint32_t freq = atoi(uart_rxbuf);
        si5351_set_frequency(freq);
        for (uint8_t i = 0; i < UART_BUFSIZE; i++) uart_rxbuf[i] = 0;
        uart_state = IDLE;
    }
}
