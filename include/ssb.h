#ifndef SSB_H
#define SSB_H

#include "dsp.h"

typedef enum {
    LSB,
    USB
} SSB_Sideband;

typedef struct {
    arm_biquad_casd_df1_inst_q31 Filter_I;
    arm_biquad_casd_df1_inst_q31 Filter_Q;
    q31_t FilterState_I[40];
    q31_t FilterState_Q[40];
    uint32_t lo_phase;
    uint32_t lo_dphi;
    SSB_Sideband sideband;
} SSB_Demodulator;

void SSB_Demodulator_init(SSB_Demodulator* demod, SSB_Sideband sideband);
void set_ssb_sideband(SSB_Demodulator* demod, SSB_Sideband sideband);
void demod_ssb(SSB_Demodulator* demod);

#endif
