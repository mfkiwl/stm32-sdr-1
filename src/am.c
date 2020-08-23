#include "am.h"

const uint8_t AM_FilterStages = 4;
const q31_t AM_FilterCoeffs[20] = {
    0x00258df2, 0x000cd00f, 0x00258df2, 0x6c6fdf05, 0xd08458c3,
    0x40000000, 0xae6ec754, 0x40000000, 0x6ad0c0c3, 0xc9913a74,
    0x40000000, 0x9deb270a, 0x40000000, 0x69a86baf, 0xc3daec9d,
    0x40000000, 0x9a21b434, 0x40000000, 0x69a7fe3f, 0xc0f5bd3b,
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
}

// simple AM demodulation
void demod_am(AM_Demodulator* demod) {
    swap_halfword_deinterleave(
        (uint32_t*)i2s_receive_buffer,
        (uint32_t*)inbuffer_I,
        (uint32_t*)inbuffer_Q,
        I2S_BUFSIZE
    );

    arm_biquad_cascade_df1_fast_q31(
        &demod->Filter_I,
        inbuffer_I,
        outbuffer_I,
        I2S_BUFSIZE2
    );

    arm_biquad_cascade_df1_fast_q31(
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
