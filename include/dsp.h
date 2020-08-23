#ifndef DSP_H
#define DSP_H

#include <stdint.h>
#include <stm32f4xx.h>
#include <arm_math.h>

#include "i2s.h"

#define I2S_BUFSIZE2 (I2S_BUFSIZE/2)

extern const float twopi;

extern q31_t outbuffer[I2S_BUFSIZE2];
extern q31_t inbuffer_I[I2S_BUFSIZE2];
extern q31_t inbuffer_Q[I2S_BUFSIZE2];
extern q31_t outbuffer_I[I2S_BUFSIZE2];
extern q31_t outbuffer_Q[I2S_BUFSIZE2];

void swap_halfword(uint32_t *src, uint32_t *dst, uint32_t blocksize);
void swap_halfword_deinterleave(uint32_t *src, uint32_t *A, uint32_t *B, uint32_t blocksize);
void interleave(uint32_t *I, uint32_t *Q, uint32_t *dst, uint32_t blocksize);
void mono_to_stereo(uint32_t *src, uint32_t *dst, uint32_t blocksize);
void swap_halfword_duplicate(uint32_t *src, uint32_t *dst, uint32_t blocksize);

#endif
