#ifndef I2S_H
#define I2S_H

#include <stdint.h>
#include <stdbool.h>
#include <stm32f4xx.h>

#define I2S_BUFSIZE 1024

extern volatile int32_t* i2s_receive_buffer;
extern volatile int32_t* i2s_transmit_buffer;
extern volatile bool i2s_buffer_full;

void i2s_init();
void i2s_gpio_init();
void i2s_dma_init();

#endif
