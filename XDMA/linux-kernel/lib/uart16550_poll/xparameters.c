#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <sys/mman.h> // for mmap if directly accessing BARs

#define UARTNS550_0_BASEADDR 0x30000 // 假设 UART 从 BAR 0 的地址 0x3_0000 开始
#define UARTNS550_1_BASEADDR    0x40000 /* IPIF base address */
#define SCR_OFFSET       0x1C // 暂存寄存器 (读/写)// Scratch Pad Register
// XDMA 设备文件路径
#define XDMA_USER_DEV "/dev/xdma0_user" // 根据你的设备路径调整

char IS_MAPPED	=	0;
volatile unsigned char *	UART_REGS=MAP_FAILED;

int GetUartBase() {

    int ret;
	int user_fd;
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
    // UARTNS550_0_BASEADDR 是 UART 相对于 BAR0 起始地址的偏移
    if (UARTNS550_0_BASEADDR + (SCR_OFFSET + 1) > actual_bar0_size) {
        fprintf(stderr, "Error: UARTNS550_0_BASEADDR + register size (0x%lX) exceeds actual BAR0 size (0x%lX).\n",
                UARTNS550_0_BASEADDR + (SCR_OFFSET + 1), actual_bar0_size);
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
	if(!IS_MAPPED)
	{	
		UART_REGS = (unsigned char *)mmap(NULL, mmap_length, PROT_READ | PROT_WRITE, MAP_SHARED, user_fd, 0);
	}
    if (UART_REGS == MAP_FAILED) {
        perror("Failed to mmap UART registers");
        close(user_fd);
		IS_MAPPED=0;
        return EXIT_FAILURE;
    }else
	{
		IS_MAPPED=1;
		printf("UART registers mapped to virtual address: %p (Mapped size: 0x%lX)\n", UART_REGS, mmap_length);
	}
    
	
}