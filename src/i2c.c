#include "i2c.h"

void i2c_init() {
    // enable GPIOB clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    // PB8 (SCL) and PB9 (SDA)
    GPIOB->MODER |= GPIO_MODER_MODE9_1 | GPIO_MODER_MODE8_1;
    GPIOB->OTYPER |= GPIO_OTYPER_OT9 | GPIO_OTYPER_OT8;
    GPIOB->PUPDR |= GPIO_PUPDR_PUPDR9_0 | GPIO_PUPDR_PUPDR8_0;
    GPIOB->AFR[1] |= (4<<GPIO_AFRH_AFSEL9_Pos) | (4<<GPIO_AFRH_AFSEL8_Pos);
    
    // Enable I2C1 clock
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->CR2 |= (16<<I2C_CR2_FREQ_Pos);
    I2C1->CCR = 80;
    I2C1->TRISE = 17;
    I2C1->CR1 |= I2C_CR1_PE;
}

void i2c_write_buf(uint8_t address, uint8_t* buf, uint32_t length) {
    i2c_start(address);
    for (uint32_t i = 0; i < length; i++) {
        i2c_write(buf[i]);
    }
    i2c_stop();
}

void i2c_start(uint8_t address) {
    I2C1->CR1 |= I2C_CR1_START;
    while ( !(I2C1->SR1 & I2C_SR1_SB) );
    I2C1->DR = address;    
    while ( !(I2C1->SR1 & I2C_SR1_ADDR) );
    I2C1->SR2;
}

void i2c_stop() {
    while ( !(I2C1->SR1 & I2C_SR1_TXE) );
    I2C1->CR1 |= I2C_CR1_STOP;
}

void i2c_write(uint8_t data) {
    while ( !(I2C1->SR1 & I2C_SR1_TXE) );
    I2C1->DR = data;
}

uint8_t i2c_read(uint8_t ack) {
    if (ack) {
        I2C1->CR1 |= I2C_CR1_ACK;
    }
    while ( !(I2C1->SR1 & I2C_SR1_RXNE) );
    return I2C1->DR;
}
