#ifndef XUARTNS550_CDEV_H /* prevent circular inclusions */
#define XUARTNS550_CDEV_H /* by using protection macros */


#include <linux/wait.h> // for wait_queue
#include <linux/semaphore.h> // for semaphores
//#include <llinux/spinlock.h>
#include "xuartns550.h" // 假设已经包含了这个头文件
#include "xparameters.h"
#include "xuartns550_l.h"

#define UART_BUFFER_SIZE	32768
/*环形缓冲区是一种高效的数据结构，尤其适合于生产者-消费者模型，例如内核中的中断处理程序（生产者）和用户空间的应用程序（消费者）。
环形缓冲区基本原理:环形缓冲区使用一个固定大小的数组和两个指针：头部（head）和尾部（tail）。
头部指针（head）：指向下一次写入数据的位置，由生产者更新。					尾部指针（tail）：指向下一次读取数据的位置，由消费者更新。
当数据写入时，头部指针向前移动。当数据被读取时，尾部指针向前移动。		如果指针到达数组末尾，它会“环绕”回到数组的开头，这也是它得名“环形”的原因。*/
struct uart_dev {
    struct cdev cdev;
    dev_t dev_num;
    char rx_buffer[UART_BUFFER_SIZE];
    char tx_buffer[UART_BUFFER_SIZE];
    int16_t rx_head, rx_tail;
	int16_t tx_head, tx_tail;
    wait_queue_head_t rx_wait_queue;
	wait_queue_head_t tx_wait_queue;
	XUartNs550 *uart_instance; // 指向UART实例的指针
	spinlock_t rx_lock; // 接收锁
	spinlock_t tx_lock; // 发送锁
};
extern 	int 	AXIUart_cdev_init(void);
extern 	void 	AXIUart_cdev_exit(void);
extern XUartNs550 UartNs550Instance;	
extern struct uart_dev AXIUart_dev;
#endif