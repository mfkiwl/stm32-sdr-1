#include "uart.h"

volatile uint8_t rxcnt = 0;
volatile char uart_rxbuf[UART_BUFSIZE] = {0};
volatile UART_StateMachine uart_state;

void uart_init() {
    // configure PA2 (TX) and PA3 (RX)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;    
    GPIOA->MODER |= GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1;
    GPIOA->AFR[0] |= (7<<GPIO_AFRL_AFSEL2_Pos) |
                     (7<<GPIO_AFRL_AFSEL3_Pos);
    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3);
    GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED2_0 |
                      GPIO_OSPEEDR_OSPEED3_0;
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR2 | GPIO_PUPDR_PUPDR3);

    // configure USART2
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    // 115.2kBaud 22.8125
    USART2->BRR = 0x16D;
    USART2->CR1 |= USART_CR1_RXNEIE;
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE;
    NVIC_EnableIRQ(USART2_IRQn);
    USART2->CR1 |= USART_CR1_UE;
    // clear buffer
    for (uint8_t i = 0; i < UART_BUFSIZE; i++) uart_rxbuf[i] = 0;
    // set state machine to idle mode
    uart_state = IDLE;
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
