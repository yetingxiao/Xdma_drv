#ifndef BOARD_INFO_H
#define BOARD_INFO_H

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/timer.h>

#define VI53XX_DEV_NAME 	    "vi53xx"
#define VI53XX_DEV_BOARDS       "boards"
#define VI53XX_DEV_MAPING       "mapping"

#define ES5341                  "es5341"
#define ES5311                  "es5311"
#define ES5342                  "es5342"
#define ES5341_ID  		        (0x0000B100)
#define ES5311_ID  		        (0x0000B000)
#define ES5342_ID  		        (0x0000B200)
#define BOARD_TEPE_NUM          (3)

#define KDEVICE_MINOR_COUNT 	(128)
#define INCA_DT_COUNT 		    (10)

#define BLINK					(0x1)
#define LED_BLINK_ON            (0x1)
#define LED_BLINK_OFF           (0x0)
#define PERIOD					(500)

#define DUMP					(0x2)

#define DEVICE_BOARD_BASE       (0x50000)
#define DEVICE_BOARD_TYPE       (DEVICE_BOARD_BASE + 0x300)
#define DEVICE_BOARD_ID         (DEVICE_BOARD_BASE + 0x1C0)
#define DEVICE_MDL_VERSION      (DEVICE_BOARD_BASE + 0x200)
#define DEVICE_FPGA_VERSION     (DEVICE_BOARD_BASE + 0x204)
#define DEVICE_SERIAL           (DEVICE_BOARD_BASE + 0x000)
#define DEVICE_LED_BLINK        (DEVICE_BOARD_BASE + 0x100)

typedef struct {
	unsigned int state;
	int ch;
	int dir;
	unsigned char	*pData;    
	unsigned int	offset;    
} device_control_state;

struct board_info {
	uint32_t  inca_dt;
	char *board_name;
	uint32_t count;
};

struct device_info {
    unsigned int board_id;		
    unsigned int mdl_version;
    unsigned int fpga_version;
    unsigned int board_type;
    unsigned int serial;
    unsigned int led_blink;

	uint32_t    board_inst;		//	MINOR(xcdev->cdevno)
	char	device_name[32];

	void *reg_base;
	int state;
	int create_flag;
    struct timer_list timer;
    unsigned int (*read_reg)(void *reg_base, unsigned int offset);
    unsigned int (*write_reg)(void *reg_base, unsigned int offset, unsigned int val);
	void (*device_control)(void *reg_base, unsigned int cmd, device_control_state *dc);
};

int init_board_info(void);
void board_info_exit(void);
char *get_board_name(unsigned int board_id);
unsigned int get_board_instance(uint32_t inca_dt);
unsigned int get_major_idx(uint32_t inca_dt);
void init_device_info(struct device_info *info, void *reg_base);

#endif

