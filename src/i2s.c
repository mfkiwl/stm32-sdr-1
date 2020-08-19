#include "i2s.h"

volatile uint8_t i2s_tx_cnt, i2s_rx_cnt;
volatile int32_t left_buffer, right_buffer;

void i2s_init() {
    i2s_tx_cnt = 0;
    i2s_rx_cnt = 0;
    left_buffer = 0xaaaaaa00;
    right_buffer = 0xaaaaaa00;

    i2s_gpio_init();

    /* I2S2 setup
        I2S mode
        Output master clock
        fs = 48 kHz
        24 bit data
        32 bit word length
    */
 
    // configure I2S PLL
    RCC->CR &= ~RCC_CR_PLLI2SON;
    RCC->PLLI2SCFGR = (258<<RCC_PLLI2SCFGR_PLLI2SN_Pos) |
                      (6<<RCC_PLLI2SCFGR_PLLI2SR_Pos);
    // enable I2S PLL
    RCC->CR |= RCC_CR_PLLI2SON;
    // wait for I2S PLL to become ready
    while(!(RCC->CR & RCC_CR_PLLI2SRDY));

    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    //SPI2->CR2 = SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN;
    // SPI2, Master transmit
    SPI2->I2SPR = SPI_I2SPR_MCKOE | SPI_I2SPR_ODD | 3;
    SPI2->I2SCFGR = SPI_I2SCFGR_I2SMOD |
                    SPI_I2SCFGR_I2SCFG_1 |
                    SPI_I2SCFGR_DATLEN_0;

    SPI2->CR2 |= SPI_CR2_TXEIE; // | SPI_CR2_RXNEIE;

    // I2S2ext, Slave receive
    I2S2ext->I2SCFGR = SPI_I2SCFGR_I2SMOD |
                       SPI_I2SCFGR_I2SCFG_0 |
                       SPI_I2SCFGR_DATLEN_0;
    I2S2ext->CR2 |= SPI_CR2_RXNEIE;


    NVIC_EnableIRQ(SPI2_IRQn);

    // enable I2S2
    I2S2ext->I2SCFGR |= SPI_I2SCFGR_I2SE;
    SPI2->I2SCFGR |= SPI_I2SCFGR_I2SE;
}

void i2s_gpio_init() {
    /* GPIO mapping:
        LRCLK:  PB12    (I2S2_WS)
        BCLK:   PB13    (I2S2_CK)
        ADCDAT: PB14    (I2S2ext_SD)
        DACDAT: PB15    (I2S2_SD)
        MCLK:   PC6     (I2S2_MCK)

        I2S2 = Master transmit
        I2S2ext = Slave receive
    */
    // GPIOB setup
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;    
    GPIOB->MODER |= (2<<GPIO_MODER_MODER15_Pos) | 
                    (2<<GPIO_MODER_MODER14_Pos) |
                    (2<<GPIO_MODER_MODER13_Pos) |
                    (2<<GPIO_MODER_MODER12_Pos);
    GPIOB->OTYPER &= ~(GPIO_OTYPER_OT15 | 
                       GPIO_OTYPER_OT14 |
                       GPIO_OTYPER_OT13 |
                       GPIO_OTYPER_OT12);
    GPIOB->OSPEEDR |= (2<<GPIO_OSPEEDR_OSPEED15_Pos) |
                      (2<<GPIO_OSPEEDR_OSPEED14_Pos) |
                      (2<<GPIO_OSPEEDR_OSPEED13_Pos) |
                      (2<<GPIO_OSPEEDR_OSPEED12_Pos);
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR15 |
                      GPIO_PUPDR_PUPDR14 |
                      GPIO_PUPDR_PUPDR13 |
                      GPIO_PUPDR_PUPDR12);
    GPIOB->AFR[1] |= (5<<GPIO_AFRH_AFSEL15_Pos) |
                     (6<<GPIO_AFRH_AFSEL14_Pos) |
                     (5<<GPIO_AFRH_AFSEL13_Pos) |
                     (5<<GPIO_AFRH_AFSEL12_Pos);

    // GPIOC setup
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;    
    GPIOC->MODER |= (2<<GPIO_MODER_MODER6_Pos);
    GPIOC->OTYPER &= ~GPIO_OTYPER_OT6;
    GPIOC->OSPEEDR |= (2<<GPIO_OSPEEDR_OSPEED6_Pos);
    GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR6;
    GPIOC->AFR[0] |= (5<<GPIO_AFRL_AFSEL6_Pos);
}

void SPI2_IRQHandler() {
    //GPIOA->ODR ^= GPIO_ODR_OD5;
    if (SPI2->SR & SPI_SR_TXE) {
        GPIOA->BSRR |= GPIO_BSRR_BR5;
        if (SPI2->SR & SPI_SR_CHSIDE) {
            if (!i2s_tx_cnt) {
                SPI2->DR = (right_buffer>>16);
                i2s_tx_cnt = 1;
            } else {
                SPI2->DR = (right_buffer);
                i2s_tx_cnt = 0;
            }
        } else {
            if (!i2s_tx_cnt) {
                SPI2->DR = (left_buffer>>16);
                i2s_tx_cnt = 1;
            } else {
                SPI2->DR = (left_buffer);
                i2s_tx_cnt = 0;
            }
        }
    } else if (I2S2ext->SR & SPI_SR_RXNE) {
        GPIOA->BSRR |= GPIO_BSRR_BS5;
        if (I2S2ext->SR & SPI_SR_CHSIDE) {
            if (!i2s_rx_cnt) {
                right_buffer = (I2S2ext->DR<<16);
                i2s_rx_cnt = 1;
            } else {
                right_buffer |= I2S2ext->DR;
                i2s_rx_cnt = 0;
            }
        } else {
            if (!i2s_rx_cnt) {
                left_buffer = (I2S2ext->DR<<16);
                i2s_rx_cnt = 1;
            } else {
                left_buffer |= I2S2ext->DR;
                i2s_rx_cnt = 0;
            }
        }
    }
}
