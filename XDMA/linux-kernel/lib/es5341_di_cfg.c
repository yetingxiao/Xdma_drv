#include "es5341_di_cfg.h"
#include "vi53xx_api.h"
#include "rtos.h"

static unsigned int di_hl[2][DI_SUM] = {
	{
		0,
		DI1_LOWLEVEL_CONTROL, DI2_LOWLEVEL_CONTROL, DI3_LOWLEVEL_CONTROL, DI4_LOWLEVEL_CONTROL,
                DI5_LOWLEVEL_CONTROL, DI6_LOWLEVEL_CONTROL, DI7_LOWLEVEL_CONTROL, DI8_LOWLEVEL_CONTROL,
                DI9_LOWLEVEL_CONTROL, DI10_LOWLEVEL_CONTROL, DI11_LOWLEVEL_CONTROL, DI12_LOWLEVEL_CONTROL,
                DI13_LOWLEVEL_CONTROL, DI14_LOWLEVEL_CONTROL, DI15_LOWLEVEL_CONTROL, DI16_LOWLEVEL_CONTROL,
	},
	{
		0,
		DI1_HIGHLEVEL_CONTROL, DI2_HIGHLEVEL_CONTROL, DI3_HIGHLEVEL_CONTROL, DI4_HIGHLEVEL_CONTROL,
		DI5_HIGHLEVEL_CONTROL, DI6_HIGHLEVEL_CONTROL, DI7_HIGHLEVEL_CONTROL, DI8_HIGHLEVEL_CONTROL,
		DI9_HIGHLEVEL_CONTROL, DI10_HIGHLEVEL_CONTROL, DI11_HIGHLEVEL_CONTROL, DI12_HIGHLEVEL_CONTROL,
		DI13_HIGHLEVEL_CONTROL, DI14_HIGHLEVEL_CONTROL, DI15_HIGHLEVEL_CONTROL, DI16_HIGHLEVEL_CONTROL,
	}
};

#define DI_CONFIG_IP(__name)                                   		\
static int __name ## _config(unsigned int board_id, unsigned int val)      			\
{                                                                       \
	vi53xx_write_register(board_id, BAR0, __name, val);			\
									\
	return 0;							\
}                                                                       

static void din_vld(unsigned int board_id, unsigned int val)
{
	vi53xx_write_register(board_id, BAR0, DIN_VLD, val);
}

DI_CONFIG_IP(DIN1);
DI_CONFIG_IP(DIN2);
DI_CONFIG_IP(DIN3);
DI_CONFIG_IP(DIN4);

static void Call_DI_Config(unsigned int board_id, int channel, unsigned int val)
{
	switch(channel) {
	case DI1:
	case DI2:
	case DI3:
	case DI4:
		DIN4_config(board_id, val);
		DIN3_config(board_id, 0);
		DIN2_config(board_id, 0);
		DIN1_config(board_id, 0);
		break;	
	case DI5:
	case DI6:
	case DI7:
	case DI8:
		DIN3_config(board_id, val);
		DIN4_config(board_id, 0);
		DIN2_config(board_id, 0);
		DIN1_config(board_id, 0);
		break;	
	case DI9:
	case DI10:
	case DI11:
	case DI12:
		DIN2_config(board_id, val);
		DIN3_config(board_id, 0);
		DIN4_config(board_id, 0);
		DIN1_config(board_id, 0);
		break;	
	case DI13:
	case DI14:
	case DI15:
	case DI16:
		DIN1_config(board_id, val);
		DIN2_config(board_id, 0);
		DIN3_config(board_id, 0);
		DIN4_config(board_id, 0);
		break;	
	default:
		printf("unknown channel : board_id = %d, channel = %d DI config\n", board_id, channel);
		break;	
	}

        din_vld(board_id, 1);
        usleep(1000);
        din_vld(board_id, 0);
}

static unsigned int get_input_code(float val)
{
	unsigned int data = 0;

	data = (unsigned int) (val * 1024) / V;
	
	return data;
}

static unsigned int get_control(int channel, int highlow)
{
	return (di_hl[highlow][channel]);	
} 

static unsigned int get_reg_value(int channel, int highlow, unsigned int input_code)
{
	unsigned int reg_value = 0;
	unsigned int control;
		
	control = get_control(channel, highlow);
	reg_value = (control << MASK) | (input_code << 2);	
	dbg_info("    channel = %d, control = %d, reg_value = 0x%x\n", channel, control, reg_value);

	return reg_value;	
}

static unsigned int get_di_value(int channel, int highlow, float val)
{
	return get_reg_value(channel, highlow, get_input_code(val));	
}

void handler_di_config(unsigned int board_id, int ch, int highlow, float value)
{
	unsigned int val = 0;

	val = get_di_value(ch, highlow, value);
    Call_DI_Config(board_id, ch, val);
}

int es5341_di_cfg_init(unsigned int board_id)
{
	int ret,i;
	struct device_info *info;	

	ret = check_boards_info();
	
	if (ret != 0) {
		if (ret > 0) {
			info = vi53xx_get_error_boards_info();
			for (i=0; i<ret; i++)
				vi53xx_log(LOG_ERR,"info[%d].board_inst = %d, info[%d].board_id = %d\n", i, info[i].board_inst, i, info[i].board_id);
		}
		
		free_board_info(board_id);
		
		return -1;
	}
	
	if (vi53xx_device_open(board_id) < 0) {
		vi53xx_log(LOG_ERR,"16f2 board_id = %d device not found\n",board_id);
		return -1;
	}
		
	return 0;
}

void es5341_di_cfg_exit(unsigned int board_id) 
{
	vi53xx_device_close(board_id);
}
