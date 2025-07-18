#include <linux/version.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include "board_info.h"
#include "vi53xx_proc.h"

struct board_info *vi53xx_inca_board_name_list = NULL;

static void write_reg(void *addr, unsigned int val)
{
	iowrite32(val, addr);
}

static unsigned int read_info(void* addr)
{
	return ioread32(addr);
}

static char *_get_board_name(unsigned int board_id)
{
	switch (board_id) {
	case ES5341_ID:
		return ES5341;
	case ES5311_ID:
		return ES5311;
	default:
		return NULL;
	}

	return NULL;
}

static unsigned int _get_board_instance(uint32_t inca_dt)
{
	uint32_t n, idt;

	for(n=0; vi53xx_inca_board_name_list[n].board_name != NULL; n++) {
		idt = vi53xx_inca_board_name_list[n].inca_dt;

		if (inca_dt == idt)
			return (vi53xx_inca_board_name_list[n].count)++;
	}

	return -1;
}

char *get_board_name(unsigned int board_id)
{
	return _get_board_name(board_id);
}

unsigned int get_board_instance(uint32_t inca_dt)
{
	return _get_board_instance(inca_dt);
}

unsigned int get_board_inca_dt(void *reg_base, unsigned int offset)
{
	return read_info(reg_base + offset);
}

void set_led_blink_state(void *reg_base, unsigned int offset, unsigned int state)
{
	void *reg = reg_base + offset;

	write_reg(reg, state);
}

void init_device_info(struct device_info *info, void *reg_base)
{
	sprintf(info->device_name, "%s", get_board_name(get_board_inca_dt(reg_base, DEVICE_BOARD_TYPE)));

	info->board_id     = read_info(reg_base + DEVICE_BOARD_ID);
	info->mdl_version  = read_info(reg_base + DEVICE_MDL_VERSION);
	info->fpga_version = read_info(reg_base + DEVICE_FPGA_VERSION);
	info->board_type   = read_info(reg_base + DEVICE_BOARD_TYPE);
	info->serial       = 0x9910001;
	info->led_blink    = read_info(reg_base + DEVICE_LED_BLINK);
}

int init_board_info()
{
	int size = sizeof(*vi53xx_inca_board_name_list) * INCA_DT_COUNT;

	vi53xx_inca_board_name_list = kmalloc(size, GFP_KERNEL);
	if (!vi53xx_inca_board_name_list) {
		vi53xx_inca_board_name_list = vmalloc(size);
			if (!vi53xx_inca_board_name_list)
				return -ENOMEM;
	}

	memset(vi53xx_inca_board_name_list, 0, size);

	vi53xx_inca_board_name_list[0].inca_dt = ES5341_ID;
	vi53xx_inca_board_name_list[0].board_name = ES5341;
	vi53xx_inca_board_name_list[0].count = 0;

	vi53xx_inca_board_name_list[1].inca_dt = ES5311_ID;
	vi53xx_inca_board_name_list[1].board_name = ES5311;
	vi53xx_inca_board_name_list[1].count = 0;

	create_board_proc();
	create_instance_board_id_mmap();

	return 0;
}

void board_info_exit()
{
	remove_board_proc();

	if (vi53xx_inca_board_name_list) {
		kfree(vi53xx_inca_board_name_list);
		vi53xx_inca_board_name_list = NULL;
	}
}

