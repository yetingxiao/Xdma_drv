#ifndef VI53XX_API_H
#define VI53XX_API_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/ioctl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <matio.h>

#define CH1									(1)
#define CH2									(2)
#define BAR0								(0x0)
#define BAR1								(0x1)
#define H2C_DIR								(0)
#define C2H_DIR								(1)

#define __user 

#ifdef vi53xx_debug 
    #define dbg_info(format, ...)  printf(format, ##__VA_ARGS__)
#else
    #define dbg_info(format, ...)
#endif

#define H2C_CHANNEL_STATUS_REG              (0x0044)
#define C2H_CHANNEL_STATUS_REG              (0x1044)
#define A_BUFF                              (0x3000)

#define DMA_CHANNEL_NUM                     (2)
#define MAX_DEV_HANDLE                      (128)

#define RTPC_READ							(0)
#define RTPC_WRITE							(1)

/* parVar.mat */
typedef struct parvar {
	char mat_path[512];
	size_t size;
	unsigned int *data;
	unsigned int rows;
	unsigned int columns;
	mat_t *matfile;
	matvar_t *tmp; 
	matvar_t *matvar;
	int data_type;
} parvar_mat_t;


struct vi53xx_file {
	FILE *file;
};

struct device_info {
    unsigned int board_inst;
    unsigned int board_id;
    unsigned int board_type;
    char board_name[32];
};

typedef struct vi53xx_dev_handle {
    int  dev_handle;
    unsigned int board_id;
    char dev_name[32];
} vi53xx_dev_handle;

typedef struct vi53xx_boardid_to_idx {
    int idx;
    unsigned int board_id;

} vi53xx_boardid_to_idx;

struct vi53xx_xdma_info {
    /*reg info*/
    int __user bar;
    unsigned int offset;
    int __user wr;
    unsigned int __user w_val;   
    unsigned int __user *r_val; 

    /*xdma info*/
    unsigned char __user dir;     
    int __user channel;			/* 0-host2card 1-card2host */
    void __user *buf;
    size_t  __user pos;
    size_t  __user len;  
};

#define RTPC_IOCTL_TYPE_XDMA (1)

#define CMD_XDMA_INIT (0)
#define CMD_XDMA_START (CMD_XDMA_INIT + 1)
#define CMD_XDMA_STOP (CMD_XDMA_START + 1)
#define CMD_XDMA_REG (CMD_XDMA_STOP + 1)
#define CMD_XDMA_PRINT (CMD_XDMA_REG + 1)
#define CMD_XDMA_SETTING (CMD_XDMA_PRINT + 1)
#define CMD_XDMA_EXIT (CMD_XDMA_SETTING + 1)
#define CMD_XDMA_DUMP (CMD_XDMA_EXIT + 1)
#define RTPC_IOCTL_XDMA_INIT _IOWR(RTPC_IOCTL_TYPE_XDMA, CMD_XDMA_INIT, struct vi53xx_xdma_info)
#define RTPC_IOCTL_XDMA_START _IOWR(RTPC_IOCTL_TYPE_XDMA, CMD_XDMA_START, struct vi53xx_xdma_info)
#define RTPC_IOCTL_XDMA_STOP _IOWR(RTPC_IOCTL_TYPE_XDMA, CMD_XDMA_STOP, struct vi53xx_xdma_info)
#define RTPC_IOCTL_XDMA_REG _IOWR(RTPC_IOCTL_TYPE_XDMA, CMD_XDMA_REG, struct vi53xx_xdma_info)
#define RTPC_IOCTL_XDMA_PRINT _IOWR(RTPC_IOCTL_TYPE_XDMA, CMD_XDMA_PRINT, struct vi53xx_xdma_info)
#define RTPC_IOCTL_XDMA_SETTING _IOWR(RTPC_IOCTL_TYPE_XDMA, CMD_XDMA_SETTING, struct vi53xx_xdma_info)
#define RTPC_IOCTL_XDMA_EXIT _IOWR(RTPC_IOCTL_TYPE_XDMA, CMD_XDMA_EXIT, struct vi53xx_xdma_info)
#define RTPC_IOCTL_XDMA_DUMP _IOWR(RTPC_IOCTL_TYPE_XDMA, CMD_XDMA_DUMP, struct vi53xx_xdma_info)

int vi53xx_xdma_get_ch_buff(int board_id, int channel, unsigned char *buff, size_t pos, size_t len);
int vi53xx_xdma_set_ch_buff(int board_id, int channel, unsigned char *buff, size_t pos, size_t len);
int vi53xx_xdma_stop(int board_id, int channel, int direction);
int vi53xx_xdma_start(int board_id, int channel, int direction);
int vi53xx_read_register(int board_id, int bar, unsigned int offset, unsigned int *val);
int vi53xx_write_register(int board_id, int bar, unsigned int offset, unsigned int val);
unsigned int vi53xx_get_xdma_poll_status(int board_id, int channel, int dir);
void *vi53xx_xdma_mmap(int board_id, int channel, int direction, size_t size);
void vi53xx_xdma_unmap(int board_id, void *addr, size_t size, int channel, int direction);
int vi53xx_xdma_dump_ch_data(int board_id, int channel, unsigned char *buff, size_t pos, size_t len);

/* board info*/
unsigned int  vi53xx_get_board_type(unsigned int board_id);
struct device_info *vi53xx_get_error_boards_info(void);
void vi53xx_dump_info();
int get_idx(unsigned int board_id);

/* init & exit entry */
int check_boards_info(void);
void free_board_info(unsigned int board_id);
int vi53xx_device_open(unsigned int board_id);
void vi53xx_device_close(unsigned int board_id);

/* log file */
int vi53xx_write_log(void *buf, size_t size);
int vi53xx_log(int priority, const char *format, ...);

/* scope module */
unsigned int get_buff_status(int board_id, unsigned int reg);
void clear_a_buff(int board_id);
void scope_count_sampling(int board_id, unsigned int val);
void start_scope(int board_id);
void scope_select_ch(int board_id, unsigned char *datachlsel);
void scope_slect_mode(int board_id, unsigned char modeSel);

/* parvar mat function */
int set_parvar_mat_path(char *name);
int check_parvar_mat_data_type_uint32(void);
unsigned int get_rows(void);
unsigned int get_columns(void);
unsigned int *get_parvar_data(void);

#endif

