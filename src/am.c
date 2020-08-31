#include "am.h"

const uint8_t AM_FilterStages = 5;
const q31_t AM_FilterCoeffs[25] = {
    0x0005d059, 0x0004594e, 0x0005d059, 0x70284772, 0xcdd80953,
    0x40000000, 0xbc2d7af7, 0x3fffffff, 0x6dd6c6b7, 0xc9a54da8,
    0x40000000, 0xa367831a, 0x40000000, 0x6b6fdd8d, 0xc527aa23,
    0x40000000, 0x9c4eaf09, 0x40000000, 0x6a1edb5c, 0xc24487c2,
    0x40000000, 0x9a1d9c14, 0x40000000, 0x69eef51f, 0xc09f59f3,
};

void AM_Demodulator_init(AM_Demodulator* demod) {
    // filter initialization
    arm_biquad_cascade_df1_init_q31(&demod->Filter_I,
                                    AM_FilterStages,
                                    (q31_t*)AM_FilterCoeffs,
                                    demod->FilterState_I,
                                    1);

    arm_biquad_cascade_df1_init_q31(&demod->Filter_Q,
                                    AM_FilterStages,
                                    (q31_t*)AM_FilterCoeffs,
                                    demod->FilterState_Q,
                                    1);

    demod->lo_phase = 0;
    demod->lo_dphi = 0x10000000;
}

// simple AM demodulation
void demod_am(AM_Demodulator* demod) {
    q31_t cos_q31;

    swap_halfword_deinterleave(
        (uint32_t*)i2s_receive_buffer,
        (uint32_t*)inbuffer_I,
        (uint32_t*)inbuffer_Q,
        I2S_BUFSIZE
    );

    for (uint32_t i = 0; i < I2S_BUFSIZE2; i++) {
        demod->lo_phase = (demod->lo_phase+demod->lo_dphi)&0x7fffffff;
        cos_q31 = arm_cos_q31((q31_t)demod->lo_phase);

        arm_mult_q31((inbuffer_I+i),
                     &cos_q31,
                     (inbuffer_I+i),
                     1);
        arm_mult_q31((inbuffer_Q+i),
                     &cos_q31,
                     (inbuffer_Q+i),
                     1);
    }

    arm_biquad_cascade_df1_q31(
        &demod->Filter_I,
        inbuffer_I,
        outbuffer_I,
        I2S_BUFSIZE2
    );

    arm_biquad_cascade_df1_q31(
        &demod->Filter_Q,
        inbuffer_Q,
        outbuffer_Q,
        I2S_BUFSIZE2
    );

    interleave((uint32_t*)outbuffer_I,
               (uint32_t*)outbuffer_Q,
               (uint32_t*)i2s_receive_buffer,
               I2S_BUFSIZE);

    arm_cmplx_mag_q31((q31_t*)i2s_receive_buffer,
                      outbuffer,
                      I2S_BUFSIZE2);

    swap_halfword_duplicate(
        (uint32_t*)outbuffer,
        (uint32_t*)i2s_transmit_buffer,
        I2S_BUFSIZE
    );
}
