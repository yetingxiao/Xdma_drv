#ifndef XUARTNS550_CDEV_H /* prevent circular inclusions */
#include <linux/wait.h> // for wait_queue
#include <linux/semaphore.h> // for semaphores
#include "xuartns550.h" // 假设已经包含了这个头文件
#include "xparameters.h"
#include "xuartns550_l.h"

#define XUARTNS550_CDEV_H /* by using protection macros */
#define UART_BUFFER_SIZE 4096
struct uart_dev {
    struct cdev cdev;
    dev_t dev_num;
    char rx_buffer[UART_BUFFER_SIZE];
    char tx_buffer[UART_BUFFER_SIZE];
    unsigned int rx_head, rx_tail;
	unsigned int tx_head, tx_tail;
    wait_queue_head_t rx_wait_queue;
	wait_queue_head_t tx_wait_queue;
	XUartNs550 *uart_instance; // 指向UART实例的指针
    spinlock_t lock; // 用于保护缓冲区访问的自旋锁
};
extern 	int 	my_uart_cdev_init(void);
extern 	void 	my_uart_cdev_exit(void);
#endif