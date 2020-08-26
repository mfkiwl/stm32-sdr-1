#include "ssb.h"

// band pass filter
// bandwitdth: 2700 Hz
// center frequency: 5000 Hz
const uint8_t SSB_FilterStages = 10;
const q31_t SSB_FilterCoeffs[50] = {
    0x0002d37f,0x00005d3f,0x0002d37f,0x60211ba4,0xc4afa1be,
    0x40000000,0x8292b1bc,0x3fffffff,0x66af2324,0xc4323771,
    0x40000000,0xbc4b4372,0x40000000,0x5adfe0e3,0xc383bcd9,
    0x40000000,0x89407801,0x3fffffff,0x6c186771,0xc29fb9a6,
    0x40000000,0xb0810a9b,0x40000000,0x57df396b,0xc1f2d7c2,
    0x40000000,0x8c0e142c,0x40000000,0x6f4a78e0,0xc14e3339,
    0x40000000,0xad15d976,0x3fffffff,0x56838100,0xc0decca8,
    0x40000000,0x8d1ffd30,0x40000000,0x70d887c2,0xc08dab99,
    0x40000000,0xac0270d0,0x40000000,0x561ece37,0xc03d5cf4,
    0x40000000,0x8d7e6d36,0x40000000,0x718753cf,0xc0263b35,
};

// band pass filter,
// bandwidth: 200 Hz
// center frequency: 750 Hz
const uint8_t CW_FilterStages = 9;
const q31_t CW_FilterCoeffs[45] = {
    0x00002a3c,0x00000000,0xffffd5c4,0x6544f3e0,0xc05d1ca9,
    0x40000000,0x9d3f0323,0x3fffffff,0x64d8f391,0xc04d44a8,
    0x40000000,0x97e0e7fd,0x3fffffff,0x65c92d04,0xc04c11bf,
    0x40000000,0x9c0e627d,0x40000000,0x649dfffa,0xc02e0031,
    0x40000000,0x98e92184,0x40000000,0x66324c23,0xc02ccdd9,
    0x40000000,0x9bc1f878,0x40000000,0x6485ea63,0xc0154e46,
    0x40000000,0x992e927a,0x3fffffff,0x666ead9a,0xc014a302,
    0x40000000,0x9baac11d,0x40000000,0x6480dd5f,0xc005f1d8,
    0x40000000,0x9943ef11,0x3fffffff,0x668aa188,0xc005bed9,
};

// Tuning offset is 3350 Hz @ fs = 48kHz
const uint32_t dphi_usb = 0x08eeeeee;
// Tuning offset is 6650 Hz @ fs = 48kHz
const uint32_t dphi_lsb = 0x11bbbbbb;
// Tuning offset is 4250 Hz @ fs = 48kHz
const uint32_t dphi_cw = 0x0b555555;

void SSB_Demodulator_init(SSB_Demodulator* demod, SSB_Sideband sideband) {
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

    demod->lo_phase = 0;
    set_ssb_sideband(demod, sideband);
}

void CW_Demodulator_init(SSB_Demodulator* demod, SSB_Sideband sideband) {
    // filter initialization
    arm_biquad_cascade_df1_init_q31(&demod->Filter_I,
                                    CW_FilterStages,
                                    (q31_t*)CW_FilterCoeffs,
                                    demod->FilterState_I,
                                    1);

    arm_biquad_cascade_df1_init_q31(&demod->Filter_Q,
                                    CW_FilterStages,
                                    (q31_t*)CW_FilterCoeffs,
                                    demod->FilterState_Q,
                                    1);



    demod->lo_phase = 0;
    demod->lo_dphi = dphi_cw;
    demod->sideband = sideband;
}

void set_ssb_sideband(SSB_Demodulator* demod, SSB_Sideband sideband) {
    demod->sideband = sideband;
    switch (demod->sideband) {
        case LSB:
            demod->lo_dphi = dphi_lsb;
            break;
        case USB:
            demod->lo_dphi = dphi_usb;
            break;
    }   
}

// Weaver method SSB demodulation
void demod_ssb(SSB_Demodulator* demod) {
    q31_t cos_q31, sin_q31;

    swap_halfword_deinterleave(
        (uint32_t*)i2s_receive_buffer,
        (uint32_t*)inbuffer_I,
        (uint32_t*)inbuffer_Q,
        I2S_BUFSIZE
    );

    //arm_biquad_cascade_df1_fast_q31(
    arm_biquad_cascade_df1_q31(
        &demod->Filter_I,
        inbuffer_I,
        outbuffer_I,
        I2S_BUFSIZE2
    );

    //arm_biquad_cascade_df1_fast_q31(
    arm_biquad_cascade_df1_q31(
        &demod->Filter_Q,
        inbuffer_Q,
        outbuffer_Q,
        I2S_BUFSIZE2
    );

    for (uint32_t i = 0; i < I2S_BUFSIZE2; i++) {
        demod->lo_phase = (demod->lo_phase+demod->lo_dphi)%0x80000000; 
        cos_q31 = arm_cos_q31((q31_t)demod->lo_phase);
        sin_q31 = arm_sin_q31((q31_t)demod->lo_phase);
        
        arm_mult_q31((outbuffer_I+i),
                     &cos_q31,
                     (outbuffer_I+i),
                     1);
        arm_mult_q31((outbuffer_Q+i),
                     &sin_q31,
                     (outbuffer_Q+i),
                     1);
    }
    
    arm_sub_q31(outbuffer_I,
                outbuffer_Q,
                outbuffer,
                I2S_BUFSIZE2);

    swap_halfword_duplicate(
        (uint32_t*)outbuffer,
        (uint32_t*)i2s_transmit_buffer,
        I2S_BUFSIZE
    );
}
