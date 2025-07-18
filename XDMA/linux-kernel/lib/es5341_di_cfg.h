
#ifndef ES5341_DI_CFG_H
#define ES5341_DI_CFG_H

#define DI_CONFIG_IP_BASE		(0x20000)

#define DIN1				(DI_CONFIG_IP_BASE + 0x100)
#define DIN2				(DI_CONFIG_IP_BASE + 0x110)
#define DIN3				(DI_CONFIG_IP_BASE + 0x120)
#define DIN4				(DI_CONFIG_IP_BASE + 0x130)
#define DIN_VLD				(DI_CONFIG_IP_BASE + 0x150)

#define DI1_HIGHLEVEL_CONTROL		(0x3)
#define DI1_LOWLEVEL_CONTROL		(0x4)
#define DI2_HIGHLEVEL_CONTROL		(0x6)
#define DI2_LOWLEVEL_CONTROL		(0x5)
#define DI3_HIGHLEVEL_CONTROL		(0x1)
#define DI3_LOWLEVEL_CONTROL		(0x2)
#define DI4_HIGHLEVEL_CONTROL		(0x8)
#define DI4_LOWLEVEL_CONTROL		(0x7)

#define DI5_HIGHLEVEL_CONTROL		(0x3)
#define DI5_LOWLEVEL_CONTROL		(0x4)
#define DI6_HIGHLEVEL_CONTROL		(0x6)
#define DI6_LOWLEVEL_CONTROL		(0x5)
#define DI7_HIGHLEVEL_CONTROL		(0x1)
#define DI7_LOWLEVEL_CONTROL		(0x2)
#define DI8_HIGHLEVEL_CONTROL		(0x8)
#define DI8_LOWLEVEL_CONTROL		(0x7)

#define DI9_HIGHLEVEL_CONTROL		(0x3)
#define DI9_LOWLEVEL_CONTROL		(0x4)
#define DI10_HIGHLEVEL_CONTROL		(0x6)
#define DI10_LOWLEVEL_CONTROL		(0x5)
#define DI11_HIGHLEVEL_CONTROL		(0x1)
#define DI11_LOWLEVEL_CONTROL		(0x2)
#define DI12_HIGHLEVEL_CONTROL		(0x7)
#define DI12_LOWLEVEL_CONTROL		(0x8)

#define DI13_HIGHLEVEL_CONTROL		(0x3)
#define DI13_LOWLEVEL_CONTROL		(0x4)
#define DI14_HIGHLEVEL_CONTROL		(0x7)
#define DI14_LOWLEVEL_CONTROL		(0x8)
#define DI15_HIGHLEVEL_CONTROL		(0x1)
#define DI15_LOWLEVEL_CONTROL		(0x2)
#define DI16_HIGHLEVEL_CONTROL		(0x5)
#define DI16_LOWLEVEL_CONTROL		(0x6)

#define V 				(2.5)
#define MASK 				(12)

enum _DI {
	DI1 = 1,
	DI2,
	DI3,
	DI4,
	DI5,
	DI6,
	DI7,
	DI8,
	DI9,
	DI10,
	DI11,
	DI12,
	DI13,
	DI14,
	DI15,
	DI16,
	DI_SUM,
};

void handler_di_config(unsigned int board_id, int channel, int highlow, float value);
int es5341_di_cfg_init(unsigned int board_id);
void es5341_di_cfg_exit(unsigned int board_id);

#endif
