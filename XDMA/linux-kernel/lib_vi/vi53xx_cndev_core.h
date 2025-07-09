
#ifndef RTPC_CNDEV_CORE_H
#define RTPC_CNDEV_CORE_H

#include "libxdma.h"

enum cdev_type {
    CHAR_CTRL,
};

enum flags_bits {
	CDEV_CTRL,
};

struct es_cdev {
	int major;		/* major number */
	struct class *cdev_class;
	dev_t dev;
	char device_name[32];
	uint32_t board_inst;
};

int vi53xx_dev_init(struct xdma_pci_dev *xpdev);
void vi53xx_dev_clean(struct xdma_pci_dev *xpdev);
void vi53xx_cdev_exit(void);
int vi53xx_cdev_init(void);

#endif
