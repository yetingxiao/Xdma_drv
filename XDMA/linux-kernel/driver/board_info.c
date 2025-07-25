#include <linux/version.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include "board_info.h"
#include "vi53xx_proc.h"

struct board_info *vi53xx_inca_board_name_list = NULL;

static char *_get_board_name(unsigned int board_id)
{
	switch (board_id) {
	case ES5341_ID:
		return ES5341;
	case ES5311_ID:
		return ES5311;
	case ES5342_ID:
		return ES5342;
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

static unsigned int _get_major_idx(uint32_t inca_dt)
{
	uint32_t n, idt;

	for(n=0; vi53xx_inca_board_name_list[n].board_name != NULL; n++) {
		idt = vi53xx_inca_board_name_list[n].inca_dt;

		if (inca_dt == idt)
			return n; 
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

unsigned int get_major_idx(uint32_t inca_dt)
{
	return _get_major_idx(inca_dt);
}

static void timer_isr(unsigned long data)
{
    struct device_info *info; 
	void *reg; 
    struct timer_list *used = (struct timer_list *)data;
    info = container_of(used, struct device_info, timer);
	reg = info->reg_base; 

	if (info->state == LED_BLINK_ON) {
		info->state = LED_BLINK_OFF;
	} else {
		info->state = LED_BLINK_ON;
	}

	info->write_reg(reg, DEVICE_LED_BLINK, info->state);

    info->timer.expires = jiffies + msecs_to_jiffies(PERIOD);
    add_timer(&info->timer);
}

static void BlinkLED(void *ifo, int time)
{
    struct device_info *info = (struct device_info *)ifo;

	if (info->create_flag)
		return;

	setup_timer(&info->timer, timer_isr, (unsigned long)&info->timer);
	info->timer.expires = jiffies + msecs_to_jiffies(time);
	add_timer(&info->timer);
	info->create_flag = 1;
}

static void call_blink_control(void *ifo, unsigned int state)
{
    struct device_info *info = (struct device_info *)ifo;

	if (state) {
		BlinkLED(info, 10);
	} else {
		del_timer(&info->timer);
		info->create_flag = 0;
		info->state = LED_BLINK_OFF;
		info->write_reg(info->reg_base, DEVICE_LED_BLINK, info->state);
	}
}

static void call_dump_control(void *ifo, device_control_state *dc)
{
	unsigned char *v = dc->pData;
	unsigned char offset = dc->offset;

	pr_info("0x%x = 0x%x\n", offset, v[offset]);
}

static void device_control(void *info, unsigned int cmd, device_control_state *dc)
{
	if (cmd == BLINK) 
		call_blink_control(info, dc->state);
	else if (cmd == DUMP) 
		call_dump_control(info, dc);
}

void init_device_info(struct device_info *info, void *reg_base)
{
	unsigned int board_type = info->read_reg(reg_base, DEVICE_BOARD_TYPE); 

	sprintf(info->device_name, "%s", get_board_name(board_type));

	info->board_id     = info->read_reg(reg_base , DEVICE_BOARD_ID);
	info->mdl_version  = info->read_reg(reg_base , DEVICE_MDL_VERSION);
	info->fpga_version = info->read_reg(reg_base , DEVICE_FPGA_VERSION);
	info->board_type   = info->read_reg(reg_base , DEVICE_BOARD_TYPE);
	info->serial       = 0x9910001;
	info->led_blink    = info->read_reg(reg_base , DEVICE_LED_BLINK);
	info->reg_base     = reg_base;
	info->create_flag  = 0;

	info->device_control = device_control; 
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

	vi53xx_inca_board_name_list[2].inca_dt = ES5342_ID;
	vi53xx_inca_board_name_list[2].board_name = ES5342;
	vi53xx_inca_board_name_list[2].count = 0;

	create_boards_info_proc();

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

