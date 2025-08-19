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
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/aer.h>
/* include early, to verify it depends only on the headers above */
#include "libxdma_api.h"
#include "libxdma.h"
#include "xdma_mod.h"
#include "xdma_cdev.h"
#include "version.h"
//======================================add by ycf 2025.8.4=============================================
#include "xuart_cdev.h"
//======================================add by ycf 2025.8.4=============================================
//======================================add by ycf 2025.8.12=============================================
#include "board_info.h"

/* 
int vi53xx_debug = 0;
module_param(vi53xx_debug, int, 0644);
MODULE_PARM_DESC(vi53xx_debug, "Activate debugging for vi53xx module (default:0=off)."); 
*/


#define DRV_MODULE_NAME		    "vi53xx"
#define DRV_MODULE_DESC	        "driver of es53xx designed by vehinfo(VI)"
#define RTPC_XDMA_VID           0x10ee
#define RTPC_XDMA_DID           0x16F2
//======================================add by ycf 2025.8.12=============================================
#include "xparameters.h"
//#define DRV_MODULE_NAME		"xdma"
//#define DRV_MODULE_DESC		"Xilinx XDMA Reference Driver"

static char version[] =
	DRV_MODULE_DESC " " DRV_MODULE_NAME " v" DRV_MODULE_VERSION "\n";

MODULE_AUTHOR("Xilinx, Inc.");
MODULE_DESCRIPTION(DRV_MODULE_DESC);
MODULE_VERSION(DRV_MODULE_VERSION);
MODULE_LICENSE("Dual BSD/GPL");

//======================================add by ycf 2025.7.27=============================================
extern const struct  pci_info vi53xx_info;
static DEFINE_SEMAPHORE(vi53xx_mutex);
static void vi53xx_lock(void)
{
    down(&vi53xx_mutex);
}

static void vi53xx_unlock(void)
{
    up(&vi53xx_mutex);
}
//======================================add by ycf 2025.7.27=============================================

//======================================add by ycf 2025.8.12=============================================

void *xdma_device_map(const char *mname, struct pci_dev *pdev)
{//类似功能void *xdma_device_open(const char *mname, struct pci_dev *pdev, int *user_max,int *h2c_channel_max, int *c2h_channel_max)
	struct xdma_dev *xdev = NULL;
	int rv = 0;

	/* allocate zeroed device book keeping structure */
	xdev = alloc_dev_instance(pdev);
	if (!xdev)
		return NULL;

	xdev->mod_name = mname;

    platform_setup_fops(xdev);

	rv = pci_enable_device(pdev);
	if (rv) {
		dbg_init("pci_enable_device() failed, %d.\n", rv);
		goto free_xdev;
	}

	/* enable bus master capability */
	pci_set_master(pdev);

	rv = request_regions(xdev, pdev);
	if (rv)
		goto err_regions;

	rv = map_bars(xdev, pdev);
	if (rv)
		goto err_map;

	rv = set_dma_mask(pdev);
	if (rv)
		goto err_mask;

	return (void *)xdev;

err_mask:
	unmap_bars(xdev, pdev);
err_map:
	if (xdev->got_regions)
		pci_release_regions(pdev);
err_regions:
	if (!xdev->regions_in_use)
		pci_disable_device(pdev);
free_xdev:
	kfree(xdev);

	return NULL;
}

void xdma_device_umap(struct pci_dev *pdev, void *dev_xdev)
{//功能类似void xdma_device_close(struct pci_dev *pdev, void *dev_hndl)
	struct xdma_dev *xdev = (struct xdma_dev *)dev_xdev;

	dbg_init("pdev 0x%p, xdev 0x%p.\n", pdev, dev_xdev);

	if (!dev_xdev)
            return;

	unmap_bars(xdev, pdev);

	if (xdev->got_regions) {
        dbg_init("pci_release_regions 0x%p.\n", pdev);
	    pci_release_regions(pdev);
	}

	if (!xdev->regions_in_use) {
        dbg_init("pci_disable_device 0x%p.\n", pdev);
	    pci_disable_device(pdev);
	}

	kfree(xdev);
}

static int _xdma_channel_init(struct xdma_dev *xdev, struct xdma_cfg_info *cfg, struct pci_info *vi53xx_xdma_info)
{
    vi53xx_xdma_info->setup(xdev->pdev, cfg);

    xdev->fops->poll_init(xdev, cfg->channel, cfg->direction);
    xdev->fops->stop(xdev, cfg->channel, cfg->direction);
    xdev->fops->create_transfer(xdev, cfg->channel, cfg);

    return 0;
}

static void  init_dma_cfg(struct xdma_cfg_info **dma_cfg, int direction, int channel)
{
    dma_cfg[direction][channel].direction  = direction;
    dma_cfg[direction][channel].channel = channel +1;
}

static int xdma_channel_init(struct xdma_pci_dev *xpdev, struct pci_info *info)
{
    int channel = 0;
    int direction;
    struct xdma_dev *xdev = xpdev->xdev;
    struct xdma_cfg_info **dma_cfg = xpdev->dma_cfg;

    if (!info)
        return -1;

    for (channel = 0; channel < XDMA_CHANNEL_NUM; channel++) { /*use CH1 CH2 --> XDMA_CHANNEL_NUM*/
        /* C2H */
        direction = C2H_DIR;
        init_dma_cfg(dma_cfg, direction, channel);
        _xdma_channel_init(xdev, &dma_cfg[direction][channel], info);

	    /* H2C */
        direction = H2C_DIR;
        init_dma_cfg(dma_cfg, direction, channel);
        if (dma_cfg[direction][channel].channel == CH1)
            _xdma_channel_init(xdev, &dma_cfg[direction][channel], info);
    }

    return 0;
}

static void dump_vi53xx_device_info(struct pci_dev *pdev)
{
	pr_info("0x%p ,  device found - pci bus: %d - id: 0x%04x - devfn: 0x%04x - pcie_type: 0x%04x.\n",
		    pdev, pdev->bus->number, pdev->device, pdev->devfn, pci_pcie_type(pdev));
}
//======================================add by ycf 2025.8.12=============================================

/* SECTION: Module global variables */
static int xpdev_cnt;
//
struct pci_device_id pci_ids[] = {
	//vender/device id确认使用xilinx 0x10ee, 0x16f2 ,0x16f2 es5311  或者PCI_DEVICE(RTPC_XDMA_VID, RTPC_XDMA_DID)
    { PCI_DEVICE(0x10ee, 0x16f2),.driver_data = (kernel_ulong_t) &vi53xx_info },
	//vender/device id确认使用xilinx 0x10ee, 0x16f3 ,0x16f3 es5341
	{ PCI_DEVICE(0x10ee, 0x16f3), },
	{ PCI_DEVICE(0x10ee, 0x9048), },
	{ PCI_DEVICE(0x10ee, 0x9044), },
	{ PCI_DEVICE(0x10ee, 0x9042), },
	{ PCI_DEVICE(0x10ee, 0x9041), },
	{ PCI_DEVICE(0x10ee, 0x903f), },
	{ PCI_DEVICE(0x10ee, 0x9038), },
	{ PCI_DEVICE(0x10ee, 0x9028), },
	{ PCI_DEVICE(0x10ee, 0x9018), },
	{ PCI_DEVICE(0x10ee, 0x9034), },
	{ PCI_DEVICE(0x10ee, 0x9024), },
	{ PCI_DEVICE(0x10ee, 0x9014), },
	{ PCI_DEVICE(0x10ee, 0x9032), },
	{ PCI_DEVICE(0x10ee, 0x9022), },
	{ PCI_DEVICE(0x10ee, 0x9012), },
	{ PCI_DEVICE(0x10ee, 0x9031), },
	{ PCI_DEVICE(0x10ee, 0x9021), },
	{ PCI_DEVICE(0x10ee, 0x9011), },

	{ PCI_DEVICE(0x10ee, 0x8011), },
	{ PCI_DEVICE(0x10ee, 0x8012), },
	{ PCI_DEVICE(0x10ee, 0x8014), },
	{ PCI_DEVICE(0x10ee, 0x8018), },
	{ PCI_DEVICE(0x10ee, 0x8021), },
	{ PCI_DEVICE(0x10ee, 0x8022), },
	{ PCI_DEVICE(0x10ee, 0x8024), },
	{ PCI_DEVICE(0x10ee, 0x8028), },
	{ PCI_DEVICE(0x10ee, 0x8031), },
	{ PCI_DEVICE(0x10ee, 0x8032), },
	{ PCI_DEVICE(0x10ee, 0x8034), },
	{ PCI_DEVICE(0x10ee, 0x8038), },

	{ PCI_DEVICE(0x10ee, 0x7011), },
	{ PCI_DEVICE(0x10ee, 0x7012), },
	{ PCI_DEVICE(0x10ee, 0x7014), },
	{ PCI_DEVICE(0x10ee, 0x7018), },
	{ PCI_DEVICE(0x10ee, 0x7021), },
	{ PCI_DEVICE(0x10ee, 0x7022), },
	{ PCI_DEVICE(0x10ee, 0x7024), },
	{ PCI_DEVICE(0x10ee, 0x7028), },
	{ PCI_DEVICE(0x10ee, 0x7031), },
	{ PCI_DEVICE(0x10ee, 0x7032), },
	{ PCI_DEVICE(0x10ee, 0x7034), },
	{ PCI_DEVICE(0x10ee, 0x7038), },

	{ PCI_DEVICE(0x10ee, 0x6828), },
	{ PCI_DEVICE(0x10ee, 0x6830), },
	{ PCI_DEVICE(0x10ee, 0x6928), },
	{ PCI_DEVICE(0x10ee, 0x6930), },
	{ PCI_DEVICE(0x10ee, 0x6A28), },
	{ PCI_DEVICE(0x10ee, 0x6A30), },
	{ PCI_DEVICE(0x10ee, 0x6D30), },

	{ PCI_DEVICE(0x10ee, 0x4808), },
	{ PCI_DEVICE(0x10ee, 0x4828), },
	{ PCI_DEVICE(0x10ee, 0x4908), },
	{ PCI_DEVICE(0x10ee, 0x4A28), },
	{ PCI_DEVICE(0x10ee, 0x4B28), },

	{ PCI_DEVICE(0x10ee, 0x2808), },
#ifdef INTERNAL_TESTING
	{ PCI_DEVICE(0x1d0f, 0x1042), 0},
#endif
	/* aws */
	{ PCI_DEVICE(0x1d0f, 0xf000), },
	{ PCI_DEVICE(0x1d0f, 0xf001), },
	{0,}
};
//int id_table_size=sizeof(pci_ids) / sizeof(pci_ids[0]);
MODULE_DEVICE_TABLE(pci, pci_ids);

static void xpdev_free(struct xdma_pci_dev *xpdev)
{
	struct xdma_dev *xdev = xpdev->xdev;

	pr_info("xpdev 0x%p, destroy_interfaces, xdev 0x%p.\n", xpdev, xdev);
	xpdev_destroy_interfaces(xpdev);
	xpdev->xdev = NULL;
	pr_info("xpdev 0x%p, xdev 0x%p xdma_device_close.\n", xpdev, xdev);
	xdma_device_close(xpdev->pdev, xdev);
	xpdev_cnt--;

	kfree(xpdev);
}

static struct xdma_pci_dev *xpdev_alloc(struct pci_dev *pdev)
{
	struct xdma_pci_dev *xpdev = kmalloc(sizeof(*xpdev), GFP_KERNEL);

	if (!xpdev)
		return NULL;
	memset(xpdev, 0, sizeof(*xpdev));

	xpdev->magic = MAGIC_DEVICE;
	xpdev->pdev = pdev;
	xpdev->user_max = MAX_USER_IRQ;
	xpdev->h2c_channel_max = XDMA_CHANNEL_NUM_MAX;
	xpdev->c2h_channel_max = XDMA_CHANNEL_NUM_MAX;
//======================================add by ycf 2025.8.13=============================================	
	xpdev->bus = pdev->bus->number;
//======================================add by ycf 2025.8.13=============================================
	xpdev_cnt++;
	return xpdev;
}

static int probe_one(struct pci_dev *pdev, const struct pci_device_id *id)
{
	int rv = 0;
	int ret=0;
	struct xdma_pci_dev *xpdev = NULL;
	struct xdma_dev *xdev;
	void *hndl;

	xpdev = xpdev_alloc(pdev);
	if (!xpdev)
		return -ENOMEM;


//======================================add by ycf 2025.8.13=============================================
    struct pci_info *vi53xx_xdma_info = (struct pci_info *)(id->driver_data);
    dump_vi53xx_device_info(pdev);//pdev->device 0x16f2区分板卡
	//xdev = xdma_device_map(DRV_MODULE_NAME, pdev);功能等同xdma_device_open
//======================================add by ycf 2025.8.13=============================================
	hndl = xdma_device_open(DRV_MODULE_NAME, pdev, &xpdev->user_max,&xpdev->h2c_channel_max, &xpdev->c2h_channel_max);
	//hndl->fops = &rtpc_xdma_fops;
	if (!hndl) {
		pr_warn("xdma_device_open Failed!\n");
		rv = -EINVAL;
		goto err_out;
	}
//======================================add by ycf 2025.8.13=============================================
	xpdev->xdev = hndl;
    xpdev->dma_cfg[H2C_DIR] = ((struct xdma_dev *)hndl)->H2C_dma_cfg;
    xpdev->dma_cfg[C2H_DIR] = ((struct xdma_dev *)hndl)->C2H_dma_cfg;

    if (xdma_channel_init(xpdev, vi53xx_xdma_info)) {
	    rv = -EINVAL;
        goto err_out;
    }
//======================================add by ycf 2025.8.13=============================================

	if (xpdev->user_max > MAX_USER_IRQ) {
		pr_err("Maximum users limit reached\n");
		rv = -EINVAL;
		goto err_out;
	}

	if (xpdev->h2c_channel_max > XDMA_CHANNEL_NUM_MAX) {
		pr_err("Maximun H2C channel limit reached\n");
		rv = -EINVAL;
		goto err_out;
	}

	if (xpdev->c2h_channel_max > XDMA_CHANNEL_NUM_MAX) {
		pr_err("Maximun C2H channel limit reached\n");
		rv = -EINVAL;
		goto err_out;
	}

	if (!xpdev->h2c_channel_max && !xpdev->c2h_channel_max)
		pr_warn("NO engine found!\n");

	/* make sure no duplicate */
	xdev = xdev_find_by_pdev(pdev);
	if (!xdev) {
		pr_warn("NO xdev found!\n");
		rv =  -EINVAL;
		goto err_out;
	}

	if (hndl != xdev) {
		pr_err("xdev handle mismatch\n");
		rv =  -EINVAL;
		goto err_out;
	}

	pr_info("%s xdma%d, pdev 0x%p, xdev 0x%p, 0x%p, usr %d, ch %d,%d.\n",
		dev_name(&pdev->dev), xdev->idx, pdev, xpdev, xdev,
		xpdev->user_max, xpdev->h2c_channel_max,
		xpdev->c2h_channel_max);

	xpdev->xdev = hndl;

	rv = xpdev_create_interfaces(xpdev);
	if (rv)
		goto err_out;

	dev_set_drvdata(&pdev->dev, xpdev);
	
	if (xpdev->user_max) {
		u32 mask = (1 << (xpdev->user_max + 1)) - 1;
		rv = xdma_user_isr_enable(hndl, mask);
		if (rv)
		{
			pr_err("Failed to Enable user interrupt\n");
			goto err_out;
		}
		//======================================add by ycf 2025.8.4=============================================
		/*
		 * xdma_user_isr_register - register a user ISR handler
		 * It is expected that the xdma will register the ISR, and for the user
		 * interrupt, it will call the corresponding handle if it is registered and
		 * enabled.
		 *
		 * @pdev: ptr to the the pci_dev struct	
		 * @mask: bitmask of user interrupts (0 ~ 15)to be registered
		 *		bit 0: user interrupt 0
		 *		...
		 *		bit 15: user interrupt 15
		 *		any bit above bit 15 will be ignored.
		 * @handler: the correspoinding handler
		 *		a NULL handler will be treated as de-registeration
		 * @name: to be passed to the handler, ignored if handler is NULL`
		 * @dev: to be passed to the handler, ignored if handler is NULL`
		 * return < 0 in case of error
		 * TODO: exact error code will be defined later
		 */
		printk(KERN_INFO "register user interrupt \n");	
		//XUartNs550 *UartInstancePtr=kmalloc(sizeof(XUartNs550),GFP_KERNEL);
		XUartNs550 *UartInstancePtr=&UartNs550Instance;
		if (UartInstancePtr == NULL) {
			pr_err("Failed to allocate memory for UartInstancePtr\n");
			return -ENOMEM;
		}
		ret=xdma_user_isr_register(xpdev->xdev,mask,(irq_handler_t)XUartNs550_KernelIntHandlerEntry,UartInstancePtr);
		if (ret < 0) {
			pr_err("Failed to register XDMA user ISR\n");
			return ret;
		}
		pr_info("BAR%d mapped at 0x%p\n", 0,xdev->bar[0]);
		UART_KERNEL_REGS=xdev->bar[0];
		IS_KERNEL_MAPPED=1;
		//======================================add by ycf 2025.8.4=============================================
	}
	
	
	
//======================================add by ycf 2025.7.27=============================================
	vi53xx_lock();
	list_add(&xpdev->list, &pcie_device_list);
	vi53xx_unlock();
//======================================add by ycf 2025.7.27=============================================	

//======================================add by ycf 2025.8.7=============================================	
	printk(KERN_INFO "AXIUart: Before Cdev Initializing\n");	
	AXIUart_cdev_init();
//======================================add by ycf 2025.8.7=============================================

	return 0;

err_out:
	pr_err("pdev 0x%p, err %d.\n", pdev, rv);
	xpdev_free(xpdev);
	return rv;
}

static void remove_one(struct pci_dev *pdev)
{
	struct xdma_pci_dev *xpdev;

	if (!pdev)
		return;

	xpdev = dev_get_drvdata(&pdev->dev);
	if (!xpdev)
		return;
//======================================add by ycf 2025.8.13=============================================
	//vi53xx_dev_clean(xpdev);	//不必要xpdev_free(xpdev);中执行xpdev_destroy_interfaces(xpdev); 中会执行vi53xx_dev_clean(xpdev)
	xdma_free_resource(xpdev->pdev, xpdev->xdev);
//======================================add by ycf 2025.8.13=============================================
	pr_info("pdev 0x%p, xdev 0x%p, 0x%p.\n",
		pdev, xpdev, xpdev->xdev);
	xpdev_free(xpdev);
//======================================add by ycf 2025.7.27=============================================
    vi53xx_lock();
	list_del(&xpdev->list);
	vi53xx_unlock();
//======================================add by ycf 2025.7.27=============================================
	dev_set_drvdata(&pdev->dev, NULL);
}

static pci_ers_result_t xdma_error_detected(struct pci_dev *pdev,
					pci_channel_state_t state)
{
	struct xdma_pci_dev *xpdev = dev_get_drvdata(&pdev->dev);

	switch (state) {
	case pci_channel_io_normal:
		return PCI_ERS_RESULT_CAN_RECOVER;
	case pci_channel_io_frozen:
		pr_warn("dev 0x%p,0x%p, frozen state error, reset controller\n",
			pdev, xpdev);
		xdma_device_offline(pdev, xpdev->xdev);
		pci_disable_device(pdev);
		return PCI_ERS_RESULT_NEED_RESET;
	case pci_channel_io_perm_failure:
		pr_warn("dev 0x%p,0x%p, failure state error, req. disconnect\n",
			pdev, xpdev);
		return PCI_ERS_RESULT_DISCONNECT;
	}
	return PCI_ERS_RESULT_NEED_RESET;
}

static pci_ers_result_t xdma_slot_reset(struct pci_dev *pdev)
{
	struct xdma_pci_dev *xpdev = dev_get_drvdata(&pdev->dev);

	pr_info("0x%p restart after slot reset\n", xpdev);
	if (pci_enable_device_mem(pdev)) {
		pr_info("0x%p failed to renable after slot reset\n", xpdev);
		return PCI_ERS_RESULT_DISCONNECT;
	}

	pci_set_master(pdev);
	pci_restore_state(pdev);
	pci_save_state(pdev);
	xdma_device_online(pdev, xpdev->xdev);

	return PCI_ERS_RESULT_RECOVERED;
}

static void xdma_error_resume(struct pci_dev *pdev)
{
	struct xdma_pci_dev *xpdev = dev_get_drvdata(&pdev->dev);

	pr_info("dev 0x%p,0x%p.\n", pdev, xpdev);
#if PCI_AER_NAMECHANGE
	pci_aer_clear_nonfatal_status(pdev);
#else
	pci_cleanup_aer_uncorrect_error_status(pdev);
#endif

}

#if KERNEL_VERSION(4, 13, 0) <= LINUX_VERSION_CODE
static void xdma_reset_prepare(struct pci_dev *pdev)
{
	struct xdma_pci_dev *xpdev = dev_get_drvdata(&pdev->dev);

	pr_info("dev 0x%p,0x%p.\n", pdev, xpdev);
	xdma_device_offline(pdev, xpdev->xdev);
}

static void xdma_reset_done(struct pci_dev *pdev)
{
	struct xdma_pci_dev *xpdev = dev_get_drvdata(&pdev->dev);

	pr_info("dev 0x%p,0x%p.\n", pdev, xpdev);
	xdma_device_online(pdev, xpdev->xdev);
}

#elif KERNEL_VERSION(3, 16, 0) <= LINUX_VERSION_CODE
static void xdma_reset_notify(struct pci_dev *pdev, bool prepare)
{
	struct xdma_pci_dev *xpdev = dev_get_drvdata(&pdev->dev);

	pr_info("dev 0x%p,0x%p, prepare %d.\n", pdev, xpdev, prepare);

	if (prepare)
		xdma_device_offline(pdev, xpdev->xdev);
	else
		xdma_device_online(pdev, xpdev->xdev);
}
#endif

static const struct pci_error_handlers xdma_err_handler = {
	.error_detected	= xdma_error_detected,
	.slot_reset	= xdma_slot_reset,
	.resume		= xdma_error_resume,
#if KERNEL_VERSION(4, 13, 0) <= LINUX_VERSION_CODE
	.reset_prepare	= xdma_reset_prepare,
	.reset_done	= xdma_reset_done,
#elif KERNEL_VERSION(3, 16, 0) <= LINUX_VERSION_CODE
	.reset_notify	= xdma_reset_notify,
#endif
};

static struct pci_driver pci_driver = {
	.name = DRV_MODULE_NAME,
	.id_table = pci_ids,
	.probe = probe_one,
	.remove = remove_one,
	.err_handler = &xdma_err_handler,
};

static int xdma_mod_init(void)
{
	if (init_board_info() < 0) {
	    pr_info("Init board info fail. no mem\n");
        return 0;
	}
	int rv;

	pr_info("%s", version);

	if (desc_blen_max > XDMA_DESC_BLEN_MAX)
		desc_blen_max = XDMA_DESC_BLEN_MAX;
	pr_info("desc_blen_max: 0x%x/%u, timeout: h2c %u c2h %u sec.\n",
		desc_blen_max, desc_blen_max, h2c_timeout, c2h_timeout);

	rv = xdma_cdev_init();
	if (rv < 0)
		return rv;

	return pci_register_driver(&pci_driver);
	
	//vi53xx_xdma_mod_init();
}

static void xdma_mod_exit(void)
{
	/* unregister this driver from the PCI bus driver */
	dbg_init("pci_unregister_driver.\n");
	pci_unregister_driver(&pci_driver);
	xdma_cdev_cleanup();
//======================================add by ycf 2025.8.7=============================================
	//vi53xx_xdma_mod_exit();
	board_info_exit();
	AXIUart_cdev_exit();
//======================================add by ycf 2025.8.7=============================================
}

module_init(xdma_mod_init);
module_exit(xdma_mod_exit);
