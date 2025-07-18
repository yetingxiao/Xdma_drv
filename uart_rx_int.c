#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <sys/mman.h> // for mmap if directly accessing BARs


// --- XDMA IRQ Block User Interrupt Enable Register 定义 ---
// 这些偏移量是相对于 /dev/xdmaX_user 文件描述符的。
// BAR1_FD_OFFSET: BAR1在文件描述符中的起始偏移（通常为 BAR0 的大小，这里假设为 256B）
#define XDMA_BAR1_FD_OFFSET 0x10000

// XDMA IRQ Block 内部寄存器偏移 (相对于 IRQ Block 的基地址)
// 请务必根据你的 XDMA Product Guide (PG195) 或驱动源码核对这些值
//#define XDMA_IRQ_BLOCK_BASE_OFFSET       0x0002 // IRQ Block 内部的基地址
#define XDMA_IRQ_BLOCK_BASE_OFFSET       0x0000 // IRQ Block 内部的基地址
//#define XDMA_USER_INT_ENABLE_REG_OFFSET  0x04  // 用户中断使能寄存器偏移
#define XDMA_USER_INT_ENABLE_REG_OFFSET  0x108  // 用户中断使能寄存器偏移
#define XDMA_USER_INT_ENABLE_MASK        0xFFFF // 用户中断使能寄存器的所有位 (16个用户中断)

// XDMA 设备文件路径
#define XDMA_USER_DEV_PATH "/dev/xdma0_user" // 根据你的设备路径调整

// 函数：从 BAR 读取 32 位寄存器值
ssize_t read_xdma_reg(int fd, unsigned long offset, unsigned int *value) {
    ssize_t bytes_read = pread(fd, value, sizeof(unsigned int), offset);
    if (bytes_read != sizeof(unsigned int)) {
        perror("Error reading XDMA register");
        return -1;
    }
    return bytes_read;
}

// 函数：向 BAR 写入 32 位寄存器值
ssize_t write_xdma_reg(int fd, unsigned long offset, unsigned int value) {
    ssize_t bytes_written = pwrite(fd, &value, sizeof(unsigned int), offset);
    if (bytes_written != sizeof(unsigned int)) {
        perror("Error writing XDMA register");
        return -1;
    }
    return bytes_written;
}


int enable_Xdma_usr_int()
{
    int fd_xdma_user;
    unsigned int reg_value;
    unsigned long irq_enable_reg_addr;

    // 1. 计算用户中断使能寄存器的实际文件描述符偏移
    irq_enable_reg_addr = XDMA_BAR1_FD_OFFSET + XDMA_IRQ_BLOCK_BASE_OFFSET + XDMA_USER_INT_ENABLE_REG_OFFSET;

    printf("Calculated User Interrupt Enable Register Address: 0x%lx\n", irq_enable_reg_addr);

    // 2. 打开 XDMA 的用户设备文件 (用于访问 BAR0 和 BAR1)
    fd_xdma_user = open(XDMA_USER_DEV_PATH, O_RDWR | O_SYNC); // O_SYNC 确保写操作同步到硬件
    if (fd_xdma_user < 0) {
        perror("Failed to open XDMA user device");
        return EXIT_FAILURE;
    }
    printf("Opened %s successfully.\n", XDMA_USER_DEV_PATH);

    // 3. 读取当前用户中断使能寄存器的值 (推荐先读再改)
    if (read_xdma_reg(fd_xdma_user, irq_enable_reg_addr, &reg_value) == -1) {
        close(fd_xdma_user);
        return EXIT_FAILURE;
    }
    printf("Current XDMA User Interrupt Enable Register (0x%lx): 0x%08x\n", irq_enable_reg_addr, reg_value);

    // 4. 设置新的用户中断使能值
    // 假设你希望使能用户中断 0 和用户中断 1
    // XDMA_USER_INT_ENABLE_MASK 用于确保只修改相关的位，避免影响其他控制位
    unsigned int enable_mask_to_set = (1 << 0) | (1 << 1); // 使能 usr_irq_req[0] 和 usr_irq_req[1]

    // 方法一：只设置指定的位，保留其他位不变 (推荐)
    reg_value |= enable_mask_to_set;

    // 方法二：如果你想精确设置为某个值 (会覆盖所有位，慎用)
    // reg_value = enable_mask_to_set;

    printf("Setting XDMA User Interrupt Enable Register to: 0x%08x\n", reg_value);

    // 5. 将新值写入用户中断使能寄存器
    if (write_xdma_reg(fd_xdma_user, irq_enable_reg_addr, reg_value) == -1) {
        close(fd_xdma_user);
        return EXIT_FAILURE;
    }
    printf("XDMA User Interrupts enabled (Bits 0 & 1). Verification:\n");

    // 6. 再次读取以验证 (可选，但推荐)
    if (read_xdma_reg(fd_xdma_user, irq_enable_reg_addr, &reg_value) == -1) {
        close(fd_xdma_user);
        return EXIT_FAILURE;
    }
    printf("Verified XDMA User Interrupt Enable Register (0x%lx): 0x%08x\n", irq_enable_reg_addr, reg_value);


    // 程序结束，关闭文件描述符
    close(fd_xdma_user);
    printf("XDMA user interrupt enable operation completed.\n");

    return EXIT_SUCCESS;
}


// 16550D UART 寄存器偏移量 (相对于 UART_BASE_OFFSET)
#define RBR_THR_OFFSET   0x00 // 接收缓冲寄存器 / 发送保持寄存器 (读/写) // Receiver Buffer / Transmit Holding Register
#define IER_OFFSET       0x04 // 中断使能寄存器 (读/写) // Interrupt Enable Register
#define FCR_OFFSET       0x08 // FIFO 控制寄存器 (写) // Interrupt Identification / FIFO Control Register
#define IIR_OFFSET       0x08 // 中断识别寄存器 (读)
#define LCR_OFFSET       0x0C // 线路控制寄存器 (读/写)// Line Control Register
#define MCR_OFFSET       0x10 // 调制解调器控制寄存器 (读/写)// Modem Control Register
#define LSR_OFFSET       0x14 // 线路状态寄存器 (读)// Line Status Register
#define MSR_OFFSET       0x18 // 调制解调器状态寄存器 (读)// Modem Status Register
#define SCR_OFFSET       0x1C // 暂存寄存器 (读/写)// Scratch Pad Register
#define DLL_OFFSET       0x00 // 除数锁存器低位字节 (当 LCR 的 DLAB = 1 时)
#define DLM_OFFSET       0x04 // 除数锁存器高位字节 (当 LCR 的 DLAB = 1 时)

// AXI UART 16550 IER位定义 (部分)
#define UART_IER_RX_DATA_AVAIL 0x01 // Enable receive data available interrupt
#define UART_IER_TX_EMPTY      0x02 // Enable transmit holding register empty interrupt

// LCR 寄存器位定义
#define LCR_DLAB_ENABLE  0x80 // DLAB = 1
#define LCR_DLAB_DISABLE 0x00 // DLAB = 0

// XDMA 设备文件
#define XDMA_USER_DEV "/dev/xdma0_user"     // 用于访问FPGA BARs
#define XDMA_EVENT_DEV "/dev/xdma0_events_0" // 用于接收用户中断0

// UART 寄存器相对于 XDMA BAR 0 起始地址的偏移量
// !!!! 务必根据你的FPGA设计修改这些偏移量 !!!!
#define UART_BASE_OFFSET 0x30000 // 假设 UART 从 BAR 0 的地址 0x4A2_0000 开始
#define UART_BAR_SIZE 0x100   // UART寄存器空间大小

// LCR 数据格式位定义 (8N1, 奇校验)
// 8位数据: 0x03
// 1停止位: 0x00 (默认)
// 奇校验:  0x04 | 0x08 = 0x0C (PEN=1, EPS=0, SP=0)
#define LCR_8N1_ODD_PARITY (0x03 | 0x0C)

// FCR 寄存器位定义
#define FCR_FIFO_ENABLE   0x01 // FIFO 使能
#define FCR_RX_FIFO_RESET 0x02 // 复位接收 FIFO
#define FCR_TX_FIFO_RESET 0x04 // 复位发送 FIFO

// LSR 寄存器位定义
#define LSR_THRE          0x20 // 发送保持寄存器空闲 (Transmit Holding Register Empty)
// LSR 寄存器位定义
#define LSR_DR            0x01 // 数据就绪 (Data Ready)
// IER 寄存器位定义
#define IER_RX_DATA_AVAIL 0x01 // 接收数据可用中断使能

// UART 时钟频率 (假设你的 UART IP 参考时钟为 100 MHz)
// !!!! 务必根据你的FPGA设计修改此值 !!!!
#define UART_CLOCK_FREQ_HZ 100000000 // 100 MHz

// 目标波特率
//#define BAUD_RATE 1562500
//#define BAUD_RATE 57600
//#define BAUD_RATE 230400
//#define BAUD_RATE 390625
//#define BAUD_RATE 781250
//#define BAUD_RATE 6250000
#define BAUD_RATE 345600
// 计算波特率除数
// 除数 = UART_CLOCK_FREQ_HZ / (16 * BAUD_RATE)
#define BAUD_RATE_DIVISOR (UART_CLOCK_FREQ_HZ / (16 * BAUD_RATE))

volatile unsigned char *uart_regs = MAP_FAILED;
    int user_fd;
	int event_fd;


// 写入 UART 寄存器函数
void write_uart_reg(unsigned int offset, unsigned char value) {
    if (uart_regs != MAP_FAILED) {
        *(uart_regs + UART_BASE_OFFSET + offset) = value;
        // printf("Wrote 0x%02X to offset 0x%02X (Physical: 0x%08X)\n", value, offset, UART_BASE_OFFSET + offset);
    } else {
        fprintf(stderr, "Error: UART registers not mapped for writing.\n");
    }
}

// 读取 UART 寄存器函数
unsigned char read_uart_reg(unsigned int offset) {
    if (uart_regs != MAP_FAILED) {
        unsigned char value = *(uart_regs + UART_BASE_OFFSET + offset);
        // printf("Read 0x%02X from offset 0x%02X (Physical: 0x%08X)\n", value, offset, UART_BASE_OFFSET + offset);
        return value;
    } else {
        fprintf(stderr, "Error: UART registers not mapped for reading.\n");
        return 0xFF; // 返回错误值
    }
}

// 初始化 UART：设置波特率、奇偶校验、停止位、启用 FIFO
void uart_init() {
    printf("Initializing UART...\n");

    // 1. 禁用中断，防止配置过程中产生不必要的中断
    write_uart_reg(IER_OFFSET, 0x00);

    // 2. 启用 DLAB (Divisor Latch Access Bit) 以访问波特率寄存器
    write_uart_reg(LCR_OFFSET, LCR_DLAB_ENABLE);

    // 3. 设置波特率除数
    unsigned int divisor = BAUD_RATE_DIVISOR;
    write_uart_reg(DLL_OFFSET, (unsigned char)(divisor & 0xFF));        // DLL (低位)
    write_uart_reg(DLM_OFFSET, (unsigned char)((divisor >> 8) & 0xFF)); // DLM (高位)
    printf("Baud rate divisor set to: %u (DLL=0x%02X, DLM=0x%02X)\n", divisor, (unsigned char)(divisor & 0xFF), (unsigned char)((divisor >> 8) & 0xFF));

    // 4. 配置线路控制寄存器 (LCR): 8 位数据，奇校验，1 停止位，禁用 DLAB
    write_uart_reg(LCR_OFFSET, LCR_8N1_ODD_PARITY);
    printf("LCR set to 0x%02X (8N1, Odd Parity, DLAB disabled)\n", LCR_8N1_ODD_PARITY);

    // 5. 配置 FIFO 控制寄存器 (FCR): 启用 FIFO，并清除 TX/RX FIFO
    write_uart_reg(FCR_OFFSET, FCR_FIFO_ENABLE | FCR_RX_FIFO_RESET | FCR_TX_FIFO_RESET);
    printf("FCR set to 0x%02X (FIFO enabled, TX/RX FIFO cleared)\n", FCR_FIFO_ENABLE | FCR_RX_FIFO_RESET | FCR_TX_FIFO_RESET);

    // 6. 启用接收数据可用中断 (可选，这里我们使用轮询，但启用中断可以用于更复杂的驱动)
    write_uart_reg(IER_OFFSET, IER_RX_DATA_AVAIL);
    printf("IER set to 0x%02X (Receive Data Available Interrupt Enabled)\n", IER_RX_DATA_AVAIL);

    printf("UART initialization complete. Waiting for incoming data...\n");
}

// 持续接收数据
void receive_data(){
    unsigned char received_char;
    unsigned char lsr_value;
    int bytes_received = 0;
	size_t data_len=4;
	char *data_to_rec[data_len];
	size_t current_index = 0;
	static unsigned int loop_count = 0;
	
    printf("Starting data reception loop\n");

    {
        // 读取线路状态寄存器 (LSR)
       // lsr_value = read_uart_reg(LSR_OFFSET);

        // 检查数据就绪 (Data Ready, DR) 位
     //   if ((lsr_value & LSR_DR) != 0) 
		{
            // DR 为 1，表示接收 FIFO 中有数据
           
            // 可以添加额外的逻辑，如将数据存储到缓冲区
			
			if (current_index < data_len) {
                received_char = read_uart_reg(RBR_THR_OFFSET); // 从接收缓冲寄存器 (RBR) 读取数据
				bytes_received++;
                // printf("Sent: '%c'\n", data_to_send[current_index]);
                data_to_rec[current_index++]=received_char;
				//printf("Received byte %d: 0x%02X ('%c')\n", bytes_received++, received_char, (isprint(received_char) ? received_char : '.'));
            } else {
                // 所有数据已发送，重新开始
				for(int i=0;i<data_len;i++)
				{
					printf("Received byte %d: 0x%02X ('%c')\n", bytes_received, data_to_rec[i], (isprint(data_to_rec[i]) ? data_to_rec[i] : '.'));
				}
                current_index = 0;
                loop_count++;
                printf("Data string sent %u times.\n", loop_count);
            }
        }
        // 小延迟，避免过度轮询，降低 CPU 占用，但可能会错过高速数据流中的少量字节
        // 适当调整 usleep 值，或者使用中断方式来避免轮询
    }
}

void cleanup() {
    if (uart_regs != MAP_FAILED) {
        if (munmap((void *)uart_regs, UART_BASE_OFFSET + SCR_OFFSET + 1) == -1) {
            perror("Error unmapping UART registers");
        } else {
            printf("UART registers unmapped.\n");
        }
    }
    if (user_fd != -1) {
        if (close(user_fd) == -1) {
            perror("Error closing user_fd XDMA device");
        } else {
            printf("XDMA device closed.\n");
        }
    }
	if (event_fd != -1) {
        if (close(event_fd) == -1) {
            perror("Error closing event_fd XDMA device");
        } else {
            printf("XDMA device closed.\n");
        }
    }
	
    printf("Cleanup complete.\n");
}


int main() {

    //unsigned char *uart_regs;
    struct pollfd pfd;
    int ret;
    char rx_buf[256];
    ssize_t bytes_read;
	enable_Xdma_usr_int();

    // 1. 打开 XDMA 用户设备文件 (用于访问BAR0)
    user_fd = open(XDMA_USER_DEV, O_RDWR | O_SYNC);
    if (user_fd < 0) {
        perror("Failed to open XDMA user device");
        return EXIT_FAILURE;
    }
	printf("Opened XDMA user_fd devices.\n");
	long page_size = getpagesize();
    printf("System Page Size: %ld bytes (0x%lX)\n", page_size, page_size);

    // !!!! 重要：将此值替换为你在 Vivado 中实际配置的 PCIe BAR0 大小 !!!!
    size_t actual_bar0_size = 0x100000; // 示例：1MB

    // 验证你的 UART 基地址偏移是否在 BAR0 的实际范围内
    // UART_BASE_OFFSET 是 UART 相对于 BAR0 起始地址的偏移
    if (UART_BASE_OFFSET + (SCR_OFFSET + 1) > actual_bar0_size) {
        fprintf(stderr, "Error: UART_BASE_OFFSET + register size (0x%lX) exceeds actual BAR0 size (0x%lX).\n",
                UART_BASE_OFFSET + (SCR_OFFSET + 1), actual_bar0_size);
        cleanup();
        return EXIT_FAILURE;
    }

    // 通常，最简单且安全的方法是映射整个 BAR0
    size_t mmap_length = actual_bar0_size;

    // 确保映射长度是页对齐的（虽然通常 BAR 大小本身就是页对齐的）
    if (mmap_length % page_size != 0) {
        mmap_length = ((mmap_length + page_size - 1) / page_size) * page_size;
        fprintf(stderr, "Warning: Adjusted mmap_length to be page-aligned: 0x%lX\n", mmap_length);
    }

    // 现在，使用这个计算出的 mmap_length 来调用 mmap
    uart_regs = (unsigned char *)mmap(NULL, mmap_length, PROT_READ | PROT_WRITE, MAP_SHARED, user_fd, 0);
    if (uart_regs == MAP_FAILED) {
        perror("Failed to mmap UART registers");
        close(user_fd);
        return EXIT_FAILURE;
    }
    printf("UART registers mapped to virtual address: %p (Mapped size: 0x%lX)\n", uart_regs, mmap_length);
	
    // 3. 打开 XDMA 用户中断事件文件
    event_fd = open(XDMA_EVENT_DEV, O_RDONLY);
    if (event_fd < 0) {
        perror("Failed to open XDMA event device");
        munmap(uart_regs, actual_bar0_size);
		//munmap(uart_regs, UART_BAR_SIZE);
        close(user_fd);
        return EXIT_FAILURE;
    }

    printf("Opened XDMA event_fd devices.\n");

    // 4. 配置 UART 16550: 波特率、数据格式等
    // 这需要根据你的UART配置和FPGA时钟来计算分频器
    // 假设已经通过其他方式或默认配置好波特率等
    // 例如：设置LCR，然后设置DLAB=1来访问DLL/DLM寄存器设置分频器
    // 例如：*(volatile unsigned char *)(uart_regs + LCR_OFFSET) = 0x03; // 8N1
    // 例如：*(volatile unsigned char *)(uart_regs + IER_OFFSET) = UART_IER_RX_DATA_AVAIL | UART_IER_TX_EMPTY;
	uart_init();

    // 清除并使能 UART 接收数据可用中断
    //*(volatile unsigned char *)(uart_regs + IER_OFFSET) = UART_IER_RX_DATA_AVAIL;
	//*(volatile unsigned char *)(uart_regs + IER_OFFSET) = UART_IER_RX_DATA_AVAIL;
	*(volatile unsigned char *)(uart_regs + IER_OFFSET) = UART_IER_RX_DATA_AVAIL | UART_IER_TX_EMPTY;
    printf("UART RX Data Available Interrupt enabled.\n");

    // 5. 配置 pollfd 结构体以监听中断事件
    pfd.fd = event_fd;
    pfd.events = POLLIN; // 监听可读事件

    printf("Waiting for UART RX interrupt...\n");

    while (1) {
        // 等待中断，无限等待
        ret = poll(&pfd, 1, -1);
        if (ret < 0) {
            perror("Poll error");
            break;
        }

        if (pfd.revents & POLLIN) {
            // 收到 XDMA 用户中断 (来自 UART)
            unsigned char interrupt_status[4]; // XDMA event files usually return 4 bytes
            bytes_read = read(event_fd, interrupt_status, sizeof(interrupt_status));
            if (bytes_read < 0) {
                perror("Read from XDMA event device failed");
                break;
            }
            printf("Received XDMA user interrupt! Status: 0x%02x%02x%02x%02x\n",
                   interrupt_status[3], interrupt_status[2], interrupt_status[1], interrupt_status[0]);

            // 检查 UART 的中断识别寄存器 (IIR) 来确定中断类型
            unsigned char iir_val = *(volatile unsigned char *)(uart_regs + FCR_OFFSET);
            printf("UART IIR: 0x%02x\n", iir_val);

            // 如果是接收数据中断 (IIR bit 0 = 0, bits 3:1 = 010b or 110b for FIFO trigger)
            if ((iir_val & 0x0F) == 0x04 || (iir_val & 0x0F) == 0x0C) { // IIR bits 3:0 for Rx Data Available (0x04 or 0x0C if FIFO enabled)
                // 读取 UART 接收 FIFO 中的所有数据直到为空
                while ((*(volatile unsigned char *)(uart_regs + LSR_OFFSET) & 0x01)) { // LSR bit 0 (DR) indicates Data Ready
                    char received_char = *(volatile unsigned char *)(uart_regs + RBR_THR_OFFSET);
                    printf("Received char: '%c' (0x%02x)\n", received_char, received_char);
					//receive_data();
                }
            }
            // TODO: 添加其他中断类型处理，例如 TX Empty 中断
            else if (1) { 
                // 其他中断类型...
                // 可以在这里写入更多数据到THR
            }
            
        }
    }

    // 清理
	cleanup() ;
/*     munmap(uart_regs, mmap_length);
    close(user_fd);
    close(event_fd); */

    return EXIT_SUCCESS;
}