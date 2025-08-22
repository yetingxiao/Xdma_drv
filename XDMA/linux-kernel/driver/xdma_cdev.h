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

#ifndef __XDMA_CHRDEV_H__
#define __XDMA_CHRDEV_H__

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include "xdma_mod.h"

#define XDMA_NODE_NAME	"xdma"
#define XDMA_MINOR_BASE (0)
#define XDMA_MINOR_COUNT (255)

enum xpdev_flags_bits {
	XDF_CDEV_USER,
	XDF_CDEV_CTRL,
	XDF_CDEV_XVC,
	XDF_CDEV_EVENT,
	XDF_CDEV_SG,
	XDF_CDEV_BYPASS,
//======================================add by ycf 2025.8.12=============================================
	XDF_CDEV_CHAR_CTRL,
//======================================add by ycf 2025.8.12=============================================
};

enum cdev_type {
	CHAR_USER,
	CHAR_CTRL,
	CHAR_XVC,
	CHAR_EVENTS,
	CHAR_XDMA_H2C,
	CHAR_XDMA_C2H,
	CHAR_BYPASS_H2C,
	CHAR_BYPASS_C2H,
	CHAR_BYPASS,
//======================================add by ycf 2025.8.12=============================================
	CDEV_CTRL,
//======================================add by ycf 2025.8.12=============================================
};
extern struct es_cdev *es_cdevPtr;

void xdma_cdev_cleanup(void);
int xdma_cdev_init(void);

int char_open(struct inode *inode, struct file *file);
int char_close(struct inode *inode, struct file *file);
int xcdev_check(const char *fname, struct xdma_cdev *xcdev, bool check_engine);
void cdev_ctrl_init(struct xdma_cdev *xcdev);
void cdev_xvc_init(struct xdma_cdev *xcdev);
void cdev_event_init(struct xdma_cdev *xcdev);
void cdev_sgdma_init(struct xdma_cdev *xcdev);
void cdev_bypass_init(struct xdma_cdev *xcdev);
long char_ctrl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

void xpdev_destroy_interfaces(struct xdma_pci_dev *xpdev);
int xpdev_create_interfaces(struct xdma_pci_dev *xpdev);

int bridge_mmap(struct file *file, struct vm_area_struct *vma);


void vi53xx_cdev_exit(void);
int vi53xx_cdev_init(void);
extern int vi53xx_dev_init(struct xdma_pci_dev *xpdev);
extern void  vi53xx_dev_clean(struct xdma_pci_dev *xpdev);
#endif /* __XDMA_CHRDEV_H__ */
