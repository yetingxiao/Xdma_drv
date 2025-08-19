#include <linux/hrtimer.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/timex.h>
#include <linux/tracepoint.h>
#include <trace/events/irq.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/sysctl.h>
#include <trace/events/napi.h>
#include <linux/rtc.h>
#include <linux/time.h>
#include <linux/rbtree.h>
#include <linux/cpu.h>
#include <linux/syscalls.h>
#include <linux/percpu_counter.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/sort.h>
#include <asm/irq_regs.h>
#include <asm/unistd.h>
#include <asm/cacheflush.h>
#include "vi53xx_xdma_ctrl.h"
#include "libxdma.h"

struct vi53xx_xdma_info xdma_setting;

static struct vi53xx_xdma_info *get_xdma_setting(void)
{
	return &xdma_setting;
}

static int copy_to_user__buffer(void *data, void __user *buf, size_t size)
{
	return  copy_to_user(buf, data, size);
}

static int copy_from_user__buffer(void *data, void __user *buf, size_t size)
{
	return copy_from_user(data, buf, size);
}

static int vi53xx_xdma_stop(struct xdma_dev *xdev, int channel, int dir)
{
	//pr_info("Before Vi53xx_xdma_ctrl.c vi53xx_xdma_stop \n");
	if (xdev->fops->stop)
		xdev->fops->stop(xdev, channel, dir);

    return 0;
}

static int vi53xx_xdma_reg(struct xdma_dev *xdev, struct vi53xx_xdma_info *info)
{
    int ret = 0;
	void *reg_base;
    unsigned int offset;
    unsigned int val;

    reg_base = xdev->bar[info->bar];
    offset = info->offset;
    val = info->w_val;
	//pr_info("Before Vi53xx_xdma_ctrl.c vi53xx_xdma_reg \n");
    if (info->wr == READ) {
        val  = xdev->fops->read_reg(reg_base, offset);
        //dbg_init("read bar[%d] offset[0x%x] value = [0x%x]\n",info->bar, offset, val);
        ret = copy_to_user__buffer(&val, info->r_val, sizeof(unsigned int));
    } else if (info->wr == WRITE) {
        xdev->fops->write_reg(reg_base, offset, val);
        dbg_init("write bar[%d] offset[0x%x] value = [0x%x]\n",info->bar, offset, val);
    }

    return ret;
}

static int vi53xx_xdma_start(struct xdma_dev *xdev, int channel, int dir)
{
    struct xdma_cfg_info cfg;

    if (dir == H2C_DIR) {
        cfg = xdev->H2C_dma_cfg[channel];
    } else if (dir == C2H_DIR) {
        cfg = xdev->C2H_dma_cfg[channel];
    }
	//pr_info("Before Vi53xx_xdma_ctrl.c vi53xx_xdma_start \n");
    if (xdev->fops->start_desc)
        xdev->fops->start_desc(xdev, channel, &cfg);
    if (xdev->fops->start)
        xdev->fops->start(xdev, channel, dir);

    return 0;
}

unsigned char *get_pdata(struct xdma_cfg_info *cfg)
{
	return  cfg->pData[0];
}

static struct xdma_cfg_info get_xdma_cfg(struct xdma_dev *xdev, struct vi53xx_xdma_info *info)
{
	int dir = 0;
    int channel;
    struct xdma_cfg_info cfg;

    dir = info->dir;
    channel = info->channel;

    if (dir == H2C_DIR)
        cfg = xdev->H2C_dma_cfg[channel-1];
    else
        cfg = xdev->C2H_dma_cfg[channel-1];

    return cfg;
}

static int vi53xx_xdma_exit(struct xdma_cfg_info *cfg)
{
    return 0;
}

long vi53xx_ioctl_xdma(unsigned int cmd, unsigned long arg, void *data)
{
	int ret = -EINVAL;
    int dir = 0;
    int channel;
	struct vi53xx_xdma_info xdma_info;
    struct xdma_cfg_info cfg;
	struct xdma_dev *xdev = (struct xdma_dev *)data;
    int pos = 0;
	//pr_info("Before Vi53xx_xdma_ctrl.c vi53xx_ioctl_xdma \n");
	switch (cmd) {
	case CMD_XDMA_INIT:
		ret = copy_from_user__buffer(&xdma_info, (void *)arg, sizeof(struct vi53xx_xdma_info));
        if (!ret) {
            channel = xdma_info.channel;
            cfg = xdev->H2C_dma_cfg[channel-1];
            pos = xdma_info.pos;
		    ret = copy_from_user__buffer(get_pdata(&cfg) + pos, (void *)xdma_info.buf, xdma_info.len);
        }
        break;
	case CMD_XDMA_START:
		ret = copy_from_user__buffer(&xdma_info, (void *)arg, sizeof(struct vi53xx_xdma_info));
        if (!ret) {
            dir = xdma_info.dir;
            channel = xdma_info.channel;
            ret =  vi53xx_xdma_start(xdev, channel, dir);
        }
	break;
	case CMD_XDMA_STOP:
		ret = copy_from_user__buffer(&xdma_info, (void *)arg, sizeof(struct vi53xx_xdma_info));
        if (!ret) {
            dir = xdma_info.dir;
            channel = xdma_info.channel;
            ret = vi53xx_xdma_stop(xdev, channel, dir);
        }
	break;
	case CMD_XDMA_REG:
		ret = copy_from_user__buffer(&xdma_info, (void *)arg, sizeof(struct vi53xx_xdma_info));
        if (!ret) {
            ret = vi53xx_xdma_reg(xdev, &xdma_info);
        }
	break;
	case CMD_XDMA_PRINT:
		ret = copy_from_user__buffer(&xdma_info, (void *)arg, sizeof(struct vi53xx_xdma_info));
        if (!ret) {
            pos = xdma_info.pos;
            channel = xdma_info.channel;
            cfg = xdev->C2H_dma_cfg[channel-1];
            ret = copy_to_user__buffer(get_pdata(&cfg) + pos, xdma_info.buf, xdma_info.len);
        }
        break;
	case CMD_XDMA_SETTING:
		ret = copy_from_user__buffer(&xdma_info, (void *)arg, sizeof(struct vi53xx_xdma_info));
        if (!ret) {
            xdma_setting = xdma_info;
        }
        break;
	case CMD_XDMA_EXIT:
		ret = copy_from_user__buffer(&xdma_info, (void *)arg, sizeof(struct vi53xx_xdma_info));
        if (!ret) {
            cfg = get_xdma_cfg(xdev, &xdma_info);
            vi53xx_xdma_exit(&cfg);
        }
        break;
	case CMD_XDMA_DUMP:
		ret = copy_from_user__buffer(&xdma_info, (void *)arg, sizeof(struct vi53xx_xdma_info));
        if (!ret) {
            pos = xdma_info.pos;
            channel = xdma_info.channel;
            cfg = xdev->H2C_dma_cfg[channel-1];
            ret = copy_to_user__buffer(get_pdata(&cfg) + pos, xdma_info.buf, xdma_info.len);
        }
        break;
	default:
		break;
	}

	return ret;
}

static int _xdma_mmap(struct xdma_dev *xdev, struct vi53xx_xdma_info *info, struct vm_area_struct *vma)
{
	struct xdma_cfg_info cfg = get_xdma_cfg(xdev, info);

    if (xdev->fops->xdma_mmap)
	return xdev->fops->xdma_mmap(xdev, vma, &cfg);

    return 0;
}

int _vi53xx_xdma_mmap(struct vm_area_struct *vma, void *data)
{
	struct xdma_dev *xdev = (struct xdma_dev *)data;
	//pr_info("Before Vi53xx_xdma_ctrl.c _vi53xx_xdma_mmap \n");
	return _xdma_mmap(xdev, get_xdma_setting(), vma);
}

