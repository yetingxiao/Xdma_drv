#include "vi53xx_proc.h"
#include  "libxdma.h"
//======================================remove by ycf 2025.7.25=============================================
//#include "vi53xx_cndev_core.h"
//======================================remove by ycf 2025.7.25=============================================
//======================================add by ycf 2025.7.25=============================================
//#include "xdma_cdev.h"
//struct xdma_pci_dev  xdma_cdev is needed
#include "xdma_mod.h"
//======================================add by ycf 2025.7.25=============================================

//======================================remove by ycf 2025.7.26=============================================
//#include "board_info.h"
//======================================remove by ycf 2025.7.26=============================================

//======================================add by ycf 2025.7.26=============================================
// board_info.h is no needed by vi53xx_proc,only list const bellow is needed
#define VI53XX_DEV_NAME 	    "vi53xx"
#define VI53XX_DEV_BOARDS       "boards"
#define VI53XX_DEV_MAPING       "mapping"
//======================================add by ycf 2025.7.26=============================================
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
// 		proc/$VI53XX_DEV_NAME/$VI53XX_DEV_BOARDS文件，关联boards_fops操作的board_open函数最终调用函数
static int boards_show(struct seq_file *m, void *v)
{
	struct xdma_pci_dev *xpdev;
	struct xdma_cdev *xcdev;

	list_for_each_entry(xpdev, &pcie_device_list, list) {
//======================================add by ycf 2025.8.15=============================================
		//xcdev = &xpdev->ctrl_cdev;
		xcdev = &xpdev->vi53xx_ctrl_cdev;
//======================================add by ycf 2025.8.15=============================================
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
// 		proc/$VI53XX_DEV_NAME/$VI53XX_DEV_MAPING文件，关联mapping_fops操作mapping_open函数最终调用的函数
static int mapping_show(struct seq_file *m, void *v)
{
	struct xdma_pci_dev *xpdev;
	struct xdma_cdev *xcdev;

	list_for_each_entry(xpdev, &pcie_device_list, list) {
//======================================add by ycf 2025.8.15=============================================
		//xcdev = &xpdev->ctrl_cdev;
		xcdev = &xpdev->vi53xx_ctrl_cdev;
//======================================add by ycf 2025.8.15=============================================
		seq_printf(m, "%-10s %d:%d:0x%x\n",
						xcdev->info.device_name,  // name of board/device
						MINOR(xcdev->cdevno),    // logical instance number
						xcdev->info.board_id,
						xcdev->info.board_type
						);
	}

	return 0;
}

ssize_t mapping_write(struct file *file, const char __user *buf,
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

	seq_printf(m, "Board_id: '%d'\n",
					xcdev->info.board_id
				);

	return 0;
}

ssize_t device_write(struct file *file, const char __user *buf,
				size_t count, loff_t *ppos)
{
	int ret;
	int ch, dir;
	unsigned int offset;
	char k_buf[256] = {0};
	char cmd[255];
	int len = MIN(255, count);
	device_control_state dc;
	struct xdma_cfg_info *cfg;
	struct xdma_cdev *xcdev = to_device_file(file);
    struct device_info *info = &xcdev->info;
//======================================add by ycf 2025.8.15=============================================
	//struct xdma_pci_dev  *xpdev= container_of(xcdev, struct xdma_pci_dev, ctrl_cdev);
	struct xdma_pci_dev  *xpdev= container_of(xcdev, struct xdma_pci_dev, vi53xx_ctrl_cdev);
//======================================add by ycf 2025.8.15=============================================

	if(copy_from_user(k_buf, buf, len))
		return -EFAULT;

	k_buf[255] = 0;
	ret = sscanf(k_buf, "%s", cmd);
	if (ret != 1) {
		return -EINVAL;
	}

	if (strcmp(cmd, "on") == 0) {
		dbg_init("board_id = %d,led_blink = %s\n",info->board_id, cmd);
		dc.state = 1;
		info->device_control(info, BLINK, &dc);
	} else if (strcmp(cmd, "off") == 0) {
		dbg_init("board_id = %d,led_blink = %s\n",info->board_id, cmd);
		dc.state = 0;
		info->device_control(info, BLINK, &dc);	
	} else if (strcmp(cmd, "dump") == 0) {
		ret = sscanf(k_buf, "%s %d %d %x", cmd, &ch, &dir, &offset);
		if (ret == 4) {
			dbg_init("board_id = %d, channel:%d, dir:%s offset:0x%x\n",info->board_id, ch, dir ? "C2H":"H2C", offset);
			if ((dir != H2C_DIR) && (dir != C2H_DIR))
				return len;

			if (ch != CH1)
				return len;

			cfg = xpdev->dma_cfg[dir]; 
			dc.ch = ch;
			dc.dir = dir;
			dc.offset = offset;
			dc.pData = cfg[ch-1].pData[0];
			info->device_control(xcdev->xdev, DUMP, &dc);
		}
	}

	return count;
}

CMD_FILE_OPS(boards);
CMD_FILE_OPS(mapping);
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
//	创建 	/proc/$VI53XX_DEV_NAME/$VI53XX_DEV_MAPING文件			/proc/vi53xx/mapping
static int create_instance_board_id_mmap(void)
{
	if (!parent_dir) {
		pr_err("create dir %s fail\n", VI53XX_DEV_NAME);
		return -1;
	}

	if (!proc_create(VI53XX_DEV_MAPING,  S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH , parent_dir, &mapping_fops))	// 		proc/$VI53XX_DEV_NAME/$VI53XX_DEV_MAPING文件，关联mapping_fops操作函数
		goto remove_proc;

	return 0;

remove_proc:
	remove_proc_subtree(VI53XX_DEV_NAME, NULL);
	return -1;
}
//	创建 	proc/$VI53XX_DEV_NAME/$VI53XX_DEV_BOARDS文件			/proc/vi53xx/boards
static int create_board_proc(void)
{
	parent_dir = proc_mkdir(VI53XX_DEV_NAME, NULL);		// 		proc/$VI53XX_DEV_NAME根目录
	if (!parent_dir) {
		pr_err("create dir %s fail\n", VI53XX_DEV_NAME);
		return -1;
	}

	if (!proc_create(VI53XX_DEV_BOARDS,  S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH , parent_dir, &boards_fops))	// 		proc/$VI53XX_DEV_NAME/$VI53XX_DEV_BOARDS文件，关联boards_fops操作函数
		goto remove_proc;

	return 0;

remove_proc:
	remove_proc_subtree(VI53XX_DEV_NAME, NULL);
	return -1;
}
//	创建 	proc/$VI53XX_DEV_NAME/$VI53XX_DEV_BOARDS文件,/proc/$VI53XX_DEV_NAME/$VI53XX_DEV_MAPING文件	,/proc/vi53xx/mapping 	和 	/proc/vi53xx/boards
int create_boards_info_proc(void)
{
	int ret;

	ret = create_board_proc();
	ret = create_instance_board_id_mmap();

	return ret;
}

void  remove_board_proc(void)
{
	remove_proc_subtree(VI53XX_DEV_NAME, NULL);
}

