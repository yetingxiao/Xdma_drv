#include "es5341_do_cfg.h"
#include "vi53xx_api.h"
#include "rtos.h"

#define DO_CONFIG_IP(__name)                                   		\
static int __name ## _config(unsigned int board_id, unsigned char en, unsigned char ref)      	\
{                                                                       \
	vi53xx_write_register(board_id, BAR0, __name ## _En, en);		\
	vi53xx_write_register(board_id, BAR0, __name ## _Ref, ref);		\
									\
	dbg_info("%s board_id = %d En = 0x%x  Ref = 0x%x\n", __func__, board_id, en, ref);	\
	return 0;							\
}                                                                       

DO_CONFIG_IP(DO1);
DO_CONFIG_IP(DO2);
DO_CONFIG_IP(DO3);
DO_CONFIG_IP(DO4);
DO_CONFIG_IP(DO5);
DO_CONFIG_IP(DO6);
DO_CONFIG_IP(DO7);
DO_CONFIG_IP(DO8);
DO_CONFIG_IP(DO9);
DO_CONFIG_IP(DO10);
DO_CONFIG_IP(DO11);
DO_CONFIG_IP(DO12);
DO_CONFIG_IP(DO13);
DO_CONFIG_IP(DO14);
DO_CONFIG_IP(DO15);
DO_CONFIG_IP(DO16);

static void Call_DO_Config(unsigned int board_id, int channel, unsigned char en, unsigned char ref)
{
	switch(channel) {
	case DO1:
		DO1_config(board_id, en, ref);
		break;	
	case DO2:
		DO2_config(board_id, en, ref);
		break;	
	case DO3:
		DO3_config(board_id, en, ref);
		break;	
	case DO4:
		DO4_config(board_id, en, ref);
		break;	
	case DO5:
		DO5_config(board_id, en, ref);
		break;	
	case DO6:
		DO6_config(board_id, en, ref);
		break;	
	case DO7:
		DO7_config(board_id, en, ref);
		break;	
	case DO8:
		DO8_config(board_id, en, ref);
		break;	
	case DO9:
		DO9_config(board_id, en, ref);
		break;	
	case DO10:
		DO10_config(board_id, en, ref);
		break;	
	case DO11:
		DO11_config(board_id, en, ref);
		break;	
	case DO12:
		DO12_config(board_id, en, ref);
		break;	
	case DO13:
		DO13_config(board_id, en, ref);
		break;	
	case DO14:
		DO14_config(board_id, en, ref);
		break;	
	case DO15:
		DO15_config(board_id, en, ref);
		break;	
	case DO16:
		DO16_config(board_id, en, ref);
		break;	
	default:
		printf("unknown channel : board_id = %d, channel = %d DO config \n", board_id, channel);
		break;	
	}
}

void handler_f_voit_sec_ref(unsigned int board_id, unsigned char val)
{
	vi53xx_write_register(board_id, BAR0, F_VOLT_SEC_REF, val);
}

void handler_do_config(unsigned int board_id, int channel, unsigned char en, unsigned char ref)
{
	Call_DO_Config(board_id, channel, en, ref);
}

int es5341_do_cfg_init(unsigned int board_id)
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
		vi53xx_log(LOG_ERR,"board_id = %d device not found\n",board_id);
		return -1;
	}
		
	return 0;
}

void es5341_do_cfg_exit(unsigned int board_id) 
{
	vi53xx_device_close(board_id);
}


