#include "i2s.h"

volatile int32_t i2s_receive_buffer0[I2S_BUFSIZE];
volatile int32_t i2s_receive_buffer1[I2S_BUFSIZE];
volatile int32_t i2s_transmit_buffer0[I2S_BUFSIZE];
volatile int32_t i2s_transmit_buffer1[I2S_BUFSIZE];

volatile int32_t* i2s_receive_buffer;
volatile int32_t* i2s_transmit_buffer;

volatile bool i2s_receive_buffer_full;
volatile bool i2s_transmit_buffer_empty;

// This is for testing purposes only
// The received data from the previous transfer is 
// immediately transmitted again without any signal processing
//volatile int32_t* i2s_transmit_buffer0 = i2s_receive_buffer0;
//volatile int32_t* i2s_transmit_buffer1 = i2s_receive_buffer1;

void i2s_init() {
    i2s_receive_buffer_full = false;
    i2s_transmit_buffer_empty = true;

    i2s_gpio_init();
    i2s_dma_init();

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
    // SPI2, Master transmit
    SPI2->I2SPR = SPI_I2SPR_MCKOE | SPI_I2SPR_ODD | 3;
    SPI2->I2SCFGR = SPI_I2SCFGR_I2SMOD |
                    SPI_I2SCFGR_I2SCFG_1 |
                    SPI_I2SCFGR_DATLEN_0;

    SPI2->CR2 |= SPI_CR2_TXDMAEN;


    // I2S2ext, Slave receive
    I2S2ext->I2SCFGR = SPI_I2SCFGR_I2SMOD |
                       SPI_I2SCFGR_I2SCFG_0 |
                       SPI_I2SCFGR_DATLEN_0;

    I2S2ext->CR2 |= SPI_CR2_RXDMAEN;

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

void i2s_dma_init() {
    // enable DMA1 clock
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    // I2S2 transmit stream: Stream 4 Channel 0
    // disable DMA Stream and wait for transfers to finish
    DMA1_Stream4->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream4->CR & DMA_SxCR_EN);
    // clear any interrupt flags
    DMA1->HIFCR = DMA_HIFCR_CTCIF4 |
                  DMA_HIFCR_CHTIF4 |
                  DMA_HIFCR_CTEIF4 |
                  DMA_HIFCR_CDMEIF4 |
                  DMA_HIFCR_CFEIF4;
    // configure
    DMA1_Stream4->PAR = (uint32_t)&SPI2->DR;
    DMA1_Stream4->M0AR = (uint32_t)i2s_transmit_buffer0;
    DMA1_Stream4->M1AR = (uint32_t)i2s_transmit_buffer1;
    DMA1_Stream4->NDTR = (I2S_BUFSIZE<<1);
    DMA1_Stream4->FCR = (0<<DMA_SxFCR_FEIE_Pos) |
                        (1<<DMA_SxFCR_DMDIS_Pos) |
                        (3<<DMA_SxFCR_FTH_Pos);
    DMA1_Stream4->CR = (0<<DMA_SxCR_CHSEL_Pos) |
                       (0<<DMA_SxCR_MBURST_Pos) |
                       (0<<DMA_SxCR_PBURST_Pos) |
                       (0<<DMA_SxCR_CT_Pos) |
                       (1<<DMA_SxCR_DBM_Pos) |
                       (0<<DMA_SxCR_PL_Pos) |
                       (0<<DMA_SxCR_PINCOS_Pos) |
                       (2<<DMA_SxCR_MSIZE_Pos) |
                       (1<<DMA_SxCR_PSIZE_Pos) |
                       (1<<DMA_SxCR_MINC_Pos) |
                       (0<<DMA_SxCR_PINC_Pos) |
                       (0<<DMA_SxCR_CIRC_Pos) |
                       (1<<DMA_SxCR_DIR_Pos) |
                       (0<<DMA_SxCR_PFCTRL_Pos) |
                       (1<<DMA_SxCR_TCIE_Pos) |
                       (0<<DMA_SxCR_HTIE_Pos) |
                       (0<<DMA_SxCR_TEIE_Pos) |
                       (0<<DMA_SxCR_DMEIE_Pos) |
                       (1<<DMA_SxCR_EN_Pos);

    NVIC_EnableIRQ(DMA1_Stream4_IRQn);

    // I2S2ext receive stream: Stream 3 Channel 3
    // disable DMA Stream and wait for transfers to finish
    DMA1_Stream3->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream3->CR & DMA_SxCR_EN);
    // clear interrupt flags
    DMA1->LIFCR = DMA_LIFCR_CTCIF3 |
                  DMA_LIFCR_CHTIF3 |
                  DMA_LIFCR_CTEIF3 |
                  DMA_LIFCR_CDMEIF3 |
                  DMA_LIFCR_CFEIF3;
    // configure
    DMA1_Stream3->PAR = (uint32_t)&I2S2ext->DR;
    DMA1_Stream3->M0AR = (uint32_t)i2s_receive_buffer0;
    DMA1_Stream3->M1AR = (uint32_t)i2s_receive_buffer1;
    DMA1_Stream3->NDTR = (I2S_BUFSIZE<<1);
    DMA1_Stream3->FCR = (0<<DMA_SxFCR_FEIE_Pos) |
                        (1<<DMA_SxFCR_DMDIS_Pos) |
                        (3<<DMA_SxFCR_FTH_Pos);
    DMA1_Stream3->CR = (3<<DMA_SxCR_CHSEL_Pos) |
                       (0<<DMA_SxCR_MBURST_Pos) |
                       (0<<DMA_SxCR_PBURST_Pos) |
                       (1<<DMA_SxCR_CT_Pos) |
                       (1<<DMA_SxCR_DBM_Pos) |
                       (0<<DMA_SxCR_PL_Pos) |
                       (0<<DMA_SxCR_PINCOS_Pos) |
                       (2<<DMA_SxCR_MSIZE_Pos) |
                       (1<<DMA_SxCR_PSIZE_Pos) |
                       (1<<DMA_SxCR_MINC_Pos) |
                       (0<<DMA_SxCR_PINC_Pos) |
                       (0<<DMA_SxCR_CIRC_Pos) |
                       (0<<DMA_SxCR_DIR_Pos) |
                       (0<<DMA_SxCR_PFCTRL_Pos) |
                       (1<<DMA_SxCR_TCIE_Pos) |
                       (0<<DMA_SxCR_HTIE_Pos) |
                       (0<<DMA_SxCR_TEIE_Pos) |
                       (0<<DMA_SxCR_DMEIE_Pos) |
                       (1<<DMA_SxCR_EN_Pos);

    NVIC_EnableIRQ(DMA1_Stream3_IRQn);
}

void DMA1_Stream4_IRQHandler() {
    // I2S2 master transmit handler
    if (DMA1->HISR & DMA_HISR_TCIF4) {
        //GPIOA->BSRR = GPIO_BSRR_BS5;
        // update the buffer pointer
        if (DMA1_Stream4->CR & DMA_SxCR_CT) {
            i2s_transmit_buffer = i2s_transmit_buffer0;
        } else {
            i2s_transmit_buffer = i2s_transmit_buffer1;
        }
        // let the main application know
        i2s_transmit_buffer_empty = true;
        // clear the interrupt flag
        DMA1->HIFCR = DMA_HIFCR_CTCIF4;
    }
}

void DMA1_Stream3_IRQHandler() {
    // I2S2ext slave receive handler
    if (DMA1->LISR & DMA_LISR_TCIF3) {
        //GPIOA->BSRR = GPIO_BSRR_BR5;
        // update the buffer pointer
        if (DMA1_Stream3->CR & DMA_SxCR_CT) {
            i2s_receive_buffer = i2s_receive_buffer0;
        } else {
            i2s_receive_buffer = i2s_receive_buffer1;
        }
        // let the main application know
        i2s_receive_buffer_full = true;
        // clear the interrupt flag
        DMA1->LIFCR = DMA_LIFCR_CTCIF3;
    }
}
