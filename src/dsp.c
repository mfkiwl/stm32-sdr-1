#include "dsp.h"

const float twopi = 6.283185307f;

q31_t outbuffer[I2S_BUFSIZE2];
q31_t inbuffer_I[I2S_BUFSIZE2];
q31_t inbuffer_Q[I2S_BUFSIZE2];
q31_t outbuffer_I[I2S_BUFSIZE2];
q31_t outbuffer_Q[I2S_BUFSIZE2];

void swap_halfword(uint32_t *src, uint32_t *dst, uint32_t blocksize) {
    for (uint32_t i = 0; i < blocksize; i++) {
        dst[i] = (src[i]<<16) | (src[i]>>16);
    }
}

// deinterleave two signals from src into A and B, and perform
// endianess change at the same time
void swap_halfword_deinterleave(uint32_t *src, uint32_t *A, uint32_t *B, uint32_t blocksize) {
    uint32_t j;
    for (uint32_t i = 0; i < blocksize-1; i+=2) {
        j = (i>>1);
        A[j] = (src[i]<<16) | (src[i]>>16);
        B[j] = (src[i+1]<<16) | (src[i+1]>>16);
    }
}

void interleave(uint32_t *I, uint32_t *Q, uint32_t *dst, uint32_t blocksize) {
    uint32_t j;
    for (uint32_t i = 0; i < blocksize; i++) {
        j = (i>>1);
        if ((i % 2) == 0) {
            dst[i] = I[j];
        } else {
            dst[i] = Q[j];
        }
    }
}

void mono_to_stereo(uint32_t *src, uint32_t *dst, uint32_t blocksize) {
    uint32_t j;
    for (uint32_t i = 0; i < blocksize-1; i+=2) {
        j = (i>>1);
        dst[i] = src[j];
        dst[i+1] = src[j];
    }
}

void swap_halfword_duplicate(uint32_t *src, uint32_t *dst, uint32_t blocksize) {
    uint32_t j;
    for (uint32_t i = 0; i < blocksize-1; i+=2) {
        j = (i>>1);
        dst[i] = (src[j]<<16) | (src[j]>>16);
        dst[i+1] = (src[j]<<16) | (src[j]>>16);
    }
}
