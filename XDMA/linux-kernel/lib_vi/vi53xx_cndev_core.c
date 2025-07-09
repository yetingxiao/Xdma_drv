#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/fb.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include "vi53xx_xdma_ctrl.h"
#include "vi53xx_cndev_core.h"
#include "board_info.h"
#include "vi53xx_proc.h"

struct es_cdev *es_cdev;
static const struct file_operations vi53xx_xdma_fops;
static char vi53xx_minorBitTable[KDEVICE_MINOR_COUNT/sizeof(char)];
static DEFINE_SEMAPHORE(_vi53xx_mutex);

static void _vi53xx_lock(void)
{
    down(&_vi53xx_mutex);
}

static void _vi53xx_unlock(void)
{
    up(&_vi53xx_mutex);
}
static int vi53xx_minorUsed(int i)
{
    char *p=&vi53xx_minorBitTable[i/sizeof(char)];
    return (*p & (1 << (i % sizeof(char)))) != 0;       /* 0 or 1 */
}

static void vi53xx_setMinor(int i)
{
    char *p = &vi53xx_minorBitTable[i/sizeof(char)];
    *p |= 1 << (i % sizeof(char));                      /* set bit */
}

static void vi53xx_resetMinor(int i)
{
    char *p = &vi53xx_minorBitTable[i/sizeof(char)];
    *p &= ~(1 << (i % sizeof(char)));                   /* clear bit */
}

static int vi53xx_getDeviceNumber(int major, dev_t *devnumber, int inst)
{
    int n, status = 0;
    int Major = major;

    _vi53xx_lock();

    for(n=0; n < KDEVICE_MINOR_COUNT; n++) {
        if (!vi53xx_minorUsed(inst)) {
		    vi53xx_setMinor(inst);
            break;
        }
    }

    if (n == KDEVICE_MINOR_COUNT) {
		status = -ENODEV;
    } else {
		*devnumber = MKDEV(Major, inst);
    }

    _vi53xx_unlock();

    return status;
}

static dev_t get_devnumer(int major, int inst)
{
	dev_t devnumber = 0;

	if (vi53xx_getDeviceNumber(major, &devnumber, inst) < 0) {
		vi53xx_resetMinor(MINOR(devnumber));
	}

	return devnumber;
}

static inline void xpdev_flag_set(struct xdma_pci_dev *xpdev,enum flags_bits fbit)
{
	xpdev->flags |= 1 << fbit;
}

static inline void xcdev_flag_clear(struct xdma_pci_dev *xpdev,	enum flags_bits fbit)
{
	xpdev->flags &= ~(1 << fbit);
}

static inline int xpdev_flag_test(struct xdma_pci_dev *xpdev, enum flags_bits fbit)
{
	return xpdev->flags & (1 << fbit);
}

static long vi53xx_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = -EINVAL;
	int type, nr;

    struct xdma_cdev *xcdev = (struct xdma_cdev *)filp->private_data;

    type = _IOC_TYPE(cmd);
	nr = _IOC_NR(cmd);

	switch (type) {
	case RTPC_IOCTL_TYPE_XDMA:
	    ret = vi53xx_ioctl_xdma(nr, arg, xcdev->xdev);
	    break;
	default:
	    break;
	}

	return ret;
}

static int vi53xx_open(struct inode *inode, struct file *file)
{
    struct xdma_cdev *xcdev;

	/* pointer to containing structure of the character device inode */
	xcdev = container_of(inode->i_cdev, struct xdma_cdev, cdev);
	if (xcdev->magic != MAGIC_CHAR) {
		pr_err("xcdev 0x%p inode 0x%lx magic mismatch 0x%lx\n",
			    xcdev, inode->i_ino, xcdev->magic);
		return -EINVAL;
	}

	file->f_op = &vi53xx_xdma_fops;
	/* create a reference to our char device in the opened file */
	file->private_data = xcdev;

	return 0;
}

static int vi53xx_xdma_mmap(struct file *filp, struct vm_area_struct *vma)
{
    struct xdma_cdev *xcdev = (struct xdma_cdev *)filp->private_data;

	return _vi53xx_xdma_mmap(vma, xcdev->xdev);
}

static const struct file_operations vi53xx_xdma_fops = {
    .open       = vi53xx_open,
    .unlocked_ioctl = vi53xx_ioctl,
    .mmap = vi53xx_xdma_mmap,
};

static int create_sys_device(struct xdma_cdev *xcdev)
{
	xcdev->sys_device = device_create(es_cdev->cdev_class, NULL,
		xcdev->cdevno, NULL, es_cdev->device_name);

	if (!xcdev->sys_device) {
		pr_err("device_create failed\n");
		return -1;
	}

	return 0;
}

static int create_cdev(struct xdma_pci_dev *xpdev, struct xdma_cdev *xcdev, enum cdev_type type)
{
	int rv;
	int minor;
	struct xdma_dev *xdev = xpdev->xdev;
    struct device_info *info = &xcdev->info;
    uint32_t instance;
	void *reg_base = xdev->bar[0];
    uint32_t inca_dt = get_board_inca_dt(reg_base, DEVICE_BOARD_TYPE);;

	xcdev->magic = MAGIC_CHAR;
	xcdev->cdev.owner = THIS_MODULE;
	xcdev->xpdev = xpdev;
	xcdev->xdev = xdev;

    dbg_init("board type       = 0x%x\n", inca_dt);

    if ((instance = get_board_instance(inca_dt)) < 0) {
        pr_info("unknow inca_dt\n");
	    return -EINVAL;
    }

    if (!get_board_name(inca_dt)) {
        pr_info("unknow board name\n");
	    return -EINVAL;
    }

	xcdev->cdevno = get_devnumer(es_cdev->major, instance);
	es_cdev->board_inst = MINOR(xcdev->cdevno);
	info->board_inst = es_cdev->board_inst;
	init_device_info(info, reg_base);
    dbg_init("board id         = 0x%x\n", info->board_id);
    dbg_init("board instance   = 0x%x\n", info->board_inst);
    dbg_init("board name       = %s  \n", info->device_name);
    dbg_init("mdl_version      = 0x%x\n", info->mdl_version);
    dbg_init("fpga_version     = 0x%x\n", info->fpga_version);
    dbg_init("board_type       = 0x%x\n", info->board_type);
    dbg_init("board serial     = 0x%x\n", info->serial);
    dbg_init("board led_blink  = 0x%x\n", info->led_blink);

	sprintf(es_cdev->device_name, "%s_%d", get_board_name(inca_dt), es_cdev->board_inst);

    rv = kobject_set_name(&xcdev->cdev.kobj, es_cdev->device_name);
	if (rv < 0) {
		vi53xx_resetMinor(MINOR(xcdev->cdevno));
		return rv;
	}

	switch (type) {
	case CHAR_CTRL:
		minor = type;
        cdev_init(&xcdev->cdev, &vi53xx_xdma_fops);
		break;
	default:
		pr_info("type 0x%x NOT supported.\n", type);
		vi53xx_resetMinor(MINOR(xcdev->cdevno));
		return -EINVAL;
	}

	/* bring character device live */
	rv = cdev_add(&xcdev->cdev, xcdev->cdevno, 1);
	if (rv < 0) {
		vi53xx_resetMinor(MINOR(xcdev->cdevno));
		pr_err("cdev_add failed %d, type 0x%x.\n", rv, type);
		return -EINVAL;
	}

    if (es_cdev->cdev_class) {
        rv = create_sys_device(xcdev);
        if (rv < 0)
            goto del_cdev;
    }

    create_proc_device(xcdev, es_cdev->device_name);

	return 0;

del_cdev:
	vi53xx_resetMinor(MINOR(xcdev->cdevno));
        cdev_del(&xcdev->cdev);
	return rv;
}

static int destroy_xcdev(struct xdma_cdev *cdev)
{
	if (!cdev) {
		pr_warn("cdev NULL.\n");
		return -EINVAL;
	}
	if (cdev->magic != MAGIC_CHAR) {
		pr_warn("cdev 0x%p magic mismatch 0x%lx\n", cdev, cdev->magic);
		return -EINVAL;
	}

	if (!cdev->xdev) {
		pr_err("xdev NULL\n");
		return -EINVAL;
	}

    if (!cdev->sys_device) {
        pr_err("cdev sys_device NULL\n");
        return -EINVAL;
    }

	vi53xx_resetMinor(MINOR(cdev->cdevno));

    if (cdev->sys_device)
        device_destroy(es_cdev->cdev_class, cdev->cdevno);

    cdev_del(&cdev->cdev);

	return 0;
}

static void destroy_cdev(struct xdma_pci_dev *xpdev)
{
	int rv;

	if (xpdev_flag_test(xpdev, CDEV_CTRL)) {
        rv = destroy_xcdev(&xpdev->ctrl_cdev);
        if (rv < 0)
            pr_err("Failed to destroy cdev error 0x%x\n", rv);
	}
}

void vi53xx_cdev_exit(void)
{
    if (es_cdev->cdev_class)
		class_destroy(es_cdev->cdev_class);

	if (es_cdev->major)
            unregister_chrdev_region(MKDEV(es_cdev->major, 0), 255);

	if (es_cdev) {
		kfree(es_cdev);
		es_cdev = NULL;
	}
}

int vi53xx_cdev_init(void)
{
	es_cdev  = kmalloc(sizeof(*es_cdev), GFP_KERNEL);
	if (!es_cdev)
		return -1;

    es_cdev->cdev_class = class_create(THIS_MODULE, VI53XX_DEV_NAME);
    if (IS_ERR(es_cdev->cdev_class)) {
        dbg_init(VI53XX_DEV_NAME ": failed to create class");
        return -1;
    }

	if (!es_cdev->major) {
		/* allocate a dynamically allocated char device node */
		int rv = alloc_chrdev_region(&es_cdev->dev, 0, 255, VI53XX_DEV_NAME);

		if (rv) {
			pr_err("unable to allocate cdev region %d.\n", rv);
			return rv;
		}
		es_cdev->major = MAJOR(es_cdev->dev);
	}

	memset(vi53xx_minorBitTable, (char) 0, sizeof(vi53xx_minorBitTable));

    return 0;
}

int vi53xx_dev_init(struct xdma_pci_dev *xpdev)
{
    int ret = 0;

    ret = create_cdev(xpdev, &xpdev->ctrl_cdev, CHAR_CTRL);
    if (ret < 0) {
        pr_err("create_char(vi53xx_xmda) failed\n");
        goto fail;
    }

    xpdev_flag_set(xpdev, CDEV_CTRL);

    return 0;
fail:
	ret = -1;
	destroy_cdev(xpdev);
	return ret;
}

void  vi53xx_dev_clean(struct xdma_pci_dev *xpdev)
{
    destroy_cdev(xpdev);
}

