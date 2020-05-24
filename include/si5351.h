#ifndef SI5351_H
#define SI5351_H

#include <stdint.h>
#include "i2c.h"

#define DEV_WRITE_ADDRESS           0xC0
#define DEV_READ_ADDRESS            0xC1

// Si5351 register map
#define DEVICE_STATUS               0x00
#define SYS_INIT                    0x80
#define LOL_B                       0x40
#define LOL_A                       0x20
#define LOS_CLKIN                   0x10
#define LOS_XTAL                    0x08
#define REVID1                      0x02
#define REVID0                      0x01

#define INTERRUPT_STATUS_STICKY     0x01
#define SYS_INIT_STKY               0x80
#define LOL_B_STKY                  0x40
#define LOL_A_STKY                  0x20
#define LOS_CLKIN_STKY              0x10
#define LOS_XTAL_STKY               0x08

#define INTERRUPT_STATUS_MASK       0x02
#define SYS_INIT_MASK               0x80 
#define LOL_B_MASK                  0x40
#define LOL_A_MASK                  0x20
#define LOS_CLKIN_MASK              0x10
#define LOS_XTAL_MASK               0x08

#define OUTPUT_ENABLE_CONTROL       0x03
#define CLK7_OEB                    0x80
#define CLK6_OEB                    0x40
#define CLK5_OEB                    0x20
#define CLK4_OEB                    0x10
#define CLK3_OEB                    0x08
#define CLK2_OEB                    0x04
#define CLK1_OEB                    0x02
#define CLK0_OEB                    0x01

#define OEB_PIN_ENABLE_CONTROL_MASK 0x09
#define OEB_MASK7                   0x80
#define OEB_MASK6                   0x40
#define OEB_MASK5                   0x20
#define OEB_MASK4                   0x10
#define OEB_MASK3                   0x08
#define OEB_MASK2                   0x04
#define OEB_MASK1                   0x02
#define OEB_MASK0                   0x01

#define PLL_INPUT_SOURCE            0x0F
#define CLKIN_DIV1                  0x80
#define CLKIN_DIV0                  0x40
#define PLLB_SRC                    0x08
#define PLLA_SRC                    0x04

#define CLK0_CONTROL                0x10
#define CLK0_PDN                    0x80
#define MS0_INT                     0x40
#define MS0_SRC                     0x20
#define CLK0_INV                    0x10
#define CLK0_SRC1                   0x08
#define CLK0_SRC0                   0x04
#define CLK0_IDRV1                  0x02
#define CLK0_IDRV0                  0x01

#define CLK1_CONTROL                0x11
#define CLK1_PDN                    0x80
#define MS1_INT                     0x40
#define MS1_SRC                     0x20
#define CLK1_INV                    0x10
#define CLK1_SRC1                   0x08
#define CLK1_SRC0                   0x04
#define CLK1_IDRV1                  0x02
#define CLK1_IDRV0                  0x01

#define CLK2_CONTROL                0x12
#define CLK2_PDN                    0x80
#define MS2_INT                     0x40
#define MS2_SRC                     0x20
#define CLK2_INV                    0x10
#define CLK2_SRC1                   0x08
#define CLK2_SRC0                   0x04
#define CLK2_IDRV1                  0x02
#define CLK2_IDRV0                  0x01

#define CLK3_CONTROL                0x13
#define CLK3_PDN                    0x80
#define MS3_INT                     0x40
#define MS3_SRC                     0x20
#define CLK3_INV                    0x10
#define CLK3_SRC1                   0x08
#define CLK3_SRC0                   0x04
#define CLK3_IDRV1                  0x02
#define CLK3_IDRV0                  0x01

#define CLK4_CONTROL                0x14
#define CLK4_PDN                    0x80
#define MS4_INT                     0x40
#define MS4_SRC                     0x20
#define CLK4_INV                    0x10
#define CLK4_SRC1                   0x08
#define CLK4_SRC0                   0x04
#define CLK4_IDRV1                  0x02
#define CLK4_IDRV0                  0x01

#define CLK5_CONTROL                0x15
#define CLK5_PDN                    0x80
#define MS5_INT                     0x40
#define MS5_SRC                     0x20
#define CLK5_INV                    0x10
#define CLK5_SRC1                   0x08
#define CLK5_SRC0                   0x04
#define CLK5_IDRV1                  0x02
#define CLK5_IDRV0                  0x01

#define CLK6_CONTROL                0x16
#define CLK6_PDN                    0x80
#define FBA_INT                     0x40
#define MS6_SRC                     0x20
#define CLK6_INV                    0x10
#define CLK6_SRC1                   0x08
#define CLK6_SRC0                   0x04
#define CLK6_IDRV1                  0x02
#define CLK6_IDRV0                  0x01

#define CLK7_CONTROL                0x17
#define CLK7_PDN                    0x80
#define FBB_INT                     0x40
#define MS7_SRC                     0x20
#define CLK7_INV                    0x10
#define CLK7_SRC1                   0x08
#define CLK7_SRC0                   0x04
#define CLK7_IDRV1                  0x02
#define CLK7_IDRV0                  0x01

#define DIS_STATE_LOW               0x00
#define DIS_STATE_HIGH              0x01
#define DIS_STATE_HIGH_IMPEDANCE    0x02
#define DIS_STATE_NEVER             0x03

#define CLK30_DISABLE_STATE         0x18
#define CLK3_DIS_STATE1             0x80
#define CLK3_DIS_STATE0             0x40
#define CLK2_DIS_STATE1             0x20
#define CLK2_DIS_STATE0             0x10
#define CLK1_DIS_STATE1             0x08
#define CLK1_DIS_STATE0             0x04
#define CLK0_DIS_STATE1             0x02
#define CLK0_DIS_STATE0             0x01

#define CLK3_DIS_STATE_Pos          6
#define CLK2_DIS_STATE_Pos          4
#define CLK1_DIS_STATE_Pos          2
#define CLK0_DIS_STATE_Pos          0

#define CLK74_DISABLE_STATE         0x19
#define CLK7_DIS_STATE1             0x80
#define CLK7_DIS_STATE0             0x40
#define CLK6_DIS_STATE1             0x20
#define CLK6_DIS_STATE0             0x10
#define CLK5_DIS_STATE1             0x08
#define CLK5_DIS_STATE0             0x04
#define CLK4_DIS_STATE1             0x02
#define CLK4_DIS_STATE0             0x01

#define CLK7_DIS_STATE_Pos          6
#define CLK6_DIS_STATE_Pos          4
#define CLK5_DIS_STATE_Pos          2
#define CLK4_DIS_STATE_Pos          0

// this part is a cluster

#define MSNA                        0x1A
#define MSNB                        0x22

#define MS0                         0x2A
#define MS1                         0x32
#define MS2                         0x3A
#define MS3                         0x42
#define MS4                         0x4A
#define MS5                         0x52

#define MS6                         0x5A

#define SSC                         0x95

#define VCXO_PARAM                  0xA2

// and now more stuff
#define CLK0_PHOFF                  0xA5
#define CLK1_PHOFF                  0xA6
#define CLK2_PHOFF                  0xA7
#define CLK3_PHOFF                  0xA8
#define CLK4_PHOFF                  0xA9
#define CLK5_PHOFF                  0xAA

#define PLL_RESET                   0xB1
#define PLLB_RST                    0x80
#define PLLA_RST                    0x20

#define XTAL_LOAD_CAPACITANCE       0xB7
#define XTAL_CL1                    0x80
#define XTAL_CL0                    0x40

#define FANOUT_ENABLE               0xBB
#define CLKIN_FANOUT_EN             0x80
#define XO_FANOUT_EN                0x40
#define MS_FANOUT_EN                0x10

typedef struct {
    uint32_t P1;
    uint32_t P2;
    uint32_t P3;
} FBMS_Config;

void si5351_write_fbms_config(uint8_t msn, FBMS_Config config);

typedef struct {
    uint32_t P1;
    uint32_t P2;
    uint32_t P3;
    uint8_t DIV;
    uint8_t DIVBY4;
} OMS05_Config;

void si5351_write_oms05_config(uint8_t ms, OMS05_Config config);

typedef struct {
    uint8_t MS6_P1;
    uint8_t MS7_P1;
    uint8_t R6_DIV;
    uint8_t R7_DIV;
} OMS67_Config;

void si5351_write_oms67_config(OMS67_Config config);

typedef struct {
    uint16_t DN_P1;
    uint16_t DN_P2;
    uint16_t DN_P3;
    uint16_t UP_P1;
    uint16_t UP_P2;
    uint16_t UP_P3;
    uint16_t UDP;
    uint8_t SSC_EN;
    uint8_t SSC_MODE;
    uint8_t NCLK;
} SSC_Config;

void si5351_write_ssc_config(SSC_Config config);

void si5351_write_vcxo_param(uint32_t param);

void si5351_write_reg(uint8_t reg, uint8_t value);

void si5351_powerdown();

#endif
