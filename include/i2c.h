#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stm32f4xx.h>

void i2c_init();
void i2c_start(uint8_t address);
void i2c_stop();
void i2c_write(uint8_t data);
uint8_t i2c_read(uint8_t ack);
void i2c_write_buf(uint8_t address, uint8_t* buf, uint32_t length);

#endif
