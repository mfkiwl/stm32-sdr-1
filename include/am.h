#ifndef AM_H
#define AM_H

#include "dsp.h"

typedef struct {
    arm_biquad_casd_df1_inst_q31 Filter_I;
    arm_biquad_casd_df1_inst_q31 Filter_Q;
    q31_t FilterState_I[40];
    q31_t FilterState_Q[40];
    uint32_t lo_phase;
    uint32_t lo_dphi;
} AM_Demodulator;

void AM_Demodulator_init(AM_Demodulator* demod);
void demod_am(AM_Demodulator* demod);

#endif
