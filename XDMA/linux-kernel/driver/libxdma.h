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

#ifndef XDMA_LIB_H
#define XDMA_LIB_H

#include <linux/version.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/workqueue.h>
//======================================add by ycf 2025.7.25=============================================
extern struct list_head pcie_device_list;
#define MAX_EXTRA_ADJ                       (15)
#define MAX_DESC_NUM                        (64)
#define DMA_CHANNEL_NUM                     (2)
#define  H2C_DIR  			                (0)
#define  C2H_DIR  			                (1)
#define  CH1                                (1)
#define  CH2                                (2)

#define  CH1_H2C0                           (0x2000)
#define  CH1_C2H0                           (0x2000)
#define  CH2_C2H1                           (0x400000)
#define  CH2_H2C1                           (0x100)

#define XDMA_CHANNEL_NUM		            (2)
//======================================add by ycf 2025.7.25=============================================

/* Add compatibility checking for RHEL versions */
#if defined(RHEL_RELEASE_CODE)
#	define ACCESS_OK_2_ARGS (RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(8, 0))
#else
#	define ACCESS_OK_2_ARGS (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0))
#endif

#if defined(RHEL_RELEASE_CODE)
#	define HAS_MMIOWB (RHEL_RELEASE_CODE <= RHEL_RELEASE_VERSION(8, 0))
#else
#	define HAS_MMIOWB (LINUX_VERSION_CODE <= KERNEL_VERSION(5, 1, 0))
#endif

#if defined(RHEL_RELEASE_CODE)
#	define HAS_SWAKE_UP_ONE (RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(8, 0))
#	define HAS_SWAKE_UP (RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(8, 0))
#else
#	define HAS_SWAKE_UP_ONE (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
#	define HAS_SWAKE_UP (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 6, 0))
#endif

#if defined(RHEL_RELEASE_CODE)
#	define PCI_AER_NAMECHANGE (RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(8, 3))
#else
#	define PCI_AER_NAMECHANGE (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
#endif

#if	HAS_SWAKE_UP
#include <linux/swait.h>
#endif

/*
 *  if the config bar is fixed, the driver does not neeed to search through
 *  all of the bars
 */
//#define XDMA_CONFIG_BAR_NUM	1

/* SECTION: Preprocessor macros/constants */
#define XDMA_BAR_NUM (6)

/* maximum amount of register space to map */
#define XDMA_BAR_SIZE (0x8000UL)

/* Use this definition to poll several times between calls to schedule */
#define NUM_POLLS_PER_SCHED 100

#define XDMA_CHANNEL_NUM_MAX (4)
/*
 * interrupts per engine, rad2_vul.sv:237
 * .REG_IRQ_OUT	(reg_irq_from_ch[(channel*2) +: 2]),
 */
#define XDMA_ENG_IRQ_NUM	(1)
#define XDMA_MAX_ADJ_BLOCK_SIZE	0x40
#define XDMA_PAGE_SIZE		0x1000
#define RX_STATUS_EOP		(1)

/* Target internal components on XDMA control BAR */
#define XDMA_OFS_INT_CTRL	(0x2000UL)
#define XDMA_OFS_CONFIG		(0x3000UL)

/* maximum number of desc per transfer request */
#define XDMA_ENGINE_XFER_MAX_DESC		0x800
#define XDMA_ENGINE_CREDIT_XFER_MAX_DESC	0x3FF

/* maximum size of a single DMA transfer descriptor */
#define XDMA_DESC_BLEN_BITS	28
#define XDMA_DESC_BLEN_MAX	((1 << (XDMA_DESC_BLEN_BITS)) - 1)

/* bits of the SG DMA control register */
#define XDMA_CTRL_RUN_STOP			(1UL << 0)
#define XDMA_CTRL_IE_DESC_STOPPED		(1UL << 1)
#define XDMA_CTRL_IE_DESC_COMPLETED		(1UL << 2)
#define XDMA_CTRL_IE_DESC_ALIGN_MISMATCH	(1UL << 3)
#define XDMA_CTRL_IE_MAGIC_STOPPED		(1UL << 4)
#define XDMA_CTRL_IE_IDLE_STOPPED		(1UL << 6)
#define XDMA_CTRL_IE_READ_ERROR			(0x1FUL << 9)
#define XDMA_CTRL_IE_DESC_ERROR			(0x1FUL << 19)
#define XDMA_CTRL_NON_INCR_ADDR			(1UL << 25)
#define XDMA_CTRL_POLL_MODE_WB			(1UL << 26)
#define XDMA_CTRL_STM_MODE_WB			(1UL << 27)

/* bits of the SG DMA status register */
#define XDMA_STAT_BUSY			(1UL << 0)
#define XDMA_STAT_DESC_STOPPED		(1UL << 1)
#define XDMA_STAT_DESC_COMPLETED	(1UL << 2)
#define XDMA_STAT_ALIGN_MISMATCH	(1UL << 3)
#define XDMA_STAT_MAGIC_STOPPED		(1UL << 4)
#define XDMA_STAT_INVALID_LEN		(1UL << 5)
#define XDMA_STAT_IDLE_STOPPED		(1UL << 6)

#define XDMA_STAT_COMMON_ERR_MASK \
	(XDMA_STAT_ALIGN_MISMATCH | XDMA_STAT_MAGIC_STOPPED | \
	 XDMA_STAT_INVALID_LEN)

/* desc_error, C2H & H2C */
#define XDMA_STAT_DESC_UNSUPP_REQ	(1UL << 19)
#define XDMA_STAT_DESC_COMPL_ABORT	(1UL << 20)
#define XDMA_STAT_DESC_PARITY_ERR	(1UL << 21)
#define XDMA_STAT_DESC_HEADER_EP	(1UL << 22)
#define XDMA_STAT_DESC_UNEXP_COMPL	(1UL << 23)

#define XDMA_STAT_DESC_ERR_MASK	\
	(XDMA_STAT_DESC_UNSUPP_REQ | XDMA_STAT_DESC_COMPL_ABORT | \
	 XDMA_STAT_DESC_PARITY_ERR | XDMA_STAT_DESC_HEADER_EP | \
	 XDMA_STAT_DESC_UNEXP_COMPL)

/* read error: H2C */
#define XDMA_STAT_H2C_R_UNSUPP_REQ	(1UL << 9)
#define XDMA_STAT_H2C_R_COMPL_ABORT	(1UL << 10)
#define XDMA_STAT_H2C_R_PARITY_ERR	(1UL << 11)
#define XDMA_STAT_H2C_R_HEADER_EP	(1UL << 12)
#define XDMA_STAT_H2C_R_UNEXP_COMPL	(1UL << 13)

#define XDMA_STAT_H2C_R_ERR_MASK	\
	(XDMA_STAT_H2C_R_UNSUPP_REQ | XDMA_STAT_H2C_R_COMPL_ABORT | \
	 XDMA_STAT_H2C_R_PARITY_ERR | XDMA_STAT_H2C_R_HEADER_EP | \
	 XDMA_STAT_H2C_R_UNEXP_COMPL)

/* write error, H2C only */
#define XDMA_STAT_H2C_W_DECODE_ERR	(1UL << 14)
#define XDMA_STAT_H2C_W_SLAVE_ERR	(1UL << 15)

#define XDMA_STAT_H2C_W_ERR_MASK	\
	(XDMA_STAT_H2C_W_DECODE_ERR | XDMA_STAT_H2C_W_SLAVE_ERR)

/* read error: C2H */
#define XDMA_STAT_C2H_R_DECODE_ERR	(1UL << 9)
#define XDMA_STAT_C2H_R_SLAVE_ERR	(1UL << 10)

#define XDMA_STAT_C2H_R_ERR_MASK	\
	(XDMA_STAT_C2H_R_DECODE_ERR | XDMA_STAT_C2H_R_SLAVE_ERR)

/* all combined */
#define XDMA_STAT_H2C_ERR_MASK	\
	(XDMA_STAT_COMMON_ERR_MASK | XDMA_STAT_DESC_ERR_MASK | \
	 XDMA_STAT_H2C_R_ERR_MASK | XDMA_STAT_H2C_W_ERR_MASK)

#define XDMA_STAT_C2H_ERR_MASK	\
	(XDMA_STAT_COMMON_ERR_MASK | XDMA_STAT_DESC_ERR_MASK | \
	 XDMA_STAT_C2H_R_ERR_MASK)

/* bits of the SGDMA descriptor control field */
#define XDMA_DESC_STOPPED	(1UL << 0)
#define XDMA_DESC_COMPLETED	(1UL << 1)
#define XDMA_DESC_EOP		(1UL << 4)

#define XDMA_PERF_RUN	(1UL << 0)
#define XDMA_PERF_CLEAR	(1UL << 1)
#define XDMA_PERF_AUTO	(1UL << 2)

#define MAGIC_ENGINE	0xEEEEEEEEUL
#define MAGIC_DEVICE	0xDDDDDDDDUL

/* upper 16-bits of engine identifier register */
#define XDMA_ID_H2C 0x1fc0U
#define XDMA_ID_C2H 0x1fc1U

/* for C2H AXI-ST mode */
#define CYCLIC_RX_PAGES_MAX	256

#define LS_BYTE_MASK 0x000000FFUL

#define BLOCK_ID_MASK 0xFFF00000
#define BLOCK_ID_HEAD 0x1FC00000

#define IRQ_BLOCK_ID 0x1fc20000UL
#define CONFIG_BLOCK_ID 0x1fc30000UL

#define WB_COUNT_MASK 0x00ffffffUL
#define WB_ERR_MASK (1UL << 31)
#define POLL_TIMEOUT_SECONDS 10

#define MAX_USER_IRQ 16

#define MAX_DESC_BUS_ADDR (0xffffffffULL)

#define DESC_MAGIC 0xAD4B0000UL

#define C2H_WB 0x52B4UL

#define MAX_NUM_ENGINES (XDMA_CHANNEL_NUM_MAX * 2)
#define H2C_CHANNEL_OFFSET 0x1000
#define SGDMA_OFFSET_FROM_CHANNEL 0x4000
#define CHANNEL_SPACING 0x100
#define TARGET_SPACING 0x1000

#define BYPASS_MODE_SPACING 0x0100

/* obtain the 32 most significant (high) bits of a 32-bit or 64-bit address */
#define PCI_DMA_H(addr) ((addr >> 16) >> 16)
/* obtain the 32 least significant (low) bits of a 32-bit or 64-bit address */
#define PCI_DMA_L(addr) (addr & 0xffffffffUL)

#ifndef VM_RESERVED
	#define VMEM_FLAGS (VM_IO | VM_DONTEXPAND | VM_DONTDUMP)
#else
	#define VMEM_FLAGS (VM_IO | VM_RESERVED)
#endif

#ifdef __LIBXDMA_DEBUG__
#define dbg_io		pr_err
#define dbg_fops	pr_err
#define dbg_perf	pr_err
#define dbg_sg		pr_err
#define dbg_tfr		pr_err
#define dbg_irq		pr_err
#define dbg_init	pr_err
#define dbg_desc	pr_err
#else
/* disable debugging */
#define dbg_io(...)
#define dbg_fops(...)
#define dbg_perf(...)
#define dbg_sg(...)
#define dbg_tfr(...)
#define dbg_irq(...)
#define dbg_init(...)
#define dbg_desc(...)
#endif

/* SECTION: Enum definitions */
enum transfer_state {
	TRANSFER_STATE_NEW = 0,
	TRANSFER_STATE_SUBMITTED,
	TRANSFER_STATE_COMPLETED,
	TRANSFER_STATE_FAILED,
	TRANSFER_STATE_ABORTED
};

enum shutdown_state {
	ENGINE_SHUTDOWN_NONE = 0,	/* No shutdown in progress */
	ENGINE_SHUTDOWN_REQUEST = 1,	/* engine requested to shutdown */
	ENGINE_SHUTDOWN_IDLE = 2	/* engine has shutdown and is idle */
};

enum dev_capabilities {
	CAP_64BIT_DMA = 2,
	CAP_64BIT_DESC = 4,
	CAP_ENGINE_WRITE = 8,
	CAP_ENGINE_READ = 16
};

/* SECTION: Structure definitions */

struct xdma_io_cb {
	void __user *buf;
	size_t len;
	void *private;
	unsigned int pages_nr;
	struct sg_table sgt;
	struct page **pages;
	/** total data size */
	unsigned int count;
	/** MM only, DDR/BRAM memory addr */
	u64 ep_addr;
	/** write: if write to the device */
	struct xdma_request_cb *req;
	u8 write:1;
	void (*io_done)(unsigned long cb_hndl, int err);
};

struct config_regs {
	u32 identifier;
	u32 reserved_1[4];
	u32 msi_enable;
};

/**
 * SG DMA Controller status and control registers
 *
 * These registers make the control interface for DMA transfers.
 *
 * It sits in End Point (FPGA) memory BAR[0] for 32-bit or BAR[0:1] for 64-bit.
 * It references the first descriptor which exists in Root Complex (PC) memory.
 *
 * @note The registers must be accessed using 32-bit (PCI DWORD) read/writes,
 * and their values are in little-endian byte ordering.
 */
struct engine_regs {
	u32 identifier;
	u32 control;
	u32 control_w1s;
	u32 control_w1c;
	u32 reserved_1[12];	/* padding */

	u32 status;
	u32 status_rc;
	u32 completed_desc_count;
	u32 alignments;
	u32 reserved_2[14];	/* padding */

	u32 poll_mode_wb_lo;
	u32 poll_mode_wb_hi;
	u32 interrupt_enable_mask;
	u32 interrupt_enable_mask_w1s;
	u32 interrupt_enable_mask_w1c;
	u32 reserved_3[9];	/* padding */

	u32 perf_ctrl;
	u32 perf_cyc_lo;
	u32 perf_cyc_hi;
	u32 perf_dat_lo;
	u32 perf_dat_hi;
	u32 perf_pnd_lo;
	u32 perf_pnd_hi;
} __packed;

struct engine_sgdma_regs {
	u32 identifier;
	u32 reserved_1[31];	/* padding */

	/* bus address to first descriptor in Root Complex Memory */
	u32 first_desc_lo;
	u32 first_desc_hi;
	/* number of adjacent descriptors at first_desc */
	u32 first_desc_adjacent;
	u32 credits;
} __packed;

struct msix_vec_table_entry {
	u32 msi_vec_addr_lo;
	u32 msi_vec_addr_hi;
	u32 msi_vec_data_lo;
	u32 msi_vec_data_hi;
} __packed;

struct msix_vec_table {
	struct msix_vec_table_entry entry_list[32];
} __packed;

struct interrupt_regs {
	u32 identifier;
	u32 user_int_enable;
	u32 user_int_enable_w1s;
	u32 user_int_enable_w1c;
	u32 channel_int_enable;
	u32 channel_int_enable_w1s;
	u32 channel_int_enable_w1c;
	u32 reserved_1[9];	/* padding */

	u32 user_int_request;
	u32 channel_int_request;
	u32 user_int_pending;
	u32 channel_int_pending;
	u32 reserved_2[12];	/* padding */

	u32 user_msi_vector[8];
	u32 channel_msi_vector[8];
} __packed;

struct sgdma_common_regs {
	u32 padding[8];
	u32 credit_mode_enable;
	u32 credit_mode_enable_w1s;
	u32 credit_mode_enable_w1c;
} __packed;


/* Structure for polled mode descriptor writeback */
struct xdma_poll_wb {
	u32 completed_desc_count;
	u32 reserved_1[7];
} __packed;


/**
 * Descriptor for a single contiguous memory block transfer.
 *
 * Multiple descriptors are linked by means of the next pointer. An additional
 * extra adjacent number gives the amount of extra contiguous descriptors.
 *
 * The descriptors are in root complex memory, and the bytes in the 32-bit
 * words must be in little-endian byte ordering.
 */
struct xdma_desc {
	u32 control;
	u32 bytes;		/* transfer length in bytes */
	u32 src_addr_lo;	/* source address (low 32-bit) */
	u32 src_addr_hi;	/* source address (high 32-bit) */
	u32 dst_addr_lo;	/* destination address (low 32-bit) */
	u32 dst_addr_hi;	/* destination address (high 32-bit) */
	/*
	 * next descriptor in the single-linked list of descriptors;
	 * this is the PCIe (bus) address of the next descriptor in the
	 * root complex memory
	 */
	u32 next_lo;		/* next desc address (low 32-bit) */
	u32 next_hi;		/* next desc address (high 32-bit) */
} __packed;

/* 32 bytes (four 32-bit words) or 64 bytes (eight 32-bit words) */
struct xdma_result {
	u32 status;
	u32 length;
	u32 reserved_1[6];	/* padding */
} __packed;

struct sw_desc {
	dma_addr_t addr;
	unsigned int len;
};

/* Describes a (SG DMA) single transfer for the engine */
#define XFER_FLAG_NEED_UNMAP		0x1
#define XFER_FLAG_ST_C2H_EOP_RCVED	0x2	/* ST c2h only */ 
struct xdma_transfer {
	struct list_head entry;		/* queue of non-completed transfers */
	struct xdma_desc *desc_virt;	/* virt addr of the 1st descriptor */
	struct xdma_result *res_virt;   /* virt addr of result, c2h streaming */
	dma_addr_t res_bus;		/* bus addr for result descriptors */
	dma_addr_t desc_bus;		/* bus addr of the first descriptor */
	int desc_adjacent;		/* adjacent descriptors at desc_bus */
	int desc_num;			/* number of descriptors in transfer */
	int desc_index;			/* index for 1st desc. in transfer */
	int desc_cmpl;			/* completed descriptors */
	int desc_cmpl_th;		/* completed descriptor threshold */
	enum dma_data_direction dir;
#if	HAS_SWAKE_UP
	struct swait_queue_head wq;
#else
	wait_queue_head_t wq;		/* wait queue for transfer completion */
#endif

	enum transfer_state state;	/* state of the transfer */
	unsigned int flags;
	int cyclic;			/* flag if transfer is cyclic */
	int last_in_request;		/* flag if last within request */
	unsigned int len;
	struct sg_table *sgt;
	struct xdma_io_cb *cb;
};

struct xdma_request_cb {
	struct sg_table *sgt;
	u64 ep_addr;
	unsigned int aperture;

	unsigned int total_len;
	unsigned int offset;

	struct scatterlist *sg;
	unsigned int sg_idx;
	unsigned int sg_offset;

	/* Use two transfers in case single request needs to be split */
	struct xdma_transfer tfer[2];

	struct xdma_io_cb *cb;

	unsigned int sw_desc_idx;
	unsigned int sw_desc_cnt;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0)
        struct sw_desc sdesc[];
#else
	struct sw_desc sdesc[0];
#endif
};

struct xdma_engine {
	unsigned long magic;	/* structure ID for sanity checks */
	struct xdma_dev *xdev;	/* parent device */
	char name[16];		/* name of this engine */
	int version;		/* version of this engine */

	/* HW register address offsets */
	struct engine_regs *regs;		/* Control reg BAR offset */
	struct engine_sgdma_regs *sgdma_regs;	/* SGDAM reg BAR offset */
	u32 bypass_offset;			/* Bypass mode BAR offset */

	/* Engine state, configuration and flags */
	enum shutdown_state shutdown;	/* engine shutdown mode */
	enum dma_data_direction dir;
	u8 addr_align;		/* source/dest alignment in bytes */
	u8 len_granularity;	/* transfer length multiple */
	u8 addr_bits;		/* HW datapath address width */
	u8 channel:2;		/* engine indices */
	u8 streaming:1;
	u8 device_open:1;	/* flag if engine node open, ST mode only */
	u8 running:1;		/* flag if the driver started engine */
	u8 non_incr_addr:1;	/* flag if non-incremental addressing used */
	u8 eop_flush:1;		/* st c2h only, flush up the data with eop */
	u8 filler:1;

	int max_extra_adj;	/* descriptor prefetch capability */
	int desc_dequeued;	/* num descriptors of completed transfers */
	u32 desc_max;		/* max # descriptors per xfer */
	u32 status;		/* last known status of device */
	/* only used for MSIX mode to store per-engine interrupt mask value */
	u32 interrupt_enable_mask_value;

	/* Transfer list management */
	struct list_head transfer_list;	/* queue of transfers */

	/* Members applicable to AXI-ST C2H (cyclic) transfers */
	struct xdma_result *cyclic_result;
	dma_addr_t cyclic_result_bus;	/* bus addr for transfer */
	u8 *perf_buf_virt;
	dma_addr_t perf_buf_bus; /* bus address */

	/* Members associated with polled mode support */
	u8 *poll_mode_addr_virt;	/* virt addr for descriptor writeback */
	dma_addr_t poll_mode_bus;	/* bus addr for descriptor writeback */

	/* Members associated with interrupt mode support */
#if	HAS_SWAKE_UP
	struct swait_queue_head shutdown_wq;
#else
	wait_queue_head_t shutdown_wq;	/* wait queue for shutdown sync */
#endif
	spinlock_t lock;		/* protects concurrent access */
	int prev_cpu;			/* remember CPU# of (last) locker */
	int msix_irq_line;		/* MSI-X vector for this engine */
	u32 irq_bitmask;		/* IRQ bit mask for this engine */
	struct work_struct work;	/* Work queue for interrupt handling */

	struct mutex desc_lock;		/* protects concurrent access */
	dma_addr_t desc_bus;
	struct xdma_desc *desc;
	int desc_idx;			/* current descriptor index */
	int desc_used;			/* total descriptors used */

	/* for performance test support */
	struct xdma_performance_ioctl *xdma_perf;	/* perf test control */
#if	HAS_SWAKE_UP
	struct swait_queue_head xdma_perf_wq;
#else
	wait_queue_head_t xdma_perf_wq;	/* Perf test sync */
#endif

	struct xdma_kthread *cmplthp;
	/* completion status thread list for the queue */
	struct list_head cmplthp_list;
	/* pending work thread list */
	/* cpu attached to intr_work */
	unsigned int intr_work_cpu;
};

struct xdma_user_irq {
	struct xdma_dev *xdev;		/* parent device */
	u8 user_idx;			/* 0 ~ 15 */
	u8 events_irq;			/* accumulated IRQs */
	spinlock_t events_lock;		/* lock to safely update events_irq */
	wait_queue_head_t events_wq;	/* wait queue to sync waiting threads */
	irq_handler_t handler;

	void *dev;
};
//===================================add by ycf 2025.7.25===============================
struct xdma_cfg_info {
	unsigned char          direction;                    /* 0-host2card 1-card2host */
	unsigned char          SG_desc_num;                  /* Number of descriptors during DMA transfer */
	unsigned char 	       *pData[MAX_DESC_NUM];         /* Array of data pointers, the pointers point to the transferred data */
	unsigned int           data_len[MAX_DESC_NUM];       /* An array of data lengths, representing the length of data transmitted by each descriptor */
	dma_addr_t             data_phy_addr[MAX_DESC_NUM];  /* Array of physical addresses, storing the physical addresses of the transferred data */
	struct xdma_desc       *pFirst_desc;                 /* pointer to the first descriptor */
	dma_addr_t             first_desc_phy_addr;          /* physical address of the first descriptor */
    size_t                 size;
    int                    channel;
    u32                    ep_addr;
};

struct xdma_fops {
    unsigned int (*read_reg)(void *reg_base, unsigned int offset);
    unsigned int (*write_reg)(void *reg_base, unsigned int offset, unsigned int val);
    int (*start)(struct xdma_dev *xdev, unsigned char channel, unsigned char direction);
    int (*start_desc)(struct xdma_dev *xdev, unsigned char channel, struct xdma_cfg_info *cfg);
    int (*stop)(struct xdma_dev *xdev, unsigned char channel, unsigned char direction);
    int (*poll_init)(struct xdma_dev *xdev, int channel, unsigned char direction);
    void (*create_transfer)(struct xdma_dev *xdev, unsigned char channel, struct xdma_cfg_info *cfg);
    int (*xdma_mmap)(struct xdma_dev *xdev, void *vm, struct xdma_cfg_info *cfg);
};
//===================================add by ycf 2025.7.25===============================
/* XDMA PCIe device specific book-keeping */
#define XDEV_FLAG_OFFLINE	0x1
struct xdma_dev {
	struct list_head list_head;
	struct list_head rcu_node;

	unsigned long magic;		/* structure ID for sanity checks */
	struct pci_dev *pdev;	/* pci device struct from probe() */
	int idx;		/* dev index */

	const char *mod_name;		/* name of module owning the dev */

	spinlock_t lock;		/* protects concurrent access */
	unsigned int flags;

	/* PCIe BAR management */
	void __iomem *bar[XDMA_BAR_NUM];	/* addresses for mapped BARs */
	int user_bar_idx;	/* BAR index of user logic */
	int config_bar_idx;	/* BAR index of XDMA config logic */
	int bypass_bar_idx;	/* BAR index of XDMA bypass logic */
	int regions_in_use;	/* flag if dev was in use during probe() */
	int got_regions;	/* flag if probe() obtained the regions */

	int user_max;
	int c2h_channel_max;
	int h2c_channel_max;

	/* Interrupt management */
	int irq_count;		/* interrupt counter */
	int irq_line;		/* flag if irq allocated successfully */
	int msi_enabled;	/* flag if msi was enabled for the device */
	int msix_enabled;	/* flag if msi-x was enabled for the device */
#if KERNEL_VERSION(4, 12, 0) > LINUX_VERSION_CODE
	struct msix_entry entry[32];	/* msi-x vector/entry table */
#endif
	struct xdma_user_irq user_irq[16];	/* user IRQ management */
	unsigned int mask_irq_user;

	/* XDMA engine management */
	int engines_num;	/* Total engine count */
	u32 mask_irq_h2c;
	u32 mask_irq_c2h;
	struct xdma_engine engine_h2c[XDMA_CHANNEL_NUM_MAX];
	struct xdma_engine engine_c2h[XDMA_CHANNEL_NUM_MAX];
//===================================add by ycf 2025.7.25===============================	
	struct xdma_cfg_info C2H_dma_cfg[XDMA_CHANNEL_NUM];
	struct xdma_cfg_info H2C_dma_cfg[XDMA_CHANNEL_NUM];
    struct xdma_fops *fops;
//===================================add by ycf 2025.7.25===============================
	/* SD_Accel specific */
	enum dev_capabilities capabilities;
	u64 feature_id;
};

static inline int xdma_device_flag_check(struct xdma_dev *xdev, unsigned int f)
{
	unsigned long flags;

	spin_lock_irqsave(&xdev->lock, flags);
	if (xdev->flags & f) {
		spin_unlock_irqrestore(&xdev->lock, flags);
		return 1;
	}
	spin_unlock_irqrestore(&xdev->lock, flags);
	return 0;
}

static inline int xdma_device_flag_test_n_set(struct xdma_dev *xdev,
					 unsigned int f)
{
	unsigned long flags;
	int rv = 0;

	spin_lock_irqsave(&xdev->lock, flags);
	if (xdev->flags & f) {
		spin_unlock_irqrestore(&xdev->lock, flags);
		rv = 1;
	} else
		xdev->flags |= f;
	spin_unlock_irqrestore(&xdev->lock, flags);
	return rv;
}

static inline void xdma_device_flag_set(struct xdma_dev *xdev, unsigned int f)
{
	unsigned long flags;

	spin_lock_irqsave(&xdev->lock, flags);
	xdev->flags |= f;
	spin_unlock_irqrestore(&xdev->lock, flags);
}

static inline void xdma_device_flag_clear(struct xdma_dev *xdev, unsigned int f)
{
	unsigned long flags;

	spin_lock_irqsave(&xdev->lock, flags);
	xdev->flags &= ~f;
	spin_unlock_irqrestore(&xdev->lock, flags);
}

void write_register(u32 value, void *iomem);
u32 read_register(void *iomem);

struct xdma_dev *xdev_find_by_pdev(struct pci_dev *pdev);

void xdma_device_offline(struct pci_dev *pdev, void *dev_handle);
void xdma_device_online(struct pci_dev *pdev, void *dev_handle);

int xdma_performance_submit(struct xdma_dev *xdev, struct xdma_engine *engine);
struct xdma_transfer *engine_cyclic_stop(struct xdma_engine *engine);
void enable_perf(struct xdma_engine *engine);
void get_perf_stats(struct xdma_engine *engine);

int engine_addrmode_set(struct xdma_engine *engine, unsigned long arg);
int engine_service_poll(struct xdma_engine *engine, u32 expected_desc_count);

ssize_t xdma_xfer_aperture(struct xdma_engine *engine, bool write, u64 ep_addr,
			unsigned int aperture, struct sg_table *sgt,
			bool dma_mapped, int timeout_ms);
#endif /* XDMA_LIB_H */
