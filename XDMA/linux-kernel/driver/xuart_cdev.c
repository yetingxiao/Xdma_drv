#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h> // for copy_to_user/copy_from_user
#include <linux/wait.h> // for wait_queue
#include <linux/circ_buf.h> // for circular buffer macros
#include <linux/semaphore.h> // for semaphores

#include "xuart_cdev.h"
#include "xuartns550.h" // 假设已经包含了这个头文件
#include "xparameters.h"
//#include <linux/stdio.h>
#include "xil_printf.h"
#include "xuartns550_l.h"
#define UART_DEVICE_ID XPAR_UARTNS550_0_DEVICE_ID
#define UART_DEVICE_NAME "AXIUart"
static volatile int TotalErrorCount;
static volatile int TotalReceivedCount;
static volatile int TotalSentCount;
#define UART_CLOCK_HZ		XPAR_UARTNS550_0_CLOCK_HZ
#define UART_BASEADDR		XPAR_UARTNS550_0_BASEADDR


// ... 其他头文件

// ==========================================================
// 1. 定义设备相关数据结构
// ==========================================================



XUartNs550 UartNs550Instance;	
static struct uart_dev AXIUart_dev;

// ==========================================================
// 2. 文件操作回调函数
// ==========================================================
static int AXIUart_open(struct inode *inode, struct file *filp) {
    filp->private_data = &AXIUart_dev;
    return 0;
}

static int AXIUart_release(struct inode *inode, struct file *filp) {
    return 0;
}

static ssize_t AXIUart_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    // 逻辑：从内核缓冲区拷贝数据到用户缓冲区
    // 1. 等待队列：如果缓冲区为空，休眠进程
    // 2. 数据拷贝：使用 copy_to_user 拷贝数据
    // 3. 唤醒：拷贝完成后唤醒等待的进程
    // ...
    // 返回实际读取的字节数
	struct uart_dev *dev = (struct uart_dev *)filp->private_data;
    size_t to_read, copied;
    unsigned long flags;

    // 1. 等待队列：如果缓冲区为空，休眠进程
    // wait_event_interruptible会检查条件，如果条件为假，则让当前进程进入睡眠
    // CIRC_CNT检查环形缓冲区中的数据量
    if (CIRC_CNT(dev->rx_head, dev->rx_tail, UART_BUFFER_SIZE) == 0) {
        if (filp->f_flags & O_NONBLOCK) {
            return -EAGAIN; // 非阻塞模式下无数据则返回错误
        }
        if (wait_event_interruptible(dev->rx_wait_queue,
                                     CIRC_CNT(dev->rx_head, dev->rx_tail, UART_BUFFER_SIZE) > 0)) {
            return -ERESTARTSYS; // 被信号打断
        }
    }

    // 2. 数据拷贝：使用copy_to_user将数据从内核拷贝到用户空间
    spin_lock_irqsave(&dev->lock, flags);
    to_read = CIRC_CNT(dev->rx_head, dev->rx_tail, UART_BUFFER_SIZE);
    if (to_read > count) {
        to_read = count;
    }

    copied = 0;
    while (copied < to_read) {
        // 计算从缓冲区尾部可以连续读取的字节数
        unsigned int tail_end = UART_BUFFER_SIZE - dev->rx_tail;
        size_t read_len = min(to_read - copied, tail_end);

        // 将数据从内核拷贝到用户态
        if (copy_to_user(buf + copied, &dev->rx_buffer[dev->rx_tail], read_len)) {
            spin_unlock_irqrestore(&dev->lock, flags);
            return -EFAULT; // 拷贝失败
        }

        // 更新尾部指针
        dev->rx_tail = (dev->rx_tail + read_len) & (UART_BUFFER_SIZE - 1);
        copied += read_len;
    }

    spin_unlock_irqrestore(&dev->lock, flags);
    return copied;
}

static ssize_t AXIUart_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    // 逻辑：从用户缓冲区拷贝数据到内核并发送
    // 1. 数据拷贝：使用 copy_from_user 拷贝数据
    // 2. 发送数据：调用 XUartNs550_Send 函数
    // ...
    // 返回实际写入的字节数
	struct uart_dev *dev = (struct uart_dev *)filp->private_data;
    size_t to_write, written;
    unsigned long flags;

    // 1. 等待队列：如果缓冲区已满，休眠进程（可选，这里简化为非阻塞）
    // ... 可以添加 wait_event_interruptible 来支持阻塞写入

    // 2. 数据拷贝：使用copy_from_user将数据从用户拷贝到内核空间
    spin_lock_irqsave(&dev->lock, flags);
    to_write = CIRC_SPACE(dev->tx_head, dev->tx_tail, UART_BUFFER_SIZE);
    if (to_write > count) {
        to_write = count;
    }

    written = 0;
    while (written < to_write) {
        // 计算缓冲区头部可以连续写入的字节数
        unsigned int head_end = UART_BUFFER_SIZE - dev->tx_head;
        size_t write_len = min(to_write - written, head_end);

        // 将数据从用户态拷贝到内核
        if (copy_from_user(&dev->tx_buffer[dev->tx_head], buf + written, write_len)) {
            spin_unlock_irqrestore(&dev->lock, flags);
            return -EFAULT; // 拷贝失败
        }

        // 更新头部指针
        dev->tx_head = (dev->tx_head + write_len) & (UART_BUFFER_SIZE - 1);
        written += write_len;
    }
    spin_unlock_irqrestore(&dev->lock, flags);
    
    // 3. 将数据送入UART硬件
    spin_lock_irqsave(&dev->lock, flags);
    while (CIRC_CNT(dev->tx_head, dev->tx_tail, UART_BUFFER_SIZE) > 0) {
        if (XUartNs550_IsSending(dev->uart_instance)) {
            break;
        }
        u8 ByteToSend = dev->tx_buffer[dev->tx_tail];
        dev->tx_tail = (dev->tx_tail + 1) & (UART_BUFFER_SIZE - 1);
        XUartNs550_Send(dev->uart_instance, &ByteToSend, 1);
    }
    spin_unlock_irqrestore(&dev->lock, flags);

    return written;
}

static struct file_operations AXIUart_fops = {
    .owner = THIS_MODULE,
    .open = AXIUart_open,
    .release = AXIUart_release,
    .read = AXIUart_read,
    .write = AXIUart_write,
};

// ==========================================================
// 3. 中断处理程序
// ==========================================================
void XUartNs550_IntHandler(void *CallBackRef, u32 Event, unsigned int EventData) {
    // 逻辑：在中断中读取数据并放入缓冲区
    // 1. 调用 XUartNs550_Recv 读取数据
    // 2. 将数据存入 AXIUart_dev.rx_buffer
    // 3. 唤醒等待队列：调用 wake_up_interruptible
    // ...
	struct uart_dev *dev = (struct uart_dev *)CallBackRef;
    u8 ReceivedByte;
    unsigned long flags;
	u8 Errors;
	/**
 * These constants specify the handler events that are passed to
 * a handler from the driver.  These constants are not bit masks such that
 * only one will be passed at a time to the handler.
 *
 */
/* #define XUN_EVENT_RECV_DATA		1 //< Data has been received
#define XUN_EVENT_RECV_TIMEOUT		2 //< A receive timeout occurred
#define XUN_EVENT_SENT_DATA		3 //< Data has been sent 
#define XUN_EVENT_RECV_ERROR		4 //< A receive error was detected
#define XUN_EVENT_MODEM	 		5 //< A change in modem status 
*/
	/*
	 * All of the data has been sent.
	 */
	if (Event == XUN_EVENT_SENT_DATA) {
		TotalSentCount = EventData;
		spin_lock_irqsave(&dev->lock, flags);

        // 检查发送缓冲区是否有数据
        while (CIRC_CNT(dev->tx_head, dev->tx_tail, UART_BUFFER_SIZE) > 0) {
            // 检查UART发送FIFO是否可以发送
            if (XUartNs550_IsSending(dev->uart_instance)) {
                break; // UART FIFO已满
            }
            // 从内核环形缓冲区取出一个字节
            u8 ByteToSend = dev->tx_buffer[dev->tx_tail];
            dev->tx_tail = (dev->tx_tail + 1) & (UART_BUFFER_SIZE - 1);
            // 将字节写入UART硬件
            XUartNs550_Send(dev->uart_instance, &ByteToSend, 1);
        }

        spin_unlock_irqrestore(&dev->lock, flags);

        // 如果发送缓冲区变空，唤醒在等待写入的进程（可选，用于非阻塞写入）
        // wake_up_interruptible(&dev->tx_wait_queue);
	}

	/*
	 * All of the data has been received.
	 */
	if (Event == XUN_EVENT_RECV_DATA) {
		TotalReceivedCount = EventData;
		// 在访问共享缓冲区之前加锁
        spin_lock_irqsave(&dev->lock, flags);

        // 循环读取所有可用字节
        while (XUartNs550_IsReceiveData(dev->uart_instance)) {
            // 检查接收缓冲区是否已满
            if (CIRC_SPACE(dev->rx_head, dev->rx_tail, UART_BUFFER_SIZE) == 0) {
                // 缓冲区已满，丢弃数据或记录溢出错误
                break;
            }
            // 从UART硬件读取一个字节
            XUartNs550_Recv(dev->uart_instance, &ReceivedByte, 1);
            // 将字节存入内核环形缓冲区
            dev->rx_buffer[dev->rx_head] = ReceivedByte;
            dev->rx_head = (dev->rx_head + 1) & (UART_BUFFER_SIZE - 1);
        }

        // 释放锁
        spin_unlock_irqrestore(&dev->lock, flags);

        // 唤醒所有在等待读取的进程
        wake_up_interruptible(&dev->rx_wait_queue);
	}

	/*
	 * Data was received, but not the expected number of bytes, a
	 * timeout just indicates the data stopped for 4 character times.
	 */
	if (Event == XUN_EVENT_RECV_TIMEOUT) {
		TotalReceivedCount = EventData;
	}

	/*
	 * Data was received with an error, keep the data but determine
	 * what kind of errors occurred.
	 */
	if (Event == XUN_EVENT_RECV_ERROR) {
		TotalReceivedCount = EventData;
		TotalErrorCount++;
		Errors = XUartNs550_GetLastErrors(dev->uart_instance);
	}

/*     // 获取UART中断状态
    IsrStatus = XUartNs550_GetInterruptStatus(dev->uart_instance);
    // 如果是接收中断
    if (IsrStatus & XUN_LSR_RX_FIFO_ERROR) {
    }
    
    // 如果是发送中断
    if (IsrStatus & XUN_IER_TX_EMPTY) {
    }
    // 清除中断状态
    XUartNs550_ClearInterruptStatus(dev->uart_instance, IsrStatus); */
}

// ==========================================================
// 4. 驱动模块加载和卸载
// ==========================================================
int  AXIUart_cdev_init(void) {
	
    int result;
	u16 Options = 0;
    XUartNs550_Config *ConfigPtr;


    printk(KERN_INFO "AXIUart: Initializing driver module\n");

    // 1. 初始化设备结构体
    memset(&AXIUart_dev, 0, sizeof(AXIUart_dev));
    AXIUart_dev.uart_instance = &UartNs550Instance;
    spin_lock_init(&AXIUart_dev.lock);
    init_waitqueue_head(&AXIUart_dev.rx_wait_queue);
    init_waitqueue_head(&AXIUart_dev.tx_wait_queue);
    
    // 2. 注册字符设备
    result = alloc_chrdev_region(&AXIUart_dev.dev_num, 0, 1, UART_DEVICE_NAME);
    if (result < 0) {
        printk(KERN_WARNING "AXIUart: Failed to allocate device number\n");
        return result;
    }
    cdev_init(&AXIUart_dev.cdev, &AXIUart_fops);
    AXIUart_dev.cdev.owner = THIS_MODULE;
    result = cdev_add(&AXIUart_dev.cdev, AXIUart_dev.dev_num, 1);
    if (result < 0) {
        printk(KERN_WARNING "AXIUart: Failed to add cdev\n");
        unregister_chrdev_region(AXIUart_dev.dev_num, 1);
        return result;
    }
    printk(KERN_INFO "AXIUart: Device registered with major %d, minor %d\n", MAJOR(AXIUart_dev.dev_num), MINOR(AXIUart_dev.dev_num));

    // 3. 硬件初始化
	result=XUartNs550_Initialize(AXIUart_dev.uart_instance,UART_DEVICE_ID);
    if (result != XST_SUCCESS) {
        printk(KERN_ERR "AXIUart: XUartNs550_CfgInitialize failed with code %d\n", result);
        goto cleanup_cdev;
    }

    // 4. 设置波特率和流控（根据需要修改）
    //XUartNs550_SetBaud(AXIUart_dev.uart_instance,UART_CLOCK_HZ, 1562500);
    // XUartNs550_SetDataFormat(...)
    // XUartNs550_SetFlowControl(...)

    // 5. 注册中断
    // 假设 xpdev->pdev 是有效的 PCI 设备指针
    // 假设 vector 是正确的中断号
    // 假设 xpdev 和 irq_handler_t 等参数已正确获取
    XUartNs550_SetHandler(&UartNs550Instance, XUartNs550_IntHandler,&UartNs550Instance);
	Options = XUN_OPTION_DATA_INTR |XUN_OPTION_FIFOS_ENABLE;
	XUartNs550_SetOptions(&UartNs550Instance, Options); 

    //result = xdma_user_isr_register(xpdev->pdev, 0, (irq_handler_t)XUartNs550_InterruptHandler, (void *)&AXIUart_dev);
	
	
    XUartNs550_EnableIntr(AXIUart_dev.uart_instance);

    printk(KERN_INFO "AXIUart: Driver loaded successfully\n");
    return 0;

cleanup_cdev:
    cdev_del(&AXIUart_dev.cdev);
    unregister_chrdev_region(AXIUart_dev.dev_num, 1);
    return -1;



    return 0;
}

void  AXIUart_cdev_exit(void) {
	u16 Options = 0;
	printk(KERN_INFO "AXIUart: Exiting driver module\n");


    // ... 2. 注销字符设备
    cdev_del(&AXIUart_dev.cdev);
    unregister_chrdev_region(AXIUart_dev.dev_num, 1);
	
/* 	
	// ... 1. 注销中断
    // xdma_user_isr_unregister(xpdev->pdev, 0);
	XUartNs550_DisableIntr(AXIUart_dev.uart_instance);
	Options = XUartNs550_GetOptions(&UartNs550Instance);
	Options = Options & ~(XUN_OPTION_DATA_INTR |XUN_OPTION_FIFOS_ENABLE);//| XUN_OPTION_LOOPBACK 
	XUartNs550_SetOptions(&UartNs550Instance, Options); 
	*/
	printk(KERN_INFO "AXIUart: Driver unloaded\n");
}

//module_init(AXIUart_init);
//module_exit(AXIUart_exit);