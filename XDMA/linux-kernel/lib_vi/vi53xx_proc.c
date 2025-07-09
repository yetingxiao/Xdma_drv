#include "vi53xx_proc.h"
#include  "libxdma.h"
#include "vi53xx_cndev_core.h"
#include "board_info.h"

static struct proc_dir_entry *parent_dir = NULL;

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#define LIMIT(a, b, c) (MAX(a, MIN(b, c)))

static struct xdma_cdev *to_device_file(struct file *file)
{
	if (!file)
		return NULL;

	if (!file->f_inode)
		return NULL;

	return PDE_DATA(file->f_inode);
}

#ifndef DEFINE_FILE_OPS
#define DEFINE_FILE_OPS(__name)					\
static int __name ## _open(struct inode *inode, struct file *file)	\
{									\
	struct xdma_cdev *device_file = to_device_file(file);   	\
	return single_open(file, __name ## _show, device_file); 	\
}									\
\
static const struct file_operations __name ## _fops = {			\
	.owner		= THIS_MODULE,					\
	.open		= __name ## _open,				\
	.write		= __name ## _write,				\
	.read		= seq_read,					\
	.llseek		= seq_lseek,					\
	.release	= single_release,				\
}
#endif /* DEFINE_FILE_OPS */
#define CMD_FILE_OPS DEFINE_FILE_OPS

static int boards_show(struct seq_file *m, void *v)
{
	struct xdma_pci_dev *xpdev;
	struct xdma_cdev *xcdev;

	list_for_each_entry(xpdev, &pcie_device_list, list) {
		xcdev = &xpdev->ctrl_cdev;
		seq_printf(m, "%-10s %d %d:%d %04x:%04x %04x:%02x:%02x.%01x %d:%d\n",
						xcdev->info.device_name,  // name of board/device
						MINOR(xcdev->cdevno),   // logical instance number
						MAJOR(xcdev->cdevno),   // major number
						MINOR(xcdev->cdevno),   // minor number
						xpdev->pdev->vendor,   // vendor ID
						xpdev->pdev->device,   // device ID
						pci_domain_nr(xpdev->pdev->bus),  // PCI domain
						xpdev->pdev->bus->number,        // bus number
						PCI_SLOT(xpdev->pdev->devfn),   // slot number
						PCI_FUNC(xpdev->pdev->devfn), // device number
						MINOR(xcdev->cdevno),    // logical instance number
						xcdev->info.board_id
						);
	}

	return 0;
}

ssize_t boards_write(struct file *file, const char __user *buf,
				size_t count, loff_t *ppos)
{
	return count;
}

static int maping_show(struct seq_file *m, void *v)
{
	struct xdma_pci_dev *xpdev;
	struct xdma_cdev *xcdev;

	list_for_each_entry(xpdev, &pcie_device_list, list) {
		xcdev = &xpdev->ctrl_cdev;
		seq_printf(m, "%-10s %d:%d:0x%x\n",
						xcdev->info.device_name,  // name of board/device
						MINOR(xcdev->cdevno),    // logical instance number
						xcdev->info.board_id,
						xcdev->info.board_type
						);
	}

	return 0;
}

ssize_t maping_write(struct file *file, const char __user *buf,
				size_t count, loff_t *ppos)
{

	return count;
}

static void hexToVersionString(unsigned int num, char versionString[])
{
	int byte2 = (num >> 16) & 0xFF;
	int byte1 = (num >> 8) & 0xFF;
	int byte0 = num & 0xFF;

	sprintf(versionString, "%d.%d.%d", byte2, byte1, byte0);
}

static int device_show(struct seq_file *m, void *v)
{
	struct xdma_cdev *xcdev = m->private;
	char versionString[10];

	seq_printf(m, "Board: '%s_%d', Serial: %x\n",
					xcdev->info.device_name,  // name of board/device
					MINOR(xcdev->cdevno),
					xcdev->info.serial
				);

	hexToVersionString(xcdev->info.fpga_version, versionString);
	seq_printf(m, "FPGA-version: '%s'\n",
					versionString
				);

	hexToVersionString(xcdev->info.mdl_version, versionString);
	seq_printf(m, "MDL-version: '%s'\n",
					versionString
			  );

	seq_printf(m, "LED-blink: '0x%x'\n",
					xcdev->info.led_blink
				);

	seq_printf(m, "Board_type: '0x%x'\n",
					xcdev->info.board_type
				);

	seq_printf(m, "Board_id: '%x'\n",
					xcdev->info.board_id
				);

	return 0;
}

ssize_t device_write(struct file *file, const char __user *buf,
				size_t count, loff_t *ppos)
{
	int ret;
	char k_buf[256] = {0};
	char cmd[255];
	int len = MIN(255, count);
	struct xdma_cdev *xcdev = to_device_file(file);
	struct xdma_dev *xdev =xcdev->xdev;
	void *reg_base = xdev->bar[0];

	if(copy_from_user(k_buf, buf, len))
		return -EFAULT;

	k_buf[255] = 0;
	ret = sscanf(k_buf, "%s", cmd);
	if (ret != 1) {
		return -EINVAL;
	}

	if (strcmp(cmd, "on") == 0) {
		dbg_init("led_blink = %s\n",cmd);
		set_led_blink_state(reg_base, DEVICE_LED_BLINK, LED_BLINK_ON);
	} else if (strcmp(cmd, "off") == 0) {
		dbg_init("led_blink = %s\n",cmd);
		set_led_blink_state(reg_base, DEVICE_LED_BLINK, LED_BLINK_OFF);
	}

	return count;
}

CMD_FILE_OPS(boards);
CMD_FILE_OPS(maping);
CMD_FILE_OPS(device);

int create_proc_device(void *device, const char *name)
{
	if (!parent_dir) {
		pr_err("create dir %s fail\n", VI53XX_DEV_NAME);
		return -1;
	}

	if (!proc_create_data(name,  S_IFREG | 0666, parent_dir, &device_fops, device))
		return -1;

	return 0;
}

int create_instance_board_id_mmap(void)
{
	if (!parent_dir) {
		pr_err("create dir %s fail\n", VI53XX_DEV_NAME);
		return -1;
	}

	if (!proc_create(VI53XX_DEV_MAPING,  S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH , parent_dir, &maping_fops))
		goto remove_proc;

	return 0;

remove_proc:
	remove_proc_subtree(VI53XX_DEV_NAME, NULL);
	return -1;
}

int create_board_proc(void)
{
	parent_dir = proc_mkdir(VI53XX_DEV_NAME, NULL);
	if (!parent_dir) {
		pr_err("create dir %s fail\n", VI53XX_DEV_NAME);
		return -1;
	}

	if (!proc_create(VI53XX_DEV_BOARDS,  S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH , parent_dir, &boards_fops))
		goto remove_proc;

	return 0;

remove_proc:
	remove_proc_subtree(VI53XX_DEV_NAME, NULL);
	return -1;
}

void  remove_board_proc(void)
{
	remove_proc_subtree(VI53XX_DEV_NAME, NULL);
}

