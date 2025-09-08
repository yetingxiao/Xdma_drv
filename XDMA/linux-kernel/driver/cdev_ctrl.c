/*
 * This file is part of the Xilinx DMA IP Core driver for Linux
 *
 * Copyright (c) 2016-present,  Xilinx, Inc.
 * All rights reserved.
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 */

#define pr_fmt(fmt)     KBUILD_MODNAME ":%s: " fmt, __func__

#include <linux/ioctl.h>
#include "version.h"
#include "xdma_cdev.h"
#include "cdev_ctrl.h"

#if ACCESS_OK_2_ARGS
#define xlx_access_ok(X, Y, Z) access_ok(Y, Z)
#else
#define xlx_access_ok(X, Y, Z) access_ok(X, Y, Z)
#endif

/*
 * character device file operations for control bus (through control bridge)
 */
static ssize_t char_ctrl_read(struct file *fp, char __user *buf, size_t count,
		loff_t *pos)
{
	struct xdma_cdev *xcdev = (struct xdma_cdev *)fp->private_data;
	struct xdma_dev *xdev;
	void __iomem *reg;
	u32 w;
	int rv;

	rv = xcdev_check(__func__, xcdev, 0);
	if (rv < 0)
		return rv;
	xdev = xcdev->xdev;

	/* only 32-bit aligned and 32-bit multiples */
	if (*pos & 3)
		return -EPROTO;
	/* first address is BAR base plus file position offset */
	reg = xdev->bar[xcdev->bar] + *pos;
	//w = read_register(reg);
	w = ioread32(reg);
	dbg_sg("%s(@%p, count=%ld, pos=%d) value = 0x%08x\n",
			__func__, reg, (long)count, (int)*pos, w);
	rv = copy_to_user(buf, &w, 4);
	if (rv)
		dbg_sg("Copy to userspace failed but continuing\n");

	*pos += 4;
	return 4;
}

static ssize_t char_ctrl_write(struct file *file, const char __user *buf,
			size_t count, loff_t *pos)
{
	struct xdma_cdev *xcdev = (struct xdma_cdev *)file->private_data;
	struct xdma_dev *xdev;
	void __iomem *reg;
	u32 w;
	int rv;

	rv = xcdev_check(__func__, xcdev, 0);
	if (rv < 0)
		return rv;
	xdev = xcdev->xdev;

	/* only 32-bit aligned and 32-bit multiples */
	if (*pos & 3)
		return -EPROTO;

	/* first address is BAR base plus file position offset 
	BAR 基址加上文件位置偏移量
	*/
	reg = xdev->bar[xcdev->bar] + *pos;
	rv = copy_from_user(&w, buf, 4);
	if (rv)
		pr_info("copy from user failed %d/4, but continuing.\n", rv);

	dbg_sg("%s(0x%08x @%p, count=%ld, pos=%d)\n",
			__func__, w, reg, (long)count, (int)*pos);
	//write_register(w, reg);
	iowrite32(w, reg);
	*pos += 4;
	return 4;
}

static long version_ioctl(struct xdma_cdev *xcdev, void __user *arg)
{
	struct xdma_ioc_info obj;
	struct xdma_dev *xdev = xcdev->xdev;
	int rv;

	rv = copy_from_user((void *)&obj, arg, sizeof(struct xdma_ioc_info));
	if (rv) {
		pr_info("copy from user failed %d/%ld.\n",
			rv, sizeof(struct xdma_ioc_info));
		return -EFAULT;
	}
	memset(&obj, 0, sizeof(obj));
	obj.vendor = xdev->pdev->vendor;
	obj.device = xdev->pdev->device;
	obj.subsystem_vendor = xdev->pdev->subsystem_vendor;
	obj.subsystem_device = xdev->pdev->subsystem_device;
	obj.feature_id = xdev->feature_id;
	obj.driver_version = DRV_MOD_VERSION_NUMBER;
	obj.domain = 0;
	obj.bus = PCI_BUS_NUM(xdev->pdev->devfn);
	obj.dev = PCI_SLOT(xdev->pdev->devfn);
	obj.func = PCI_FUNC(xdev->pdev->devfn);
	if (copy_to_user(arg, &obj, sizeof(struct xdma_ioc_info)))
		return -EFAULT;
	return 0;
}

long char_ctrl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct xdma_cdev *xcdev = (struct xdma_cdev *)filp->private_data;
	struct xdma_dev *xdev;
	struct xdma_ioc_base ioctl_obj;
	long result = 0;
	int rv;

	rv = xcdev_check(__func__, xcdev, 0);
	if (rv < 0)
		return rv;

	xdev = xcdev->xdev;
	if (!xdev) {
		pr_info("cmd %u, xdev NULL.\n", cmd);
		return -EINVAL;
	}
	pr_info("cmd 0x%x, xdev 0x%p, pdev 0x%p.\n", cmd, xdev, xdev->pdev);

	if (_IOC_TYPE(cmd) != XDMA_IOC_MAGIC) {
		pr_err("cmd %u, bad magic 0x%x/0x%x.\n",
			 cmd, _IOC_TYPE(cmd), XDMA_IOC_MAGIC);
		return -ENOTTY;
	}

	if (_IOC_DIR(cmd) & _IOC_READ)
		result = !xlx_access_ok(VERIFY_WRITE, (void __user *)arg,
				_IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		result =  !xlx_access_ok(VERIFY_READ, (void __user *)arg,
				_IOC_SIZE(cmd));

	if (result) {
		pr_err("bad access %ld.\n", result);
		return -EFAULT;
	}

	switch (cmd) {
	case XDMA_IOCINFO:
		if (copy_from_user((void *)&ioctl_obj, (void __user *) arg,
			 sizeof(struct xdma_ioc_base))) {
			pr_err("copy_from_user failed.\n");
			return -EFAULT;
		}

		if (ioctl_obj.magic != XDMA_XCL_MAGIC) {
			pr_err("magic 0x%x !=  XDMA_XCL_MAGIC (0x%x).\n",
				ioctl_obj.magic, XDMA_XCL_MAGIC);
			return -ENOTTY;
		}
		return version_ioctl(xcdev, (void __user *)arg);
	case XDMA_IOCOFFLINE:
		xdma_device_offline(xdev->pdev, xdev);
		break;
	case XDMA_IOCONLINE:
		xdma_device_online(xdev->pdev, xdev);
		break;
	default:
		pr_err("UNKNOWN ioctl cmd 0x%x.\n", cmd);
		return -ENOTTY;
	}
	return 0;
}

/* maps the PCIe BAR into user space for memory-like access using mmap() */
int bridge_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct xdma_dev *xdev;
	struct xdma_cdev *xcdev = (struct xdma_cdev *)file->private_data;
	unsigned long off;
	unsigned long phys;
	unsigned long vsize;
	unsigned long psize;
	int rv;

	rv = xcdev_check(__func__, xcdev, 0);
	if (rv < 0)
		return rv;
	xdev = xcdev->xdev;

	off = vma->vm_pgoff << PAGE_SHIFT;
	/* BAR physical address 
	pci_resource_start 和 pci_resource_end 是 PCI 设备驱动程序中非常重要的两个函数
	它们用来获取特定 基地址寄存器 (BAR) 的物理起始和结束地址。这些地址对于驱动程序访问设备的内存映射 I/O (MMIO) 区域或 I/O 端口至关重要。
	*/
	phys = pci_resource_start(xdev->pdev, xcdev->bar) + off;
	vsize = vma->vm_end - vma->vm_start;
	/* complete resource */
	psize = pci_resource_end(xdev->pdev, xcdev->bar) -
		pci_resource_start(xdev->pdev, xcdev->bar) + 1 - off;

	dbg_sg("mmap(): xcdev = 0x%08lx\n", (unsigned long)xcdev);
	dbg_sg("mmap(): cdev->bar = %d\n", xcdev->bar);
	dbg_sg("mmap(): xdev = 0x%p\n", xdev);
	dbg_sg("mmap(): pci_dev = 0x%08lx\n", (unsigned long)xdev->pdev);
	dbg_sg("off = 0x%lx, vsize 0x%lu, psize 0x%lu.\n", off, vsize, psize);
	dbg_sg("start = 0x%llx\n",
		(unsigned long long)pci_resource_start(xdev->pdev,
		xcdev->bar));
	dbg_sg("phys = 0x%lx\n", phys);

	if (vsize > psize)
		return -EINVAL;
	/*
	 * pages must not be cached as this would result in cache line sized
	 * accesses to the end point
	 */
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	/*
	 * prevent touching the pages (byte access) for swap-in,
	 * and prevent the pages from being swapped out
	 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
        vm_flags_set(vma, VMEM_FLAGS);
#elif defined(RHEL_RELEASE_CODE)
	#if (RHEL_RELEASE_CODE > RHEL_RELEASE_VERSION(9, 4))
        vm_flags_set(vma, VMEM_FLAGS);
	#else
	vma->vm_flags |= VMEM_FLAGS;
	#endif
#else
	vma->vm_flags |= VMEM_FLAGS;
#endif
	/* make MMIO accessible to user space 
	io_remap_pfn_range 是一个 Linux 内核函数，用于将物理内存区域映射到进程的用户空间虚拟内存。这通常用于让用户空间的应用程序可以直接访问内存映射 I/O (MMIO) 的硬件寄存器或设备内存。
	int io_remap_pfn_range(struct vm_area_struct *vma,
                       unsigned long addr,
                       unsigned long pfn,
                       unsigned long size,
                       pgprot_t prot);

vma: 指向 vm_area_struct 的指针，该结构描述了将要被映射的虚拟内存区域。它包含了用户空间虚拟地址范围的信息。
addr: 用户空间中映射的起始虚拟地址。
pfn: 物理页帧号（Physical Page Frame Number）。这是将要被映射的起始物理地址（以页为单位，而不是字节）。它通常是通过将物理地址右移 PAGE_SHIFT 位来得到的。
size: 要映射的内存区域的大小，单位是字节。
prot: 页保护标志，用于定义映射内存的权限（例如，读、写、可执行、可缓存性等）。
	*/
	rv = io_remap_pfn_range(vma, vma->vm_start, phys >> PAGE_SHIFT,
			vsize, vma->vm_page_prot);
	dbg_sg("vma=0x%p, vma->vm_start=0x%lx, phys=0x%lx, size=%lu = %d\n",
		vma, vma->vm_start, phys >> PAGE_SHIFT, vsize, rv);

	if (rv)
		return -EAGAIN;
	return 0;
}

/*
 * character device file operations for control bus (through control bridge)
 */
static const struct file_operations ctrl_fops = {
	.owner = THIS_MODULE,
	.open = char_open,
	.release = char_close,
	.read = char_ctrl_read,
	.write = char_ctrl_write,
	.mmap = bridge_mmap,
	.unlocked_ioctl = char_ctrl_ioctl,
};

void cdev_ctrl_init(struct xdma_cdev *xcdev)
{
	cdev_init(&xcdev->cdev, &ctrl_fops);
}
