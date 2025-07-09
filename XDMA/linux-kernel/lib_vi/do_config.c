#include "do_config.h"
#include <rtpc_xdma.h>

#define DO_CONFIG_IP(__name)                                   		\
static int __name ## _config(unsigned char en, unsigned char ref)      	\
{                                                                       \
	rtpc_xdma_write_register(BAR0, __name ## _En, en);		\
	rtpc_xdma_write_register(BAR0, __name ## _Ref, ref);		\
									\
	printf("%s En = 0x%x  Ref = 0x%x\n", __func__, en, ref);	\
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

static void Call_DO_Config(int channel, unsigned char en, unsigned char ref)
{
	switch(channel) {
	case DO1:
		DO1_config(en, ref);
		break;	
	case DO2:
		DO2_config(en, ref);
		break;	
	case DO3:
		DO3_config(en, ref);
		break;	
	case DO4:
		DO4_config(en, ref);
		break;	
	case DO5:
		DO5_config(en, ref);
		break;	
	case DO6:
		DO6_config(en, ref);
		break;	
	case DO7:
		DO7_config(en, ref);
		break;	
	case DO8:
		DO8_config(en, ref);
		break;	
	case DO9:
		DO9_config(en, ref);
		break;	
	case DO10:
		DO10_config(en, ref);
		break;	
	case DO11:
		DO11_config(en, ref);
		break;	
	case DO12:
		DO12_config(en, ref);
		break;	
	case DO13:
		DO13_config(en, ref);
		break;	
	case DO14:
		DO14_config(en, ref);
		break;	
	case DO15:
		DO15_config(en, ref);
		break;	
	case DO16:
		DO16_config(en, ref);
		break;	
	default:
		printf("unknow DO config\n");
		break;	
	}
}

void handler_f_voit_sec_ref(unsigned char val)
{
	rtpc_xdma_write_register(BAR0, F_VOLT_SEC_REF, val);
}

void handler_do_config(int channel, unsigned char en, unsigned char ref)
{
	Call_DO_Config(channel, en, ref);
}

