#include <stdint.h>
#include <stm32f4xx.h>

#include "i2c.h"
#include "si5351.h"

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
        char c = USART2->DR;
        uart_putc(c);
    }
}

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
    si5351_write_oms05_config(MS1, oms);
    si5351_write_reg(CLK1_PHOFF, 86);

    si5351_write_reg(XTAL_LOAD_CAPACITANCE, XTAL_CL1 | XTAL_CL0 | 0x12);
    si5351_write_reg(CLK0_CONTROL, 0x0C);
    si5351_write_reg(CLK1_CONTROL, 0x0C);
    si5351_write_reg(PLL_RESET, 0xA0);
    si5351_write_reg(OUTPUT_ENABLE_CONTROL, ~(CLK1_OEB | CLK0_OEB));
}

int main(void) {
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

    uart_init();
    uart_puts("Hello World!\r\n");

    i2c_init();
    si5351_init();


    while(1) {
        GPIOA->BSRR |= GPIO_BSRR_BS5;
        busy(0x000fffff);
        GPIOA->BSRR |= GPIO_BSRR_BR5;
        busy(0x000fffff);
    }
}
