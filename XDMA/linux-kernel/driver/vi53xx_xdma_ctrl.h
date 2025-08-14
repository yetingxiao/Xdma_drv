
#ifndef RTPC_XDMA_CTRL_H
#define RTPC_XDMA_CTRL_H

#include <linux/ioctl.h>

#define RTPC_READ 0
#define RTPC_WRITE 1

#define BAR0 0x0
#define BAR1 0x1

#define MAGIC_CHAR      0xCCCCCCUL
extern struct es_cdev *es_cdev;
long vi53xx_ioctl_xdma(unsigned int cmd, unsigned long arg, void *data);
int _vi53xx_xdma_mmap(struct vm_area_struct *vma, void *data);

struct vi53xx_xdma_info {
    /*reg info*/
    int __user bar;
    unsigned int offset;
    int __user wr;
    unsigned int __user w_val;
    unsigned int __user *r_val;

    /*xdma info*/
    unsigned char __user dir;
    int __user channel;                   /* 0-host2card 1-card2host */
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

#endif
