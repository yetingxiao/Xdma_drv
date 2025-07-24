#include "vi53xx_api.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

struct vi53xx_file rfile = { 
	.file = NULL,
};

#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#define BOARD_NUM (128)
#define BITMAP_SIZE (MAX_DEV_HANDLE / 8)

static int available_board_count = 0;
static int online_board_count = 0;
static int init_check_ok = 0;
static struct device_info *board_info = NULL;
static struct device_info *board_error_info = NULL;
uint8_t id_bitmap[BITMAP_SIZE];
static vi53xx_dev_handle *dev_handle_list[MAX_DEV_HANDLE];
static vi53xx_boardid_to_idx *dev_boardid_to_idx_list[MAX_DEV_HANDLE];
static parvar_mat_t pmt;

static void init_parvar_mat(void)
{
	pmt.tmp = Mat_VarReadNext(pmt.matfile);
	if (pmt.tmp) {
		pmt.matvar = Mat_VarRead(pmt.matfile, pmt.tmp->name);
		if (pmt.matvar->data_type == MAT_T_UINT32) {
			pmt.rows = pmt.matvar->dims[0];
			pmt.columns = pmt.matvar->dims[1];
			pmt.size = Mat_VarGetSize(pmt.matvar) / sizeof(unsigned int);
			pmt.data = (unsigned int *)pmt.matvar->data;
			pmt.data_type = 1;
		} else {
			pmt.data_type = 0;
		}
	}
}

int check_parvar_mat_data_type_uint32(void)
{
	return pmt.data_type;
}

unsigned int get_rows(void)
{
	return pmt.rows;
}

unsigned int get_columns(void)
{
	return pmt.columns;
}

unsigned int *get_parvar_data(void)
{
	return (unsigned int *)pmt.data;
}

static void release_parvar_mat(void)
{
	if (pmt.matvar) {
		Mat_VarFree(pmt.matvar);
		pmt.matvar = NULL;
		pmt.data = NULL;
	}
	
	if(pmt.matfile){
		Mat_Close(pmt.matfile);
		pmt.matfile = NULL;
	}
}

static void convert_to_lower(char *str)
{
    while (*str) {
        if (*str >= 'A' && *str <= 'Z') {
            *str = *str + ('a' - 'A');
        }
        str++;
    }
}

int set_parvar_mat_path(char *name)
{
	char str[256] = {0};
	char *format = "/home/rtpc/codegen/simodels/%s/external/code/src/simulink_%s/parvar.mat";

	if (!name)
		return -1;

	strcpy(str, name);
    convert_to_lower(str);

	memset(&pmt, 0x0, sizeof(parvar_mat_t));

	sprintf(pmt.mat_path, format, str, str);

	pmt.matfile = Mat_Open(pmt.mat_path, MAT_ACC_RDONLY);
	if (!pmt.matfile) 
		return -1;
	
	init_parvar_mat();

	return 0;
}

size_t get_parvar_mat_size(void)
{
	return pmt.size; 
}

void init_id_bitmap() 
{
	int  i = 0;

    for (i = 0; i < BITMAP_SIZE; i++) {
        id_bitmap[i] = 0;
    }
}

int get_unique_id() 
{
	int i = 0;
	int index, offset;

    for (i = 0; i < MAX_DEV_HANDLE; i++) {
        index = i / 8;
        offset = i % 8;
        if ((id_bitmap[index] & (1 << offset)) == 0) {
            id_bitmap[index] |= (1 << offset); 

            return i;
        }
    }

    return 0;  
}

void destroy_id(int id) 
{
	int index, offset;

	if (id >= 0 && id < MAX_DEV_HANDLE) {
		index = id / 8;
        offset = id % 8;
        id_bitmap[index] &= ~(1 << offset); 
	}
}

static int _get_idx(unsigned int board_id)
{
    int n;
    vi53xx_boardid_to_idx *db;

    for(n=0; n < MAX_DEV_HANDLE; n++) {
        if((db=dev_boardid_to_idx_list[n]) != NULL) {
            if(db->board_id == board_id) 
                return db->idx;
        }
    }

    return 0;
}

static int vi53xx_logva(int priority, const char *format, va_list args)
{
    static const char * const logStrings[] = { "emerg", "alert", "crit", "error", "warning", "notice", "info", "debug", "unknown"};
    unsigned int index = MIN(priority, sizeof(logStrings)/sizeof(char*));

    fprintf(stderr, "[vi53xx %s] ", logStrings[index]);

    return vfprintf(stderr, format, args);
}

int vi53xx_log(int priority, const char *format, ...)
{
	va_list args;
	
	va_start(args, format);
	int ret = vi53xx_logva(priority, format, args);
	va_end(args);
	
	return ret;
}

static char *_get_board_name(unsigned int board_type)
{
	int i = 0;

    for (i=0; i<available_board_count; i++) {
		if (board_info[i].board_type == board_type) 
			return board_info[i].board_name; 
    }

    return 0;	
}

static unsigned int _get_board_instance(unsigned board_id)
{
    int i = 0;

    for (i=0; i<available_board_count; i++) {
        if (board_info[i].board_id == board_id) 
			return board_info[i].board_inst; 
    }

    return 0;	
}

static char *get_board_name(unsigned int board_type)
{
	return _get_board_name(board_type);
}

static unsigned int get_board_instance(unsigned int board_id)
{
	return _get_board_instance(board_id);
}

static int get_available_board_count()
{
    if (available_board_count > MAX_DEV_HANDLE)
        return 0;

    return available_board_count;
}

static int get_online_board_count()
{
	if (online_board_count <= 0)
		return 0;

	return online_board_count;
}

static int set_online_board_count(int cnt)
{
	online_board_count = cnt;
	
	return 0;
}

static void free_mem()
{
    if (board_info) {
        free(board_info);
        board_info = NULL;
    }

    if (board_error_info) {
        free(board_error_info);
        board_error_info = NULL;
    }
}

static struct vi53xx_dev_handle *get_dev_handle(unsigned int board_id)
{
    int n;
    vi53xx_dev_handle *dh;
    int cnt = get_available_board_count();

    for(n=0; n < cnt; n++) {
        if((dh=dev_handle_list[n]) != NULL) {
            if(dh->board_id == board_id) 
				return dh;
        }
    }

    return NULL;
}

static inline long xdma_call_ioctl(int dev_handle, unsigned long request, unsigned long arg)
{
	long ret = 0;

	ret = ioctl(dev_handle, request, arg);
	if (ret < 0) {
		printf("[error] call cmd %lx fail, ret is \n", request);
	}

	return ret;
}

int vi53xx_xdma_read_write_reg(int dev_handle, unsigned long arg)
{
	struct vi53xx_xdma_info *reg = (struct vi53xx_xdma_info *)arg;

    if (!reg)
        return -1;

    if ( (reg->bar < 0) || (reg->bar > 1) )
        return -1;

    if ( (reg->wr < 0) || (reg->wr > 1) )
        return -1;

    dbg_info("[%s:%s:%d] bar=%d, offset = 0x%x, wr = %s\n",__FILE__,__func__, __LINE__, reg->bar, reg->offset, reg->wr ? "write" : "read");

	return xdma_call_ioctl(dev_handle, RTPC_IOCTL_XDMA_REG, arg);
} 

int vi53xx_xdma_read(int dev_handle, unsigned long arg)
{
	struct vi53xx_xdma_info *read = (struct vi53xx_xdma_info *)arg;
    
    if (!read)
        return -1;

    if (!read->buf)
		return -1;

    if (read->pos < 0)
        return -1;

    if ( (read->len) < 0 )
        return -1;
    
    dbg_info("[%s:%s:%d] pos=%ld, len = %d\n",__FILE__,__func__, __LINE__, read->pos, read->len);

    return xdma_call_ioctl(dev_handle, RTPC_IOCTL_XDMA_PRINT, arg);
}

int vi53xx_xdma_write(int dev_handle, unsigned long arg)
{
	struct vi53xx_xdma_info *write = (struct vi53xx_xdma_info *)arg;
    
	if (!write)
		return -1;

   	if (!write->buf)
		return -1;

   	if (write->pos < 0)
   	    return -1;

   	if ( (write->len < 0) )
   	    return -1;

	dbg_info("[%s:%s:%d] pos=%ld, len = %d\n",__FILE__,__func__, __LINE__, write->pos, write->len);

   	return xdma_call_ioctl(dev_handle, RTPC_IOCTL_XDMA_INIT, arg);
}

int _vi53xx_xdma_start(int dev_handle, unsigned long arg)
{
   	 struct vi53xx_xdma_info *start = (struct vi53xx_xdma_info *)arg;

   	 if (!start)
   	     return -1;

   	 if ( (start->channel < 0) || (start->channel > 4) )
   	     return -1;

   	 if ( (start->dir < 0 ) || (start->dir > 1) )
   	     return -1;

   	 dbg_info("[%s:%s:%d] channel=%d, direction = %d\n",__FILE__,__func__, __LINE__, start->channel, start->dir);

   	 return xdma_call_ioctl(dev_handle, RTPC_IOCTL_XDMA_START, arg);
}

int _vi53xx_xdma_stop(int dev_handle, unsigned long arg)
{
   	 struct vi53xx_xdma_info *stop = (struct vi53xx_xdma_info *)arg;

   	 if (!stop)
   	     return -1;

   	 if ( (stop->channel < 0) || (stop->channel > 4) )
   	     return -1;

   	 if ( (stop->dir < 0) || (stop->dir > 0x1) ) 
   	     return -1;
   	 
   	 dbg_info("[%s:%s:%d] channel=%d, direction = %d\n",__FILE__,__func__, __LINE__, stop->channel, stop->dir);

   	 return xdma_call_ioctl(dev_handle, RTPC_IOCTL_XDMA_STOP, arg);
}

static int _vi53xx_xdma_mmap_setting(int dev_handle, unsigned long arg)
{
   	 struct vi53xx_xdma_info *info = (struct vi53xx_xdma_info *)arg;

   	 if (!info)
   	     return -1;

   	 if ( (info->channel < 0) || (info->channel > 4) )
   	     return -1;

   	 if ( (info->dir < 0) || (info->dir > 0x1) ) 
   	     return -1;
   	 
   	 dbg_info("[%s:%s:%d] channel=%d, direction = %d\n",__FILE__,__func__, __LINE__, info->channel, info->dir);

   	 return xdma_call_ioctl(dev_handle, RTPC_IOCTL_XDMA_SETTING, arg);
}

static int _vi53xx_xdma_exit(int dev_handle, unsigned long arg)
{
   	 struct vi53xx_xdma_info *info = (struct vi53xx_xdma_info *)arg;

   	 if (!info)
   	     return -1;

   	 if ( (info->channel < 0) || (info->channel > 4) )
   	     return -1;

   	 if ( (info->dir < 0) || (info->dir > 0x1) ) 
   	     return -1;
   	 
   	 dbg_info("[%s:%s:%d] channel=%d, direction = %d\n",__FILE__,__func__, __LINE__, info->channel, info->dir);

   	 return xdma_call_ioctl(dev_handle, RTPC_IOCTL_XDMA_EXIT, arg);
}

int vi53xx_xdma_dump(int dev_handle, unsigned long arg)
{
	struct vi53xx_xdma_info *dump = (struct vi53xx_xdma_info *)arg;
    
    if (!dump)
        return -1;

    if (!dump->buf)
		return -1;

    if (dump->pos < 0)
        return -1;

    if (dump->len < 0)
        return -1;
    
   	if ( (dump->channel < 0) || (dump->channel > 4) )
   	    return -1;

    dbg_info("[%s:%s:%d] pos=%ld, len = %d\n",__FILE__,__func__, __LINE__, dump->pos, dump->len);

    return xdma_call_ioctl(dev_handle, RTPC_IOCTL_XDMA_DUMP, arg);
}

int vi53xx_write_register(int board_id, int bar, unsigned int offset, unsigned int val)
{
    struct vi53xx_xdma_info reg;
    int dev_handle;
    vi53xx_dev_handle *dh = get_dev_handle(board_id);;

	reg.bar = bar;
	reg.offset = offset;
	reg.wr = RTPC_WRITE;
	reg.w_val = val;

    if (!dh)
        return -1;

    dev_handle = dh->dev_handle;
	return vi53xx_xdma_read_write_reg(dev_handle, (long)&reg);	
}

int vi53xx_read_register(int board_id, int bar, unsigned int offset, unsigned int *val)
{
    struct vi53xx_xdma_info reg;
    int dev_handle;
    vi53xx_dev_handle *dh = get_dev_handle(board_id);;

	reg.bar = bar;
	reg.offset = offset;
	reg.wr = RTPC_READ;
	reg.r_val = val;

    if (!dh)
		return -1;

    dev_handle = dh->dev_handle;

	return vi53xx_xdma_read_write_reg(dev_handle, (long)&reg);	
}

int vi53xx_xdma_stop(int board_id, int channel, int direction)
{
	struct vi53xx_xdma_info stop;
    int dev_handle;
    vi53xx_dev_handle *dh = get_dev_handle(board_id);;

	stop.channel = channel;
	stop.dir = direction;

    if (!dh)
        return -1;

    dev_handle = dh->dev_handle;

    return _vi53xx_xdma_stop(dev_handle, (long)&stop);
}

int vi53xx_xdma_start(int board_id, int channel, int direction)
{
	struct vi53xx_xdma_info start;
    int dev_handle;
    vi53xx_dev_handle *dh = get_dev_handle(board_id);;

	start.channel = channel;
	start.dir = direction;

    if (!dh)
		return -1;

    dev_handle = dh->dev_handle;

    return _vi53xx_xdma_start(dev_handle, (long)&start);
}

int vi53xx_xdma_set_ch_buff(int board_id, int channel, unsigned char *buff, size_t pos, size_t len)
{
    struct vi53xx_xdma_info  info;
    int dev_handle;
    vi53xx_dev_handle *dh = get_dev_handle(board_id);;

	info.channel = channel;
	info.buf = buff;
	info.pos = pos;
	info.len = len;

    if (!dh)
        return -1;

    dev_handle = dh->dev_handle;
    return  vi53xx_xdma_write(dev_handle, (long)&info);
}

int vi53xx_xdma_get_ch_buff(int board_id, int channel, unsigned char *buff, size_t pos, size_t len)
{
    struct vi53xx_xdma_info  info;
    int dev_handle;
    vi53xx_dev_handle *dh = get_dev_handle(board_id);;

	info.channel = channel;
	info.buf = buff;
	info.pos = pos;
	info.len = len;

    if (!dh)
        return -1;

    dev_handle = dh->dev_handle;
	return vi53xx_xdma_read(dev_handle, (long)&info);
}

int vi53xx_xdma_dump_ch_data(int board_id, int channel, unsigned char *buff, size_t pos, size_t len)
{
    struct vi53xx_xdma_info  info;
    int dev_handle;
    vi53xx_dev_handle *dh = get_dev_handle(board_id);

    if (!dh)
        return -1;

    dev_handle = dh->dev_handle;

	info.channel = channel;
	info.buf = buff;
	info.pos = pos;
	info.len = len;

	return vi53xx_xdma_dump(dev_handle, (long)&info);
}


static unsigned int get_h2c_status(int dev_handle, int channel)
{
	unsigned int val = 0;
	
	vi53xx_read_register(dev_handle, BAR1, H2C_CHANNEL_STATUS_REG + (channel-1) * 0x100, &val);
	
	return val;
}

static unsigned int get_c2h_status(int dev_handle, int channel)
{
	unsigned int val = 0;
	
	vi53xx_read_register(dev_handle, BAR1, C2H_CHANNEL_STATUS_REG + (channel-1) * 0x100, &val);
	
	return val;
}

unsigned int vi53xx_get_xdma_poll_status(int board_id, int channel, int dir)
{
    int dev_handle = board_id;

	if (dir == H2C_DIR)		
		return get_h2c_status(dev_handle, channel);
	else 
		return get_c2h_status(dev_handle, channel);
}

/* mmap channel to userspace */
static void *_vi53xx_xdma_mmap(int dev_handle, size_t size)
{
	void *addr;

    	/* mmap */
	addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_handle, 0);
	if (!addr) {
        dbg_info("[%s:%s:%d] mmap failed\n",__FILE__,__func__, __LINE__);
		close(dev_handle);
		return NULL;
	}

	return addr;
}

void *vi53xx_xdma_mmap(int board_id, int channel, int direction, size_t size)
{ 
	struct vi53xx_xdma_info  info;
    int dev_handle;
    vi53xx_dev_handle *dh = get_dev_handle(board_id);

    if (!dh)
        return NULL;

    dev_handle = dh->dev_handle;
	
	info.channel = channel;
	info.dir = direction;
	
	_vi53xx_xdma_mmap_setting(dev_handle, (long)&info);
	
	return _vi53xx_xdma_mmap(dev_handle, size);
}

void vi53xx_xdma_unmap(int board_id, void *addr, size_t size, int channel, int direction)
{
	struct vi53xx_xdma_info  info;
    int dev_handle;
    vi53xx_dev_handle *dh = get_dev_handle(board_id);

    if (!dh)
        return ;

    dev_handle = dh->dev_handle;
	
	info.channel = channel;
	info.dir = direction;
	
	_vi53xx_xdma_exit(dev_handle, (long)&info);
	munmap(addr, size);
}

/* scope module ops */
static void scope_init_config(int dev_handle)
{
        /*auto config*/
	vi53xx_write_register(dev_handle, BAR0, 0x04, 0x00);
	vi53xx_write_register(dev_handle, BAR0, 0x0c, 0x00);

	vi53xx_write_register(dev_handle, BAR0, 0x1004, 0x00);
	vi53xx_write_register(dev_handle, BAR0, 0x100c, 0x00);

	vi53xx_write_register(dev_handle, BAR0, 0x2004, 0x00);
	vi53xx_write_register(dev_handle, BAR0, 0x200c, 0x00);

	vi53xx_write_register(dev_handle, BAR0, 0x3004, 0xFFFFFFFF);
	vi53xx_write_register(dev_handle, BAR0, 0x300c, 0xFFFFFFFF);
}

void scope_select_ch(int board_id, unsigned char *datachlsel)
{
	unsigned int tmpval = 0;
	unsigned char i = 0;
    int dev_handle = board_id;

	tmpval = 0;
	for(i=0; i<4; i++) {
		tmpval |= datachlsel[i]<<(8*i);
	}
	vi53xx_write_register(dev_handle, BAR0, 0x00, tmpval);

	tmpval = 0;
	for(i=0; i<4; i++) {
		tmpval |= datachlsel[i+4]<<(8*i);
	}
	vi53xx_write_register(dev_handle, BAR0, 0x08, tmpval);
}

void scope_slect_mode(int board_id, unsigned char modeSel)
{
	unsigned int value = 0x0;
    int dev_handle = board_id;

	if (modeSel) {
		value |= (1 << 16);
		value += 1;
	} else {
		value = 1;
	}

	vi53xx_write_register(dev_handle, BAR0, 0x2008, 0x0);
	vi53xx_write_register(dev_handle, BAR0, 0x2000, value);
	vi53xx_write_register(dev_handle, BAR0, 0x2008, 0x1);
}

void start_scope(int board_id)
{
    int dev_handle = board_id;

	vi53xx_write_register(dev_handle, BAR0, 0x4000, 0x1);
    usleep(1);
	vi53xx_write_register(dev_handle, BAR0, 0x4000, 0x0);
}       

static void scope_clock_sampling(int board_id)
{
    int dev_handle = board_id;
	vi53xx_write_register(dev_handle, BAR0, 0x1000, 150);
}

void scope_count_sampling(int board_id, unsigned int val)
{
    int dev_handle = board_id;
	vi53xx_write_register(dev_handle, BAR0, 0x1008, val);
}

void clear_a_buff(int board_id)
{
    int dev_handle = board_id;
    unsigned int val  = 0x0;

	vi53xx_read_register(dev_handle, BAR0, 0x2008, &val);

    val |= (1 << 1);
	vi53xx_write_register(dev_handle, BAR0, 0x2008, val);

    usleep(1);

    val &= ~(1 << 1);
	vi53xx_write_register(dev_handle, BAR0, 0x2008, val);
}

unsigned int get_buff_status(int board_id, unsigned int reg)
{
	unsigned int val = 0;
    int dev_handle = board_id;

    vi53xx_read_register(dev_handle, BAR0, reg, &val);

    return val;
}

void scope_module_init(int dev_handle)
{
    vi53xx_xdma_stop(dev_handle, CH1, H2C_DIR);
    vi53xx_xdma_stop(dev_handle,CH2, C2H_DIR);

    /* init config */
    scope_init_config(dev_handle);

    /* clock sampling */
    scope_clock_sampling(dev_handle);
}

/* log file */
#define LOG_NAME "/home/vi53xx/log"

int vi53xx_write_log(void *buf, size_t size)
{
	fwrite(buf, 1, size, rfile.file);
    fflush(rfile.file);

    return 0;
}

static void dump_board_info(struct device_info *info)
{
    dbg_info("board_name = %s, board_type = 0x%x, board_inst = %d, board_id = %d\n",
			info->board_name, info->board_type, 
            info->board_inst, info->board_id
            );
}

static int extractValue(const char *filename, struct device_info *info)
{
    char line[256];
    unsigned int  Inst, id, type;
	char *name;
    int board_num = 0;
	char *separators = " \t\n"; 
    FILE *file = fopen(filename, "r");

    if (file == NULL) 
        return -1;
    
    while (fgets(line, sizeof(line), file) != NULL) 
    {
		name = strtok(line, separators);
        if(sscanf(line+11, "%x:%d:%x", &Inst,  &id, &type) == 3) {
            info[board_num].board_inst = Inst;
            info[board_num].board_id = id;
            info[board_num].board_type = type;
			sprintf(info[board_num].board_name, "%s", name);
            dump_board_info(&info[board_num]);
            board_num++;
        }
    }

    fclose(file);
    return board_num; 
}

static int _check_boards_info(struct device_info *info, struct device_info *err_info, int count)
{   
    int Index = 0;
    int i,j;

    if (count == 1)
        return 0;

    for (i = 0; i < count; i++) {
        for (j = i + 1; j < count+1; j++) {
            if (info[i].board_id == info[j].board_id) {
                err_info[Index++] = info[i];
                err_info[Index++] = info[j];
                break; 
            }
        }
    }

    return Index;
}

struct device_info *vi53xx_get_error_boards_info(void)
{
    return board_error_info;
}

static int requst_mem(int size) 
{ 
    board_info = (struct device_info *)malloc(size);
    if (NULL == board_info)
        return -1;

    memset(board_info, 0, size);

    board_error_info = (struct device_info *)malloc(size);
    if (NULL == board_error_info) 
        return -1;

    memset(board_error_info, 0, size);

    return 0;
}

static void init_dev_handle_list()
{
    int n;

    for(n=0; n < MAX_DEV_HANDLE; n++) {
        dev_handle_list[n] = NULL; 
    }
}

static void init_dev_boaid_to_idx_list()
{
    int n;

    for(n=0; n < MAX_DEV_HANDLE; n++) {
        dev_boardid_to_idx_list[n] = NULL; 
    }
}

static void dump_available_boards_info(int cnt)
{
    int i = 0;

    for (i=0; i<cnt; i++) {
		dbg_info("board_name = %s, board_type = 0x%x, board_inst = %d, board_id = %d\n",
                 board_info[i].board_name, board_info[i].board_type, 
                 board_info[i].board_inst, board_info[i].board_id
                );
    }
}

static void init_config()
{
    init_dev_handle_list();
    init_dev_boaid_to_idx_list();
	init_id_bitmap();
}

/*return
 * < 0: no mem ,/proc/vi53xx/maping   no device
 * = 0: success  
 * > 0: err_boards num
 * */
int check_boards_info(void)
{
    int board_num = 0;
    int ret = -1;
    const char *filename = "/proc/vi53xx/maping";
    int size = BOARD_NUM * sizeof(struct device_info);

    if (init_check_ok)
        return 0;

    init_check_ok = 1;

	init_config();

    ret = requst_mem(size);
    if (ret < 0) {
        printf("oom\n");
        return ret;
    }

    board_num = extractValue(filename, board_info);
    if (board_num <= 0) {
        return ret;
    }

    available_board_count = board_num;

    dump_available_boards_info(available_board_count);

    return _check_boards_info(board_info, board_error_info, board_num);
}

void free_board_info(unsigned int board_id)
{
	int idx = _get_idx(board_id);

	free_mem();
	destroy_id(idx);	
	init_dev_handle_list();
	init_dev_boaid_to_idx_list();
	release_parvar_mat();
    init_check_ok = 0;
}

unsigned int  vi53xx_get_board_type(unsigned int board_id)
{
    int i = 0;

    for (i=0; i<available_board_count; i++) {
		if (board_info[i].board_id == board_id) 
            return board_info[i].board_type; 
    }

    return 0;
}

/* board init entry */
static int get_board_handle(unsigned int board_id)
{
	int dev_handle = 0;
	char board_name[32];
    unsigned int board_type = vi53xx_get_board_type(board_id);
    unsigned int board_inst = get_board_instance(board_id);

	// board_type  instance	
	sprintf(board_name, "/dev/%s_%d", get_board_name(board_type), board_inst);
	dev_handle = open(board_name, O_RDWR);

	return dev_handle;
}

int get_idx(unsigned int board_id)
{
    int n;
    vi53xx_boardid_to_idx *db;

    for(n=0; n < MAX_DEV_HANDLE; n++) {
        if((db=dev_boardid_to_idx_list[n]) != NULL) {
            if(db->board_id == board_id) 
                return db->idx;
        }
    }

    db = malloc(sizeof(vi53xx_boardid_to_idx)); 
    if (NULL == db) {
        return -errno;
    }

    db->idx        = get_unique_id();
    db->board_id   = board_id;

    for(n=0; n < MAX_DEV_HANDLE; n++) {
        if(__sync_bool_compare_and_swap(dev_boardid_to_idx_list+n, NULL, db)) {
            return db->idx;
        }
    }

    return 0;
}

static void release_board_info()
{
	free_mem();
	available_board_count = 0;
	set_online_board_count(0);
	init_check_ok = 0; 
}

static void release_idx(unsigned int board_id)
{
	int n;
    vi53xx_boardid_to_idx *db;
	int idx = _get_idx(board_id);

	destroy_id(idx);

    for(n=0; n < MAX_DEV_HANDLE; n++) {
        if((db=dev_boardid_to_idx_list[n]) != NULL) {
			if (db->board_id == board_id) {
            	free(db);
            	dev_boardid_to_idx_list[n] = NULL;
			}
        }
    }

}

static void release_dev_handle(unsigned int id)
{
    int n;
    vi53xx_dev_handle *dh;

    for(n=0; n < MAX_DEV_HANDLE; n++) {
        if((dh=dev_handle_list[n]) != NULL) {
			if (dh->board_id == id) {
            	close(dh->dev_handle);
            	free(dh);
            	dev_handle_list[n] = NULL;
			}
        }
    }

}

/**
 * return :
 *   -ENODEV : no device 
 * */
int vi53xx_device_open(unsigned int board_id)
{
    vi53xx_dev_handle *dh;
    int n;
    int cnt = get_available_board_count();
	int dev_handle = get_board_handle(board_id);

    if (!cnt)
        return -ENODEV;

    if (dev_handle < 0)
        return -ENODEV;

    for(n=0; n < cnt; n++) {
        if((dh=dev_handle_list[n]) != NULL) {
            if(dh->board_id == board_id) 
                return dh->dev_handle;
        }
    }

	set_online_board_count(n+1);

    dh = malloc(sizeof(vi53xx_dev_handle)); 
    if (NULL == dh) {
        close(dev_handle);
        return -errno;
    }

	sprintf(dh->dev_name, "/dev/%s_%d", get_board_name(vi53xx_get_board_type(board_id)), get_board_instance(board_id));
    dh->dev_handle = dev_handle;
    dh->board_id   = board_id;

    for(n=0; n < cnt; n++) {
        if(__sync_bool_compare_and_swap(dev_handle_list+n, NULL, dh)) {
            return dh->dev_handle;
        }
    }

	return 0;
}

void vi53xx_dump_info()
{
    int i = 0;
    int cnt = get_available_board_count();

    for (i=0; i<cnt; i++) { 
        if (dev_handle_list[i]){
            printf("dev_handle :board_name = %s, board_handle = 0x%x,  board_id = %d\n",
                    dev_handle_list[i]->dev_name, 
                    dev_handle_list[i]->dev_handle, dev_handle_list[i]->board_id
            );
        }
    }

    for (i=0; i<cnt; i++) {
		if (board_info) {
        	printf("board_info :board_name = %s, board_type = 0x%x, board_inst = %d, board_id = %d\n",
                	board_info[i].board_name, board_info[i].board_type, 
                	board_info[i].board_inst, board_info[i].board_id
            );
		}
    }
}

void vi53xx_device_close(unsigned int board_id)
{
    int cnt = get_available_board_count();
	int dev_online = get_online_board_count();

	release_parvar_mat();

	if (!cnt || !dev_online) {
    	release_board_info();
		release_idx(board_id);
		return;
	}	

	dev_online--;
	if (!dev_online) 
    	release_board_info();

	set_online_board_count(dev_online);

	release_dev_handle(board_id);
	release_idx(board_id);
}

