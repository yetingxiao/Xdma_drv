#include <stddef.h>
#include <linux/printk.h>
volatile unsigned char *	UART_KERNEL_REGS=NULL;
char IS_KERNEL_MAPPED=0;

/* extern unsigned char* 	GetUartKernelBase(void) {
	if(IS_KERNEL_MAPPED)
	{
		IS_KERNEL_MAPPED=0;
		return UART_KERNEL_REGS;
	}else
	{
		pr_info("UART registers unmapped\n");
	}
} */