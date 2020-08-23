#include "ssb.h"

const uint8_t SSB_FilterStages = 4;
const q31_t SSB_FilterCoeffs[20] = {
    0x00114bd5, 0xffe4d726, 0x00114bd5, 0x7a5678d5, 0xc55f7377,
    0x40000000, 0x84bf6817, 0x40000000, 0x7bd21248, 0xc315d7d4,
    0x40000000, 0x82db6558, 0x3fffffff, 0x7d09eba7, 0xc13bcfde,
    0x40000000, 0x82754942, 0x3fffffff, 0x7db56ebc, 0xc04e2694,
};

void SSB_Demodulator_init(SSB_Demodulator* demod) {
    // filter initialization
    arm_biquad_cascade_df1_init_q31(&demod->Filter_I,
                                    SSB_FilterStages,
                                    (q31_t*)SSB_FilterCoeffs,
                                    demod->FilterState_I,
                                    1);

    arm_biquad_cascade_df1_init_q31(&demod->Filter_Q,
                                    SSB_FilterStages,
                                    (q31_t*)SSB_FilterCoeffs,
                                    demod->FilterState_Q,
                                    1);
    //demod->phase = 0;
    //demod->dphi = 0x03999999;
    demod->phase = 0.0f;
    demod->dphi = 0.215984494f;
    demod->sideband = USB;
}

// Weaver method SSB demodulation
void demod_ssb(SSB_Demodulator* demod) {
    float cos_f32, sin_f32;
    q31_t cos_q31, sin_q31;

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

    for (uint32_t i = 0; i < I2S_BUFSIZE2; i++) {
        /*
        arm_add_q31(&demod->phase, &demod->dphi, &demod->phase,1);
        demod->phase = (q31_t)((uint32_t)demod->phase % 0x7fffffff);
        cos_q31 = arm_cos_q31(demod->phase);
        sin_q31 = arm_sin_q31(demod->phase);
        */
        
        demod->phase = fmodf(demod->phase+demod->dphi, twopi);
        cos_f32 = arm_cos_f32(demod->phase);
        sin_f32 = arm_sin_f32(demod->phase);
        arm_float_to_q31(&cos_f32, &cos_q31, 1);
        arm_float_to_q31(&sin_f32, &sin_q31, 1);
        
        arm_mult_q31((outbuffer_I+i),
                     &cos_q31,
                     (outbuffer_I+i),
                     1);
        arm_mult_q31((outbuffer_Q+i),
                     &sin_q31,
                     (outbuffer_Q+i),
                     1);
    }

    arm_add_q31(outbuffer_I,
                outbuffer_Q,
                outbuffer,
                I2S_BUFSIZE2);

    swap_halfword_duplicate(
        (uint32_t*)outbuffer,
        (uint32_t*)i2s_transmit_buffer,
        I2S_BUFSIZE
    );
}


