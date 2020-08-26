#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stm32f4xx.h>

#define UART_BUFSIZE 32

typedef enum {
    IDLE,
    RECEIVE,
    END
} UART_StateMachine;

extern volatile uint8_t rxcnt;
extern volatile char uart_rxbuf[UART_BUFSIZE];
extern volatile UART_StateMachine uart_state;

void uart_init();
void uart_putc(char c);
void uart_puts(char* s);
void USART2_IRQHandler();

#endif
