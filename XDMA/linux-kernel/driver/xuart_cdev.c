#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h> // for copy_to_user/copy_from_user
#include <linux/wait.h> // for wait_queue
#include <linux/circ_buf.h> // for circular buffer macros
#include <linux/semaphore.h> // for semaphores


#include "libxdma.h"

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

extern struct es_cdev *es_cdevPtr;
// ... 其他头文件

// ==========================================================
// 1. 定义设备相关数据结构
// ==========================================================
XUartNs550 UartNs550Instance;	
struct uart_dev AXIUart_dev;
static struct file_operations AXIUart_fops;
// ==========================================================
// 2. 文件操作回调函数
// ==========================================================
static int AXIUart_open(struct inode *inode, struct file *filp) {
    filp->private_data = &AXIUart_dev;
	//filp->f_op = &AXIUart_fops;
	printk(KERN_INFO "AXIUart: Device opened successfully.\n");
    return 0;
}

static int AXIUart_release(struct inode *inode, struct file *filp) {
	printk(KERN_INFO "AXIUart: Device closed.\n");
    return 0;
}
/*
在 AXIUart_read 函数中，RX缓冲区被用作一个生产者-消费者队列。

生产者：通常是中断服务程序（ISR）。当UART接收到新数据时，ISR会被触发。它会将数据从UART硬件寄存器读取出来，然后将其写入 rx_buffer 的头部。

消费者：是 AXIUart_read 函数，它代表用户空间进程。当用户调用 read 时，它会从 rx_buffer 的尾部读取数据，然后通过 copy_to_user 拷贝给用户。
*/
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
	int buff_count;
	u32 IerRegister;
	//pr_info("AXIUart_read Start\n");
	spin_lock_irqsave(&dev->rx_lock, flags);
	buff_count=CIRC_CNT(dev->rx_head, dev->rx_tail, UART_BUFFER_SIZE);
	spin_unlock_irqrestore(&dev->rx_lock, flags);
	
	//pr_info("Buff Data %d,read %d byte\n",buff_count,count);
    // 1. 检查数据量CIRC_CNT检查环形缓冲区中可读取的数据量。
    if (buff_count== 0) {
        if (filp->f_flags & O_NONBLOCK) {
			//pr_info("AXIUart_read No Data Ready return \n");
            return -EAGAIN; // 非阻塞模式下无数据则返回错误
        }
	// wait_event_interruptible会检查这个条件。等待队列,如果缓冲区为空（CIRC_CNT 为0）进程就会进入睡眠状态;					等待中断处理程序写入新数据并唤醒它。
        if (wait_event_interruptible(dev->rx_wait_queue,
           CIRC_CNT(dev->rx_head, dev->rx_tail, UART_BUFFER_SIZE) > 0)) {
           return -ERESTARTSYS; // 被信号打断
        }
    }

    // 2. 数据拷贝：使用copy_to_user将数据从内核拷贝到用户空间 ;spin_lock_irqsave(&dev->lock, flags) 宏保护共享资源，因为rx_head和rx_tail可能会被read函数和中断处理程序同时访问。
    spin_lock_irqsave(&dev->rx_lock, flags);
    to_read = CIRC_CNT(dev->rx_head, dev->rx_tail, UART_BUFFER_SIZE);
	spin_unlock_irqrestore(&dev->rx_lock, flags);
	
    if (to_read > count) {
        to_read = count;
    }
    copied = 0;
    while (copied < to_read) {
        // 计算从缓冲区尾部可以连续读取的字节数
		spin_lock_irqsave(&dev->rx_lock, flags);
        unsigned int tail_end = UART_BUFFER_SIZE - dev->rx_tail;
		spin_unlock_irqrestore(&dev->rx_lock, flags);
		
        size_t read_len = min(to_read - copied, tail_end);

        // 将数据从内核拷贝到用户态
        if (copy_to_user(buf + copied, &dev->rx_buffer[dev->rx_tail], read_len)) {
            return -EFAULT; // 拷贝失败
        }

    //3. 更新尾部指针：是关键操作，使用位与运算 & (UART_BUFFER_SIZE - 1) 是一个高效的取模运算，前提是 UART_BUFFER_SIZE 是2的幂次方。这确保了指针在到达缓冲区末尾后能够正确地“环绕”
		spin_lock_irqsave(&dev->rx_lock, flags);
        dev->rx_tail = (dev->rx_tail + read_len) & (UART_BUFFER_SIZE - 1);
		spin_unlock_irqrestore(&dev->rx_lock, flags);
		
        copied += read_len;
    }
    
	IerRegister = XUartNs550_ReadReg(dev->uart_instance->BaseAddress,XUN_IER_OFFSET);
	XUartNs550_WriteReg(dev->uart_instance->BaseAddress, XUN_IER_OFFSET,
				 IerRegister | XUN_IER_RX_DATA);

	//pr_info("AXIUart_read end\n");
    return copied;
}
/*				spin_lock_irqsave 宏可以防止并发访问问题，确保当用户进程（read/write）和中断处理程序同时操作缓冲区时，数据一致性不会被破坏。
在 AXIUart_write 函数中，TX缓冲区同样被用作一个队列。

生产者：AXIUart_write 函数本身。当用户调用 write 时，它会将用户数据通过 copy_from_user 拷贝到 tx_buffer 的头部。

消费者：通常也是中断服务程序（ISR），在write函数的末尾直接发送数据。它会从tx_buffer的尾部读取数据，然后将其写入UART硬件寄存器进行发送。
*/
static ssize_t AXIUart_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    // 逻辑：从用户缓冲区拷贝数据到内核并发送
    // 1. 数据拷贝：使用 copy_from_user 拷贝数据
    // 2. 发送数据：调用 XUartNs550_Send 函数
    // 返回实际写入的字节数
	u32 IerRegister;
	struct uart_dev *dev = (struct uart_dev *)filp->private_data;
    size_t to_write, written;
    unsigned long flags;
	int buff_count,buff_space;
	//pr_info("AXIUart_write Start\n");

    spin_lock_irqsave(&dev->tx_lock, flags);
	buff_space=CIRC_SPACE(dev->tx_head, dev->tx_tail, UART_BUFFER_SIZE);//CIRC_CNT 计算的是缓冲区中已使用的字节数（即可读取的数据量）。CIRC_SPACE计算的是缓冲区中空闲的字节数（即可写入的空间量）。
	spin_unlock_irqrestore(&dev->tx_lock, flags);
	//pr_info("Buff Space %d,write %d byte\n",buff_space,count);
    // 1. 检查可用空间 计算缓冲区中剩余的可用空间。如果缓冲区已满，休眠进程,添加等待队列 wait_event_interruptible 来支持阻塞写入
	
	if (buff_space<count)
	{
		
		if (filp->f_flags & O_NONBLOCK) {
			pr_info("AXIUart_write No Buff space return \n");
            return -EAGAIN; // 非阻塞模式下无数据则返回错误
        } 
		
		// wait_event_interruptible会检查这个条件。等待队列,如果缓冲区为空（CIRC_CNT 为0）进程就会进入睡眠状态;					等待中断处理程序写入新数据并唤醒它。
        if (wait_event_interruptible(dev->tx_wait_queue,
           CIRC_CNT(dev->tx_head, dev->tx_tail, UART_BUFFER_SIZE)) > 0) {
		   pr_info("AXIUart_write wait_event_interruptible INT error \n");
           return -ERESTARTSYS; // 被信号打断
        }
	}	
	
	//pr_info("AXIUart_write Buff copy from user \n");
	
	spin_lock_irqsave(&dev->tx_lock, flags);
	to_write = CIRC_SPACE(dev->tx_head, dev->tx_tail, UART_BUFFER_SIZE);
	spin_unlock_irqrestore(&dev->tx_lock, flags);
	
    if (to_write > count) {
        to_write = count;
    }
	
	
    written = 0;
    while (written < to_write) {
        // 计算缓冲区头部可以连续写入的字节数
		spin_lock_irqsave(&dev->tx_lock, flags);
        unsigned int head_end = UART_BUFFER_SIZE - dev->tx_head;
		spin_unlock_irqrestore(&dev->tx_lock, flags);
		
        size_t write_len = min(to_write - written, head_end);//write_len 变量被设置为 count 和可用空间中的较小值，以防止缓冲区溢出。

    // 2. 数据拷贝：使用copy_from_user将数据从用户拷贝到内核空间将数据从用户态拷贝到内核
        if (copy_from_user(&dev->tx_buffer[dev->tx_head], buf + written, write_len)) {
            return -EFAULT; // 拷贝失败
        }
	
		spin_lock_irqsave(&dev->tx_lock, flags);
    // 3.更新头部指针	以环绕方式更新头部指针。
        dev->tx_head = (dev->tx_head + write_len) & (UART_BUFFER_SIZE - 1);
		spin_unlock_irqrestore(&dev->tx_lock, flags);
	
        written += write_len;
    }
    
    
   // 4. 将数据送入UART硬件		
    spin_lock_irqsave(&dev->tx_lock, flags);
	buff_count=CIRC_CNT(dev->tx_head, dev->tx_tail, UART_BUFFER_SIZE);
	spin_unlock_irqrestore(&dev->tx_lock, flags);

    // 唤醒发送中断处理程序，开始发送数据
    // 在这里，我们不直接发送数据，而是确保TX中断已启用,让中断来处理，这是最佳实践
	//pr_info("AXIUart TX_EMPTY Interrupt Enable \n");
	IerRegister = XUartNs550_ReadReg(dev->uart_instance->BaseAddress,XUN_IER_OFFSET);
	//if (IerRegister & XUN_IER_RX_DATA) 
	{
		XUartNs550_WriteReg(dev->uart_instance->BaseAddress, XUN_IER_OFFSET,
				 IerRegister | XUN_IER_TX_EMPTY);
	}
    
	//pr_info("AXIUart_write end\n");
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

	struct uart_dev *dev = (struct uart_dev *)CallBackRef;
    u8 ReceivedBytePtr[7];
	u8 ByteToSendPtr[10];
	u8 ReceivedByte,ByteToSend;
    unsigned long flags;
	u8 Errors;
	int i;
	int buff_count=0;
	size_t data_cnt;
	u32 IerRegister;
	int TotalReceivedCount=0;
/*
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
		spin_lock_irqsave(&dev->tx_lock, flags);
        data_cnt = CIRC_CNT(dev->tx_head, dev->tx_tail, UART_BUFFER_SIZE);
        if (data_cnt > 0) {
            // 从缓冲区取10个字节或剩余所有字节
            u8 *to_send_ptr = &dev->tx_buffer[dev->tx_tail];
            size_t send_len = min_t(size_t, 10, data_cnt);
            spin_unlock_irqrestore(&dev->tx_lock, flags);
			
			//pr_info("XUartNs550_IntHandler Before One Cycle Send \n");
            XUartNs550_Send(dev->uart_instance, to_send_ptr, send_len);
			//pr_info("XUartNs550_IntHandler After One Cycle Send \n");
			
            spin_lock_irqsave(&dev->tx_lock, flags);
            // 更新尾部指针
            dev->tx_tail = (dev->tx_tail + send_len) & (UART_BUFFER_SIZE - 1);
        } else {
            // 缓冲区为空，禁用发送中断
            //XUartNs550_DisableIntr(dev->uart_instance);
			IerRegister = XUartNs550_ReadReg(dev->uart_instance->BaseAddress,
							XUN_IER_OFFSET);
			XUartNs550_WriteReg(dev->uart_instance->BaseAddress, XUN_IER_OFFSET,
				 IerRegister & ~XUN_IER_TX_EMPTY);
        }
        spin_unlock_irqrestore(&dev->tx_lock, flags);
        
        wake_up_interruptible(&dev->tx_wait_queue); // 唤醒等待写入的进程
	}

	/*
	 * All of the data has been received.
	 */
	if (Event == XUN_EVENT_RECV_DATA) {
		TotalReceivedCount = EventData;
		// 在访问共享缓冲区之前加锁
        
		// 判断接收器和/或 FIFO 中是否有接收数据，如果有则循环读取所有可用字节直到没有
		while (XUartNs550_IsReceiveData(dev->uart_instance))  
       {	
			spin_lock_irqsave(&dev->rx_lock, flags);
			buff_count=CIRC_SPACE(dev->rx_head, dev->rx_tail, UART_BUFFER_SIZE) ;
			// 释放锁
			spin_unlock_irqrestore(&dev->rx_lock, flags);
            // 检查接收缓冲区是否已满
            if (buff_count== 0) {
                // 缓冲区已满，丢弃数据或记录溢出错误
				XUartNs550_Recv(dev->uart_instance, &ReceivedByte, 1); // 必须读取以清空FIFO
                //printk(KERN_WARNING "AXIUart: RX buffer overflow, data dropped.\n");
                break;
            }
            // 从UART硬件读取7个字节
			//pr_info("XUartNs550_IntHandler Before One Cycle Rec \n");
            XUartNs550_Recv(dev->uart_instance, ReceivedBytePtr, 7);
			//XUartNs550_Recv(dev->uart_instance, &ReceivedByte, 1);
			//pr_info("XUartNs550_IntHandler After One Cycle Rec \n");

			// 将字节存入内核环形缓冲区
			spin_lock_irqsave(&dev->rx_lock, flags);
			for(i=0;i<7;i++)
			{	
				dev->rx_buffer[dev->rx_head+i] = ReceivedBytePtr[i];
			} 
			dev->rx_head = (dev->rx_head + 7) & (UART_BUFFER_SIZE - 1);
            //dev->rx_buffer[dev->rx_head] = ReceivedByte;
            //dev->rx_head = (dev->rx_head + 1) & (UART_BUFFER_SIZE - 1);
			spin_unlock_irqrestore(&dev->rx_lock, flags);
        }

       
        // 如果接收缓冲区变空，唤醒所有在等待读取的进程
        wake_up_interruptible(&dev->rx_wait_queue);
	}

	/*
	 * Data was received, but not the expected number of bytes, a
	 * timeout just indicates the data stopped for 4 character times.
	 */
	if (Event == XUN_EVENT_RECV_TIMEOUT)
	{
			IerRegister = XUartNs550_ReadReg(dev->uart_instance->BaseAddress,XUN_IER_OFFSET);
			XUartNs550_WriteReg(dev->uart_instance->BaseAddress, XUN_IER_OFFSET,
				 IerRegister & ~XUN_IER_RX_DATA);
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
    //spin_lock_init(&AXIUart_dev.lock);
	spin_lock_init(&(AXIUart_dev.rx_lock)); // 初始化接收锁
	spin_lock_init(&(AXIUart_dev.tx_lock)); // 初始化发送锁
    init_waitqueue_head(&(AXIUart_dev.rx_wait_queue));
    init_waitqueue_head(&(AXIUart_dev.tx_wait_queue));
    
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
	
	if (es_cdevPtr->cdev_class) {
		device_create(es_cdevPtr->cdev_class, NULL, AXIUart_dev.dev_num, NULL, UART_DEVICE_NAME);
	}else{
        printk(KERN_INFO ": failed to create es_cdevPtr->cdev_class");
        return -1;
    }

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


	
    XUartNs550_SetHandler(AXIUart_dev.uart_instance, XUartNs550_IntHandler,&AXIUart_dev);
	Options = XUN_OPTION_FIFOS_ENABLE;//XUN_OPTION_DATA_INTR |
	XUartNs550_SetOptions(AXIUart_dev.uart_instance, Options); 
	

    printk(KERN_INFO "AXIUart: Driver loaded successfully\n");
    return 0;

cleanup_cdev:
	device_destroy(es_cdevPtr->cdev_class, AXIUart_dev.dev_num);
    cdev_del(&AXIUart_dev.cdev);
    unregister_chrdev_region(AXIUart_dev.dev_num, 1);
    return -1;



    return 0;
}

void  AXIUart_cdev_exit(void) {
	u16 Options = 0;
	printk(KERN_INFO "AXIUart: Exiting driver module\n");

	device_destroy(es_cdevPtr->cdev_class, AXIUart_dev.dev_num);
    // ... 2. 注销字符设备
    cdev_del(&AXIUart_dev.cdev);//从内核中删除 cdev 结构体，解除内核与驱动程序的关联。
    unregister_chrdev_region(AXIUart_dev.dev_num, 1);
	
/* 	
	// ... 1. 注销中断
    // xdma_user_isr_unregister(xpdev->pdev, 0);
	*/
}

//module_init(AXIUart_init);
//module_exit(AXIUart_exit);