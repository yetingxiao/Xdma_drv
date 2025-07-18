#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <unistd.h> // 用于 getpagesize()

// --- XDMA 和 UART 配置宏 ---

// XDMA 设备文件路径
#define XDMA_DEV_PATH "/dev/xdma0_user"

// UART 寄存器相对于 XDMA BAR 0 起始地址的偏移量
// !!!! 务必根据你的FPGA设计修改这些偏移量 !!!!
#define UART_BASE_OFFSET 0x30000 // 假设 UART 从 BAR 0 的地址 0x4A2_0000 开始

// 16550D UART 寄存器偏移量 (相对于 UART_BASE_OFFSET)
#define RBR_THR_OFFSET   0x00 // 接收缓冲寄存器 / 发送保持寄存器 (读/写)
#define IER_OFFSET       0x04 // 中断使能寄存器 (读/写)
#define FCR_OFFSET       0x08 // FIFO 控制寄存器 (写)
#define IIR_OFFSET       0x08 // 中断识别寄存器 (读)
#define LCR_OFFSET       0x0C // 线路控制寄存器 (读/写)
#define MCR_OFFSET       0x10 // 调制解调器控制寄存器 (读/写)
#define LSR_OFFSET       0x14 // 线路状态寄存器 (读)
#define MSR_OFFSET       0x18 // 调制解调器状态寄存器 (读)
#define SCR_OFFSET       0x1C // 暂存寄存器 (读/写)
#define DLL_OFFSET       0x00 // 除数锁存器低位字节 (当 LCR 的 DLAB = 1 时)
#define DLM_OFFSET       0x04 // 除数锁存器高位字节 (当 LCR 的 DLAB = 1 时)

// LCR 寄存器位定义
#define LCR_DLAB_ENABLE  0x80 // DLAB = 1
#define LCR_DLAB_DISABLE 0x00 // DLAB = 0

// LCR 数据格式位定义 (8N1, 奇校验)
// 8位数据: 0x03
// 1停止位: 0x00 (默认)
// 奇校验:  0x04 | 0x08 = 0x0C (PEN=1, EPS=0, SP=0)
#define LCR_8N1_ODD_PARITY (0x03 | 0x0C)

// FCR 寄存器位定义
#define FCR_FIFO_ENABLE   0x01 // FIFO 使能
#define FCR_RX_FIFO_RESET 0x02 // 复位接收 FIFO
#define FCR_TX_FIFO_RESET 0x04 // 复位发送 FIFO
#define FRC_FIFO_LEVEL_14 0xC0	//FIFO level 14 Byte

// LSR 寄存器位定义
#define LSR_THRE          0x20 // 发送保持寄存器空闲 (Transmit Holding Register Empty)

// UART 时钟频率 (假设你的 UART IP 参考时钟为 100 MHz)
// !!!! 务必根据你的FPGA设计修改此值 !!!!
#define UART_CLOCK_FREQ_HZ 100000000 // 100 MHz

// 目标波特率
//#define BAUD_RATE 57600
//#define BAUD_RATE 115200
//#define BAUD_RATE 230400
//#define BAUD_RATE 345600
//#define BAUD_RATE 390625
#define BAUD_RATE 781250
//#define BAUD_RATE 1562500
//#define BAUD_RATE 6250000
// 计算波特率除数
// 除数 = UART_CLOCK_FREQ_HZ / (16 * BAUD_RATE)
#define BAUD_RATE_DIVISOR (UART_CLOCK_FREQ_HZ / (16 * BAUD_RATE))

// --- 全局变量 / 文件描述符 ---
int fd_xdma = -1;
volatile unsigned char *uart_regs = MAP_FAILED;

// --- 函数声明 ---
void write_uart_reg(unsigned int offset, char value);
char read_uart_reg(unsigned int offset);
void uart_init();
void send_data_loop();
void cleanup();

int main() {
    printf("--- 通过 XDMA 操作 16550D UART 示例 ---\n");

    // 打开 XDMA 用户设备文件
    fd_xdma = open(XDMA_DEV_PATH, O_RDWR | O_SYNC);
    if (fd_xdma < 0) {
        perror("Error opening XDMA device");
        return EXIT_FAILURE;
    }
    printf("Opened XDMA device: %s\n", XDMA_DEV_PATH);

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
    uart_regs = (volatile unsigned char *)mmap(NULL, mmap_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_xdma, 0);
    if (uart_regs == MAP_FAILED) {
        perror("Error mmapping UART registers");
        cleanup();
        return EXIT_FAILURE;
    }
    printf("UART registers mapped to virtual address: %p (Mapped size: 0x%lX)\n", uart_regs, mmap_length);

    // 初始化 UART
    uart_init();

    // 持续发送数据
    send_data_loop();

    // 清理资源 (通常在 send_data_loop() 中无限循环，这里可能不会执行到)
    cleanup();

    return EXIT_SUCCESS;
}

void write_uart_reg(unsigned int offset, char value) {
    if (uart_regs != MAP_FAILED) {
        *((char*)(uart_regs + UART_BASE_OFFSET + offset)) = value;
        // 可以取消注释下面的行来查看写入调试信息
        // printf("Wrote 0x%02X to offset 0x%02X (Physical: 0x%08X)\n", value, offset, UART_BASE_OFFSET + offset);
    } else {
        fprintf(stderr, "Error: UART registers not mapped for writing.\n");
    }
}

char read_uart_reg(unsigned int offset) {
    if (uart_regs != MAP_FAILED) {
        char value = *((char*)(uart_regs + UART_BASE_OFFSET + offset));
        // 可以取消注释下面的行来查看读取调试信息
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
    write_uart_reg(FCR_OFFSET, FCR_FIFO_ENABLE | FCR_RX_FIFO_RESET | FCR_TX_FIFO_RESET|FRC_FIFO_LEVEL_14);
    printf("FCR set to 0x%02X (FIFO enabled, TX/RX FIFO cleared)\n", FCR_FIFO_ENABLE | FCR_RX_FIFO_RESET | FCR_TX_FIFO_RESET);

    // 6. 重新使能中断 (如果需要，这里我们只是为了发送数据，可以保持禁用或只使能 THR 空闲中断)
    // write_uart_reg(IER_OFFSET, 0x01); // 示例：使能接收数据可用中断

    printf("UART initialization complete.\n");
}

// 持续发送数据 "0123456789"
void send_data_loop() {
    const char *data_to_send = "0123456789abcdef";
	//,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f
    size_t data_len = strlen(data_to_send);
    size_t current_index = 0;
    unsigned int loop_count = 0;

    printf("Starting data transmission loop. Press Ctrl+C to stop.\n");

    while (1) {
        // 读取线路状态寄存器 (LSR)
        unsigned char lsr_value = read_uart_reg(LSR_OFFSET);

        // 检查发送保持寄存器空闲 (THRE) 位
        if ((lsr_value & LSR_THRE) != 0) {
            // THR 已空，可以发送下一个字节
            if (current_index < data_len) {
                write_uart_reg(RBR_THR_OFFSET, data_to_send[current_index]);
                // printf("Sent: '%c'\n", data_to_send[current_index]);
                current_index++;
            } else {
                // 所有数据已发送，重新开始
                current_index = 0;
                loop_count++;
                // printf("Data string sent %u times.\n", loop_count);
            }
        }
        // 小延迟，避免过度轮询，但会影响波特率精度
        usleep(1); // 根据实际需求调整，高速传输可能不需要
    }
}

// 清理资源
void cleanup() {
    if (uart_regs != MAP_FAILED) {
        if (munmap((void *)uart_regs, UART_BASE_OFFSET + SCR_OFFSET + 1) == -1) {
            perror("Error unmapping UART registers");
        } else {
            printf("UART registers unmapped.\n");
        }
    }
    if (fd_xdma != -1) {
        if (close(fd_xdma) == -1) {
            perror("Error closing XDMA device");
        } else {
            printf("XDMA device closed.\n");
        }
    }
    printf("Cleanup complete.\n");
}