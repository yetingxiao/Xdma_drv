#ifndef _XUART_IOCTL_H_
#define _XUART_IOCTL_H_

#include <linux/ioctl.h>

// Define a magic number for our ioctl commands to prevent conflicts.
#define UART_IOC_MAGIC 'u'

// Define the ioctl commands.
// _IOW stands for "write" (the user writes to the device)
// The second argument is a unique command number.
// The third argument is the type of the argument to be passed.
#define UART_SET_BAUD_RATE _IOW(UART_IOC_MAGIC, 1, unsigned long)
#define UART_SET_DATA_FORMAT _IOW(UART_IOC_MAGIC, 2, unsigned int)
#define UART_GET_DATA_FORMAT _IOR(UART_IOC_MAGIC, 3, unsigned int)

#endif // _XUART_IOCTL_H_